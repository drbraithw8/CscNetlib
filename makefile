
CscNetLib := libCscNet.a

CscNetLibObj := iniFile.o logger.o netCli.o netSrv.o servBase.o http.o \
					cstr.o signal.o isvalid.o fileProperties.o ioAny.o \
					std.o alloc.o hash.o hashStr.o list.o memcheck.o json.o \
					udp.o blacklist.o aes.o dtour.o

LIBS= 

.c.o :
	gcc -c -std=gnu99 $<

all: $(CscNetLib)

aes.o: aes.c aes.h
	gcc -c -std=gnu99 -maes $<

$(CscNetLib) :  $(CscNetLibObj)
	ar rcs $(CscNetLib) $(CscNetLibObj) 

clean:  
	rm $(CscNetLibObj) $(CscNetLib) 2>/dev/null ; true
