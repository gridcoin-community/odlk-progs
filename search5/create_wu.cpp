
class DB_SEGMENT : public DB_BASE {
public:
	//needed: rule, minl, next
	DB_ID_TYPE id;
	int rule;
	//ix
	//start
	int minl;
	NamerCHDLK10::NameStr next;
	bool enabled;
	//cur_wu
	int prio_adjust; //not where it should be
public:
    DB_SEGMENT(DB_CONN* p=0) : DB_BASE("tot_segment", p?p:&boinc_db) {}
    void db_parse(MYSQL_ROW &r) {
			id = atoi(r[0]);
			rule = atoi(r[1]);
			minl = atoi(r[4]);
			std::copy(r[5],r[5]+next.size(),next.begin());
			enabled = atoi(r[6]);
			prio_adjust=0;
		}
};

struct gen_padls_cfg {
	bool write;
	int batch;
	DB_APP app;
	std::string in_template;
};

void gen_padls_wu(DB_SEGMENT& item, gen_padls_cfg& cfg) {
	// TODO: if enabled
	if(!item.enabled)
		return;
	Input inp;
	CDynamicStream buf;
	inp.rule= item.rule;
	if(!NamerCHDLK10::fromName58(item.next,inp.start))
		throw EDatabase("Bad sndlk in segment row");
	inp.min_level= item.minl;
	inp.skip_below= 0;
	inp.skip_fast= 0;
	inp.skip_rule= {0};
	inp.lim_sn= 3815000000;
	inp.lim_kf= 224000;
	std::stringstream wuname;
	((wuname<<"tot5_"<<item.rule<<char(cfg.batch+'a'))<<"_").write(item.next.data(),item.next.size());
	cout<<" WU "<<wuname.str()<<endl;
	if(!cfg.write)
		return;
	inp.writeInput(buf);
	std::stringstream fninp;
	fninp<<config.download_dir<<"/"<<wuname.str()<<".in";
	std::ofstream fhinp(fninp.str(),ios::binary); // todo - noclobber
	fhinp.write((char*)buf.getbase(),buf.pos());
	fhinp.close(); if(!fhinp) {
		throw EDatabase("Unable to write next input file");
	}
	DB_WORKUNIT wu; wu.clear();
	wu.appid = cfg.app.id;
	wu.batch=20+cfg.batch;
	strcpy(wu.name, wuname.str().c_str());
	wu.rsc_fpops_est = 4e13; // 2x 1h
	wu.rsc_fpops_bound = 1e16;
	wu.rsc_memory_bound = 1e8; //todo 100M
	wu.rsc_disk_bound = 1e8; //todo 100m
	wu.delay_bound = 604800; // 7 days
	wu.priority = 1 + item.prio_adjust;
	wu.target_nresults= wu.min_quorum = 1;
	wu.max_error_results= wu.max_total_results= 8;
	wu.max_success_results= 1;
	
	vector<INFILE_DESC> infile_specs(1);
	infile_specs[0].is_remote = false;
	strcpy(infile_specs[0].name, (wuname.str()+".in").c_str());
	retval= create_work2(wu, cfg.in_template.c_str(),"templates/tot5_out",0,infile_specs,config,0,0,0);
	if(retval) throw EDatabase("create_work2 failed");
	std::stringstream qr{};
	qr<<"update tot_segment set cur_wu="<<wu.id<<" where id="<<item.id<<" limit 1;";
	retval = boinc_db.do_query(qr.str().c_str());
	if(retval || boinc_db.affected_rows()!=1) {
		throw EDatabase("segment update error");
	}
}
