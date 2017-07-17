
CscNetLib := libCscNet.a

CscNetLibObj := iniFile.o logger.o netCli.o netSrv.o servBase.o \
					cstr.o signal.o isvalid.o fileProperties.o \
					std.o alloc.o hash.o list.o memcheck.o json.o

LIBS= 

.c.o :
	gcc -c  $<

all: $(CscNetLib)

$(CscNetLib) :  $(CscNetLibObj)
	ar rcs $(CscNetLib) $(CscNetLibObj) 

clean:  
	rm $(CscNetLibObj) $(CscNetLib)
