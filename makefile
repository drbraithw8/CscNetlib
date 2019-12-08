
CscNetLib := libCscNet.a

CscNetLibObj := iniFile.o logger.o netCli.o netSrv.o servBase.o http.o \
					cstr.o signal.o isvalid.o fileProperties.o ioAny.o \
					std.o alloc.o hash.o hashStr.o list.o memcheck.o json.o \
					udp.o blacklist.o

LIBS= 

.c.o :
	gcc -c -std=gnu99 $<

all: $(CscNetLib)

$(CscNetLib) :  $(CscNetLibObj)
	ar rcs $(CscNetLib) $(CscNetLibObj) 

clean:  
	rm $(CscNetLibObj) $(CscNetLib) 2>/dev/null ; true
