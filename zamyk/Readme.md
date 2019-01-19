Programs for post-processing odlk project results
=======

These programs use classes and routines by Belyshev.

postprocess.exe
---------------

Based on zamyk.bat from Belyshev. Finds additional CF ODLKs from the ones
already in input file. It basically mimiscs the operation of zamyk.bat up to
line 38. No uniqueness check is perormed, the results are simply appended to
output file. This code should have been part of the boinc app, but for some
reason was not. This program is multi-threaded. It will use as many cpu cores
as possible.

Command line: postprocess.exe input output

Input is reas from input file, output is APPENDED to output file. The output
file is not overwritten.

ortogencnt.exe
--------------

This is basically ortogen.exe combined with izvl.exe and type_count.exe.
Additionally it checks for uniqueness and discards duplicate squares from input.
This program is multithreaded too.

```
ortogoncnt.exe -wco input
 -w : write sorted and unique cf odls back to input file
 -c : count cf odls grouped by number of their co-squares
 -o : write out_ortogon.txt and out_kf_N.txt files
 input : database of fancy diagonal latin squares
```

CF ODLSs are read from input file. Duplicates are discarded. If -w option was
set, the input file is rewritten with sorted and unique squares. If -c option
was set, statistics are printes to STDOUT in form like this:

```
Found Fancy CF:
count[1]: 129066
count[2]: 197
count[3]: 1
All: 129267
Found CF co-squares: 129412
```

If the -o option was set, out_ortogon.txt out_kf_X.txt and out_kf_mates.txt
files will be written. If such files exist, they will be overwritten.

