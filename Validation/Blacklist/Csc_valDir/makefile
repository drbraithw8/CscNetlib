LIBS :=  -L /usr/local/lib -lCscNet -lpthread

tests: blacklist.c
	gcc blacklist.c $(LIBS) -o tests

clean:
	rm tests
