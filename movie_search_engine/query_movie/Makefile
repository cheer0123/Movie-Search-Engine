# define the commands we'll use for compilation and library building
AR = ar
ARFLAGS = rcs
CC = gcc

GOOGLE_TEST_INCLUDE=${HOME}/src/googletest-release-1.8.0/googletest/include/

# define useful flags to cc/ld/etc.
CFLAGS = -g -Wall -I. -I.. -Ihtll 
LDFLAGS = -L. 


#define common dependencies
OBJS = MovieSet.o DocIdMap.o FileParser.o FileCrawler.o MovieIndex.o Assert007.o Movie.o QueryProcessor.o MovieReport.o
HEADERS = FileParser.h FileCrawler.h DocIdMap.h MovieIndex.h MovieSet.h Movie.h Assert007.h MovieReport.h


# compile everything
all: main test indexer test_movieindex


main: main.c $(OBJS)
	gcc -Wall -g -o main main.c $(OBJS) -L. libHtll.a
	@echo ===========================
	@echo Run main by running ./main dir/
	@echo \(where dir is the directory to look for files to index\)
	@echo ===========================

indexer: example_indexer.c $(OBJS)
	gcc -Wall -g -o indexer example_indexer.c $(OBJS) -L. libHtll.a
	@echo ===========================
	@echo Run indexer by running ./indexer -X filename
	@echo \(X is the field to index by, filename is the file of movies to index\)
	@echo ===========================

%.o: %.c $(HEADERS) FORCE
	$(CC) $(CFLAGS) -c $<

test: test_suite.o $(OBJS)
	g++ -g -o test_suite test_suite_a4.o $(OBJS) -L. libHtll.a \
		 -L${HOME}/lib/gtest -lgtest -lpthread
	@echo ===========================
	@echo Run tests by running ./test_suite
	@echo ===========================

test_suite.o: test_suite_a4.cc
	g++ -g -c -Wall -I $(GOOGLE_TEST_INCLUDE) test_suite_a4.cc \
		-o test_suite_a4.o


test_movieindex.o: test_movieindex.cc
	g++ -g -c -Wall -I $(GOOGLE_TEST_INCLUDE) test_movieindex.cc \
		-o test_movieindex.o


test_movieindex:  $(GOOGLE_TEST_LIB) test_movieindex.o $(OBJS)
	@echo ===========================
	@echo Building movie index test
	@echo ===========================
	g++ -o test_movieindex test_movieindex.o $(OBJS) -L. libHtll.a  \
		-L${HOME}/lib/gtest -lgtest -lpthread
	@echo ===========================
	@echo Run tests by running ./test_movieindex
	@echo ===========================

clean: FORCE
	/bin/rm -f *.o *~ main test_suite indexer test_movieindex

FORCE:
