LIBS :=  -L /usr/local/lib -lCscNet -lpthread

tests: tests.c
	gcc -std=gnu99 tests.c $(LIBS) -o tests

clean:
	rm tests
