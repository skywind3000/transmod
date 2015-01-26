#!/bin/sh
UNAME=`uname | awk -F\_ '{print $1}'`
FLAG1="-fpic -g -Wall -O3"
FLAG2="-fpic -lpthread"
CC="gcc"

if [ "$UNAME" = "CYGWIN" ] ; then
	FLAG1="-Wall -g -O3"
	FLAG2="-lpthread"
fi
if [ "$UNAME" = "Darwin" ] ; then
	FLAG1="-Wall -g -fPIC -O3"
	FLAG2="-dynamiclib -lpthread"
fi
if [ "$UNAME" = "SunOS" ] ; then
	FLAG1="-Wall -g -Wall -fpic -O3"
	FLAG2="-lsocket -lnsl -lpthread"
fi
if [ "$UNAME" = "AIX" ] ; then
	FLAG1="-g -fpic -O3"
	FLAG2="-lpthread"
	if [ -f /usr/vac/bin/gxlc ] ; then
		CC="/usr/vac/bin/gxlc"
	fi
fi
if [ "$UNAME" = "FreeBSD" ] ; then
	if [ -f /usr/bin/clang ] ; then 
		CC="/usr/bin/clang"
	fi
fi

mkdir -p ../../tmp
cd ../../tmp
mkdir transmod 2> /dev/null
cd transmod
rm -r -f *.o

$CC -c $FLAG1 ../../src/transmod/*.c 
$CC *.o -shared $FLAG2 -o transmod.so

if [ -f transmod.so ] ; then
	rm ../../bin/transmod.so 2> /dev/null
	cp transmod.so ../../bin 2> /dev/null
fi
if [ "$UNAME" = "CYGWIN" ] ; then
	chmod 755 ../../bin/transmod.so
fi
cd ../../src/transmod/
rm -r -f *.plg
chmod 644 *.c *.h *.dsp 2> /dev/null



