CC=cc
tests:
	mkdir -p bin
	$(CC) -g -o bin/avocc_tests avocc.c tests.c
	./bin/avocc_tests

clean:
	rm ./bin/*
