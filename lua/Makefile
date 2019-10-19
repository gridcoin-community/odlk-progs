all: driver driver_boinc.exe

driver: driver.cpp Makefile
	g++ -o $@ -O0 -g driver.cpp -llua -lpthread

driver_boinc.exe: driver.cpp Makefile
	g++ -o $@ -O3 -g -DWITH_BOINC -static -I ../boinc/api -I ../boinc/lib -I ../boinc -I lua-5.3.5/src driver.cpp ../boinc/api/libboinc_api.a ../boinc/lib/libboinc.a lua-5.3.5/src/liblua.a -lpthread

