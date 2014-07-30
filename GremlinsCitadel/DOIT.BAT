@echo off
if exist errors del errors
make -DCFLAGS="-Ff=256 -a -ml -N -d -w -Y -c" -DDEFS="-DNETWORK -DALPHA_TEST -DIBM -DSWAP -DOLDHELP"  >>errors

