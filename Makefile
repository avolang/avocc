CC=gcc
CCFLAGS=-g -fPIC -std=c11 -pedantic
tests:
	mkdir -p bin
	$(CC) $(CCFLAGS) -o bin/avocc_tests avocc.c tests.c
	cp ./bin/avocc_tests /tmp/a.out
	./bin/avocc_tests

clean:
	rm ./bin/avocc_tests
