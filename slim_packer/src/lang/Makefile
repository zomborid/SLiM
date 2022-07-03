.PHONY: clean checks

CC=g++
CFLAGS=-std=c++14 -DBUILD_DATE=$(BUILD_DATE)

CPP_FILES=$(wildcard *.cpp)
OBJ_FILES=$(CPP_FILES:.cpp=.o)

default: checks lang.o

# $^ is all the dependencies
# $< is the first dependency
# $@ is the target name
lang.o: $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

checks:
	ifndef BOOST_ROOT
		$(error BOOST_ROOT is undefined)
	endif

clean:
	$(RM) $(OBJ_FILES) lang.o