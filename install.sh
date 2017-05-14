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
   servBase.h fileProperties.h cstr.h alloc.h list.h \
   hash.h signal.h dynArray.h $INCDIR
cp libCscNet.a $LIBDIR


