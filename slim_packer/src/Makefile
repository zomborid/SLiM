.PHONY: clean checks

CC=g++
CFLAGS =-std=c++14 -DBUILD_DATE=$(BUILD_DATE)
CFLAGS+=-Ilang

CPP_FILES=$(wildcard *.cpp)
OBJ_FILES=$(CPP_FILES:.cpp=.o)

default: checks $(RUNNABLE)

# $^ is all the dependencies
# $< is the first dependency
# $@ is the target name
$(RUNNABLE): $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

checks:
# currently filesystem library is not supported for gcc on windows, compile with something else?
	ifneq  ($(PLATFORM),Linux)
		$(error currently filesystem library is not available for gcc on windows, alter makefile to use other compiler)
	endif

clean:
	#$(RM) $(OBJ_FILES) $(RUNNABLE)
	$(MAKE) -C lang clean