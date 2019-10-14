all: sievetest scpn2 stuff generate1

scpn1: scpn1.cpp
	g++ -g -o $@ -O1 scpn1.cpp -lprimesieve

sievetest: sievetest.cpp
	g++ -g -o $@ -O1 sievetest.cpp primesieve/libprimesieve.so

scpn2: scpn2.cpp
	g++ -o $@ -O3 -static -I ../boinc/api -I ../boinc/lib -I ../boinc scpn2.cpp primesieve/libprimesieve.a ../boinc/api/libboinc_api.a ../boinc/lib/libboinc.a -lpthread

stuff: stuff.cpp
	g++ -g -o $@ -O0 -I ../boinc/api -I ../boinc/lib -I ../boinc stuff.cpp -lprimesieve ../boinc/api/libboinc_api.a ../boinc/lib/libboinc.a -lpthread

generate1: generate1.cpp Stream.cpp wio.cpp Makefile
	g++ -o $@ -g -O1 -I .. -I .. -I ../boinc/api -I ../boinc/lib -I ../boinc/tools -I ../boinc/sched -I ../boinc/db -I /usr/include/mysql -I ../boinc generate1.cpp ../boinc/sched/libsched.a ../boinc/api/libboinc_api.a ../boinc/lib/libboinc.a ../boinc/lib/libboinc_crypt.a -lmysqlclient -lpthread -lssl -lcrypto
