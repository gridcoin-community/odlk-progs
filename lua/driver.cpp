#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <ctime>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#ifdef WITH_BOINC
#include "boinc_api.h"
#endif

using std::vector;
using std::cerr;
using std::endl;
using std::string;


lua_State* L;
string script_name;

/* api
 * boinc_finish
 * boinc_resolve
 * boinc_fraction_done
 * lock/unlock critical
 * checkpoint
 * status query
 * boinc_network
 * ?message up/down, upload_file
 * temp_exit
 * wu times
 * table dump function
 * init data access
 */

void message(const std::string& str, int err, bool notice) {
	#ifdef WITH_BOINC
	char buf[256];
	boinc_msg_prefix(buf, sizeof(buf));
	#else
	char buf[] = ":";
	#endif

	fprintf(stderr,
				"%s %s\n",
				buf, str.c_str()
	);
	if(err>=0) {
		#ifdef WITH_BOINC
		boinc_finish_message(err, str.c_str(),notice);
		#else
		fprintf(stderr,"%s exit(%d)\n",buf,err);
		#endif
		exit(err);
	}
}

int boinc_lua_finish(lua_State* L) {
	int err = luaL_checkinteger (L, 1);
	const char* str = luaL_checkstring(L, 2);
	bool notice = luaL_optinteger(L, 3, 0);
	#ifdef WITH_BOINC
	char buf[256];
	boinc_msg_prefix(buf, sizeof(buf));
	#else
	char buf[] = ":";
	#endif

	if(str) {
		fprintf(stderr,
					"%sL %s\n",
					buf, str
		);
	}
	if(err>=0) {
		#ifdef WITH_BOINC
		boinc_finish_message(err, str, notice);
		#else
		fprintf(stderr,"%s exit(%d)\n",buf,err);
		#endif
		exit(err);
	}
	return 0;
}

int boinc_lua_resolve(lua_State* L) {
	const char* name = luaL_checkstring(L, 1);
	#ifdef WITH_BOINC
	string path2;
	boinc_resolve_filename_s(name, path2);
	lua_pushstring(L,path2.c_str());
	#endif
	return 1;
}

int boinc_lua_fraction_done(lua_State* L) {
	lua_Number frac = luaL_checknumber(L, 1);
	#ifdef WITH_BOINC
	boinc_fraction_done(frac);
	#endif
	return 0;
}

int boinc_lua_critical(lua_State* L) {
	int mode = luaL_checkinteger(L, 1);
	#ifdef WITH_BOINC
	switch(mode) {
		case 0:
			boinc_begin_critical_section();
			return 0;
		case 2:
			boinc_checkpoint_completed();
			message("Checkpoint completed",-1,0);
			return 0;
		case 1:
			boinc_end_critical_section();
			return 0;
		default:
			return lua_error(L);
	}
	#else
	return 0;
	#endif
}

int boinc_lua_checkpoint_test(lua_State* L) {
	#ifdef WITH_BOINC
	lua_pushboolean(L, boinc_time_to_checkpoint());
	#else
	lua_pushboolean(L, false);
	#endif
	return 1;
}

int boinc_lua_checkpoint_done(lua_State* L) {
	message("Checkpoint completed",-1,0);
	#ifdef WITH_BOINC
	boinc_checkpoint_completed();
	#endif
	return 0;
}

int boinc_lua_status(lua_State* L) {
	#ifdef WITH_BOINC
	lua_newtable(L);
	lua_pushstring(L, "no_heartbeat"); lua_pushboolean(L, boinc_status.no_heartbeat); lua_rawset(L,-3);
	lua_pushstring(L, "suspended"); lua_pushboolean(L, boinc_status.suspended); lua_rawset(L,-3);
	lua_pushstring(L, "quit_request"); lua_pushboolean(L, boinc_status.quit_request); lua_rawset(L,-3);
	lua_pushstring(L, "reread_init_data_file"); lua_pushboolean(L, boinc_status.reread_init_data_file); lua_rawset(L,-3);
	lua_pushstring(L, "abort_request"); lua_pushboolean(L, boinc_status.abort_request); lua_rawset(L,-3);
	lua_pushstring(L, "network_suspended"); lua_pushboolean(L, boinc_status.network_suspended); lua_rawset(L,-3);
	lua_pushstring(L, "working_set_size"); lua_pushnumber(L, boinc_status.working_set_size); lua_rawset(L,-3);
	lua_pushstring(L, "max_working_set_size"); lua_pushnumber(L, boinc_status.max_working_set_size); lua_rawset(L,-3);

	lua_pushstring(L, "have_network"); lua_pushboolean(L, boinc_network_poll()); lua_rawset(L,-3);
	lua_pushstring(L, "is_standalone"); lua_pushboolean(L, boinc_is_standalone()); lua_rawset(L,-3);
	#else
	lua_pushnil(L);
	#endif
	return 1;
}

