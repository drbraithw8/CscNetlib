#!/bin/bash

INCDIR=$HOME/include/CscNetLib
LIBDIR=$HOME/lib


if [ ! -d $INCDIR ]
then
    mkdir -p $INCDIR 
else
    rm $INCDIR/*
fi
cp std.h isvalid.h iniFile.h logger.h netCli.h netSrv.h aes.h \
   ioAny.h servBase.h fileProperties.h cstr.h alloc.h list.h \
   http.h hash.h signal.h dynArray.h json.h udp.h blacklist.h dtour.h \
   $INCDIR

if [ ! -d $LIBDIR ]
then
    mkdir $LIBDIR 
else
    rm $LIBDIR/*
fi
cp libCscNet.a $LIBDIR


