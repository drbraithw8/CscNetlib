#!/bin/bash

INCDIR=$HOME/include/CscNetLib
LIBDIR=$HOME/lib


if [ ! -d $INCDIR ]
then
    mkdir -p $INCDIR 
else
    rm $INCDIR/*
fi
cp std.h isvalid.h iniFile.h logger.h netCli.h netSrv.h \
   servBase.h fileProperties.h cstr.h alloc.h list.h \
   hash.h signal.h dynArray.h json.h $INCDIR


if [ ! -d $LIBDIR ]
then
    mkdir $LIBDIR 
else
    rm $LIBDIR/*
fi
cp libCscNet.a $LIBDIR