int boinc_lua_need_network(lua_State* L) {
	#ifdef WITH_BOINC
	int mode = luaL_checkinteger(L, 1);
	switch(mode) {
		case 0:
			boinc_network_done();
			return 0;
		case 1:
			boinc_need_network();
			return 0;
		default:
			return lua_error(L);
	}
	#else
	return 0;
	#endif
}

int boinc_lua_network_usage(lua_State* L) {
	lua_Number ul = luaL_checknumber(L, 1);
	lua_Number dl = luaL_checknumber(L, 2);
	#ifdef WITH_BOINC
	boinc_network_usage(ul,dl);
	#endif
	return 0;
}
	
int boinc_lua_get_cpu_time(lua_State* L) {
	#ifdef WITH_BOINC
	double cputime; boinc_wu_cpu_time(cputime);
	lua_pushnumber(L,cputime);
	#else
	lua_pushnumber(L,double(clock())/CLOCKS_PER_SEC);
	#endif
	return 1;
}

int boinc_lua_temp_exit(lua_State* L) {
	int delay = luaL_checkinteger (L, 1);
	const char* str = luaL_optstring(L, 2, NULL);
	bool notice = luaL_optinteger(L, 3, 0);
	#ifdef WITH_BOINC
	char buf[256];
	boinc_msg_prefix(buf, sizeof(buf));
	#else
	char buf[] = ":";
	#endif
	fprintf(stderr,
				"%s temporary_exitL %s\n",
				buf, str ? str : ""
	);
	#ifdef WITH_BOINC
	boinc_temporary_exit(delay, str, notice);
	#endif
	exit(100);
	return 0;
}

static int boinc_lua_print (lua_State *L) {
	#ifdef WITH_BOINC
	char prefix[256];
	boinc_msg_prefix(prefix, sizeof(prefix));
	strncat(prefix,"l ",sizeof(prefix)-1);
	#else
	char prefix[] = ":l ";
	#endif
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_getglobal(L, "tostring");
	for (i=1; i<=n; i++) {
		const char *s;
		size_t l;
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushvalue(L, i);   /* value to print */
		lua_call(L, 1, 1);
		s = lua_tolstring(L, -1, &l);  /* get result */
		if (s == NULL)
			return luaL_error(L, "'tostring' must return a string to 'print'");
		if (i>1) fputc('\t', stderr);
		else fputs(prefix, stderr);
		fwrite(s,1,l,stderr);
		lua_pop(L, 1);  /* pop result */
	}
	fputc('\n', stderr);
	return 0;
}

int boinc_lua_setenv(lua_State* L) {
	const char* name = luaL_checkstring (L, 1);
	const char* value = luaL_optstring (L, 2, NULL);
	if(value) {
		setenv(name, value, 1);
	} else {
		unsetenv(name);
	}
	return 0;
}

int boinc_lua_exec(lua_State* L) {
	/* first arg is mandatory and that is the path to exe */
	const char* path = luaL_checkstring (L, 1);
	luaL_checktype(L, 2, LUA_TTABLE);
	char * argv [256] = {0,};
	argv[0] = strdup(path);
	/* rest of the args are args and should be shallow-copyied into args array */
	for(unsigned i=1; i<255; ++i) {
		lua_checkstack(L, 1);
		lua_geti(L, 2, i);
		if(lua_isnil(L,-1))
			break;
		argv[i] = strdup( luaL_checkstring(L,-1) );
		lua_pop(L,1);
	}
	//save_init_data_xml();
	//todo: reset signal handlers
	int rc = execv( path, argv );
	lua_pushinteger(L, errno);
	for(unsigned i=0; i<256; ++i) free(argv[i]);
	return 1;
}

int boinc_lua_libf(lua_State* L) {
	static const struct luaL_Reg mylib [] = {
		{"finish",&boinc_lua_finish}, /* exit code, message, notice */
		{"resolve",&boinc_lua_resolve}, /* path -> path */
		{"progress",&boinc_lua_fraction_done}, /* frac */
		{"critical",&boinc_lua_critical}, /* 0=enter, 1=leave, 2=checkpointed&leave */
		{"checkpoint_test",&boinc_lua_checkpoint_test},
		{"checkpoint_done",&boinc_lua_checkpoint_test},
		{"status",&boinc_lua_status}, /* void -> status table */
		{"need_network",&boinc_lua_need_network},
		{"network_usage",&boinc_lua_network_usage},
		{"get_cpu_time",&boinc_lua_get_cpu_time},
		{"temp_exit",&boinc_lua_temp_exit},
		{"print",&boinc_lua_print},
		{"exec",&boinc_lua_exec},
		{"setenv",&boinc_lua_setenv},
		//send trickle
		//recv trickle
		//upload file
		//upload file status
		{NULL, NULL}
	};
	//register api
	luaL_newlib(L, mylib);
	return 1;
}

