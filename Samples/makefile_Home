.c.o:
	gcc -c -std=gnu99 -I $(HOME)/include  $<

LIBS :=  -L $(HOME)/lib -lCscNet -lpthread

all: netCliDemo netSrvDemo servBaseDemo filePropertiesDemo \
	 parseWordsOnLines iniDemo logDemo jsonDemo

netCliDemo: netCliDemo.o
	gcc netCliDemo.o $(LIBS) -o netCliDemo

netSrvDemo: netSrvDemo.o
	gcc netSrvDemo.o $(LIBS) -o netSrvDemo

servBaseDemo: servBaseDemo.o
	gcc servBaseDemo.o $(LIBS) -o servBaseDemo

filePropertiesDemo: filePropertiesDemo.o
	gcc filePropertiesDemo.o $(LIBS) -o filePropertiesDemo

parseWordsOnLines: parseWordsOnLines.o
	gcc parseWordsOnLines.o $(LIBS) -o parseWordsOnLines

logDemo: logDemo.o
	gcc logDemo.o $(LIBS) -o logDemo

iniDemo: iniDemo.o
	gcc iniDemo.o $(LIBS) -o iniDemo

jsonDemo: jsonDemo.o
	gcc jsonDemo.o $(LIBS) -o jsonDemo

clean:
	rm netSrvDemo servBaseDemo parseWordsOnLines filePropertiesDemo \
        netCliDemo iniDemo logDemo jsonDemo *.o test.log

