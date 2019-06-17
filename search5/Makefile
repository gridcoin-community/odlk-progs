all: search mksample show

search: kan_hash.o kanonizator.o prov_blk_trans.o search5.cpp Stream.cpp wio.cpp gener.cpp Makefile
	g++ -o $@ -g -O2 -I .. -I ../boinc/api -I ../boinc/lib -I ../boinc search5.cpp kan_hash.o kanonizator.o prov_blk_trans.o ../boinc/api/libboinc_api.a ../boinc/lib/libboinc.a -lpthread

show: show.cpp Stream.cpp wio.cpp Makefile
	g++ -o $@ -g -O0 -I .. show.cpp

mksample: mksample.cpp Stream.cpp wio.cpp Makefile
	g++ -o $@ -g -O0 -I .. mksample.cpp

kan_hash.o: ../kanonizator_dlk/hash_tabl.cpp Makefile
	gcc -o $@ -c -O3 -I .. ../kanonizator_dlk/hash_tabl.cpp

kanonizator.o: ../kanonizator_dlk/kanonizator.cpp ../kanonizator_dlk/kanonizator.h ../kanonizator_dlk/massivy.h Makefile
	g++ -o $@ -c -O3 -I .. ../kanonizator_dlk/kanonizator.cpp

prov_blk_trans.o: ../family_mar/prov_blk_trans.cpp ../family_mar/prov_blk_trans.h ../family_mar/prov_blk.h
	g++ -o $@ -c -O3 -I .. ../family_mar/prov_blk_trans.cpp
