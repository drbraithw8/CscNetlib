#!/bin/bash

# Directory structure for mSys2.
	LOCAL=/usr/local
	INCLUDE=$LOCAL/include
	INCDIR=$INCLUDE/CscNetLib
	LIBDIR=$LOCAL/lib

# Does /usr/local/include exist?
	if [ ! -d $INCLUDE ]
	then
		mkdir $INCLUDE
	fi

# Does /usr/local/lib exist?
	if [ ! -d $LIBDIR ]
	then
		mkdir $LIBDIR
	fi

# Remove old include files.
	if [ ! -d $INCDIR ]
	then
		mkdir $INCDIR 
	else
		rm $INCDIR/*
	fi

cp std.h isvalid.h iniFile.h logger.h netCli.h netSrv.h \
   ioAny.h servBase.h fileProperties.h cstr.h alloc.h list.h \
   http.h hash.h signal.h dynArray.h json.h udp.h blacklist.h \
   $INCDIR
cp libCscNet.a $LIBDIR

