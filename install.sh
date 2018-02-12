#!/bin/bash

if [ ! -r /etc/shadow ]
then
    echo Need to be root in order for this script to succeed.
    exit
fi

INCDIR=/usr/local/include/CscNetLib
LIBDIR=/usr/local/lib

if [ ! -d $INCDIR ]
then
    mkdir $INCDIR 
else
    rm $INCDIR/*
fi

cp std.h isvalid.h iniFile.h logger.h netCli.h netSrv.h \
   ioAny.h servBase.h fileProperties.h cstr.h alloc.h list.h \
   http.h hash.h signal.h dynArray.h json.h udp.h $INCDIR
cp libCscNet.a $LIBDIR