void init() {
	#ifdef WITH_BOINC
	BOINC_OPTIONS opts;
	APP_INIT_DATA binitdata;
	char buf[64];
	boinc_options_defaults(opts);
	opts.direct_process_action = 0; // I handle abort/suspend myself
	int retval = boinc_init_options(&opts);
	if(retval)
			message("boinc_init_options failed",206/*EXIT_INIT_FAILURE*/,0);
	boinc_get_init_data(binitdata);
	#endif
	if(dup2(2, 1)<0)
		message("Failed to dup2 standard error into standard output",1,0);
	script_name.clear();
	L = luaL_newstate();
	if (!L)
		message("cannot create lua state",1,0);
	luaL_openlibs(L);
	//register api
	luaL_requiref(L, "boinc", boinc_lua_libf, 1);
	lua_pop(L, 1);
	lua_getglobal(L, "print");
	lua_setglobal(L, "print_orig");
	lua_pushcfunction(L, boinc_lua_print);
	lua_setglobal(L, "print");
}

void parse_arguments(int argc, const char** argv) {
	//add the argv to lua
	int i, narg;
	narg = argc - 1;  /* number of positive indices */
	lua_createtable(L, narg, 0);
	for (i = 1; i < argc; i++) {
		lua_pushstring(L, argv[i]);
		lua_rawseti(L, -2, i);
		if(!strcmp(argv[i],"--script") && i+1<argc) {
			//extract script name override
			script_name = std::string(argv[i+1]);
		}
	}
	lua_setglobal(L, "arg");
}

int script_msghandler (lua_State *L) {
	const char *msg = lua_tostring(L, 1);
	if (msg == NULL) {  /* is error object not a string? */
		if (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
				lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
			return 1;  /* that is the message */
		else
			msg = lua_pushfstring(L, "(error object is a %s value)",
															 luaL_typename(L, 1));
	}
	luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
	return 1;  /* return the traceback */
}

void report_lua_error(const std::string& str, int err, bool notice) {
	#ifdef WITH_BOINC
	char buf[256];
	boinc_msg_prefix(buf, sizeof(buf));
	#else
	char buf[] = ":";
	#endif
	const char *msg = lua_tostring(L, -1);

	fprintf(stderr,
				"%s %s\nlua: %s\n",
				buf, str.c_str(), msg
	);
	if(err>=0) {
		#ifdef WITH_BOINC
		boinc_finish_message(err, str.c_str(),notice);
		#else
		fprintf(stderr,"%s exit(%d)\n",buf,err);
		#endif
		exit(err);
	}
	lua_pop(L, 1);  /* remove message */
}

void run_script() {
	//mem leaks todo
	/*resolve the script physical name*/
	if(script_name.empty()) {
		#ifdef WITH_BOINC
		std::string fn2;
		boinc_resolve_filename_s("driver.lua",fn2); //can't fail
		script_name=std::move(fn2);
		#else
		script_name="driver.lua";
		#endif
	}
	message("Driver script: "+script_name,-1,0);

	int base = lua_gettop(L);
	/* push message handler */
	lua_pushcfunction(L, &script_msghandler);
	/* load the file and push the pseudofunction */
	if( luaL_loadfile(L, script_name.c_str()) )
		report_lua_error("Error while loading driver script.",1,0);
	/* no arguments ... */
	/* call the function in protected mode with "base" message handler*/
	if( lua_pcall(L, /*nargs*/0, /*nres*/0, /*msghandler*/base) )
		report_lua_error("Error while running driver script.",1,0);
	//lua_remove(L, base);
}

int main(int argc, const char** argv) {
	init();
	parse_arguments(argc,argv);
	run_script();
	message("Script have not called finish or exit.",1,0);
}

/*
 * the lua script will prepare the environmnet, compile the main application
 * and exec()ute main program
 * The main program may use its own lua interpreter.
 */
/* dlopen() from static binary is not allowed. So a new exe will have to be
 * compiled to extend the interpreter.
 * Lua is only 150k so the wasted size does not matter here.
 * Shoul we exec the BINARY if it already exists? Time saved from running
 * the interpreter is(?) neglible.
 * The common api stuff must be separate from this launcher cpp.
 */
/* When exec() ing, the current cpu time should be saved to app_init_data,
 * or just hope boinc client does something sensible to it.
 * Or just do not care about it and solve credit in another way.
 */

/* driver script will set it's app name, subtype, plan, batch, version ...
 * driver script will load a "common" library
 * common library will load "overrides" if exists
 * entry -> (a) app.lua (b) wu.lua
 * a) app.lua -> common.lua
 * app.lua -> wu.lua
 * app.lua -> user.lua
 * b) wu.lua -> app.lua
 * app.lua -> libs
 * app.lua -> user.lua
 *
 * The (b) variant is cleaner despite there being two redundant lines.
 * However for wus that only have input file and no config, variant (a)
 * fits better.
 * Everything that could be done multiple ways, should be defined as function
 * before loading the overrides.
 * For apps that themselves contain a interpreter, the wu.lua should be
 * executed by the built binary. Is interpreter needed in the binary?
 * It is compiled, so the script could be compiled-in rather than interpreted.
 * 
 */
