#!/bin/bash
set -e
ninja||make
rm -rf test.tmp
mkdir test.tmp
cd test.tmp

echo Check gen_lk_4_31_31
../gen_lk_4_31_31.exe IAEDCLHQS >/dev/null
cmp family_4_31_31_IAEDCLHQS.txt ../gen_lk_4_31_31/family_4_31_31_IAEDCLHQS.txt

echo Check family_mar
../family_mar.exe ../family_mar/input.txt mar_out.txt 2>/dev/null
cmp mar_out.txt ../family_mar/output.txt

echo Check kanonizator_dlk
../kanonizator_dlk.exe ../kanonizator_dlk/input.txt kan_out.txt >/dev/null
cmp kan_out.txt ../kanonizator_dlk/output.txt

echo Check psevdoass
../psevdoass.exe -abk pskan.txt 4 3 7 2 1 9 5 6 >/dev/null
cmp pskan.txt ../psevdoass/mar_abk_4.3.7.2.1.9.5.6.txt

echo ok
