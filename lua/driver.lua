print("hello from lua!")
boinc.dofile("library.lua")

function try_command(cmd)
  print("try_command",cmd)
  local r1, r2, r3 = os.execute(cmd)
  if r1 == true then
    print("success")
  else
    print("failure", r2, r3)
  end
end

print("arg: "..dump(arg))
print("boinc: "..dump(boinc))
print("undefined: "..dump(undefined))
print("boinc status: "..dump(boinc.status()))

print("resolve driver.lua: "..boinc.resolve("driver.lua"))

--print("going to exec /usr/bin/echo test")
--boinc.exec("/usr/bin/echo",{"test1","test2"})

-- to find the C compiler

-- poke around the system
try_command("gcc --version")
try_command("g++ --version")
write_to_file("dummy.c","int main(void) { return 0; }\n")
try_command("gcc -H -o dummy.exe dummy.c")
try_command("ldd ./dummy.exe")
try_command("./dummy.exe")
try_command("g++ -H -o dummy2.exe dummy.c -lboinc_api -lboinc -lpthread -lm")
try_command("ldd ./dummy2.exe")
write_to_file("dummy_boinc.c","#include <boinc/boinc_api.h>\nint main(void) { boinc_finish(0); }\n")
write_to_file("project_specific_defines.h","")
try_command("g++ -o dummy_boinc.exe -I . dummy_boinc.c -lboinc_api -lboinc -lpthread -lm")
try_command("ldd ./dummy_boinc_config.exe")

boinc.dofile("sample2_wu.lua")
boinc.appinit()

boinc.finish(0,"driver script finished")
os.exit(0)

-- uses:
-- brodnet
-- cpu fuzzer
