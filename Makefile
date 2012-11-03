#
# http://www.gnu.org/software/make/manual/make.html
#
CPP=g++
INCLUDES=
CFLAGS=-Wall -ggdb
LDFLAGS=
EXE=example.out detect-au.out
SRC=DtmfDetector.cpp DtmfGenerator.cpp
OBJ=$(patsubst %.cpp,obj/%.o,$(SRC))

#
# This is here to prevent Make from deleting secondary files.
#
.SECONDARY:
	
debug: CFLAGS += -DDEBUG=1
debug: all

#
# $< is the first dependency in the dependency list
# $@ is the target name
#
all: dirs $(addprefix bin/, $(EXE)) tags


dirs:
	mkdir -p obj
	mkdir -p bin

tags: *.cpp *.hpp
	ctags *.cpp *.hpp

bin/%.out: obj/%.o $(OBJ)
	$(CPP) $(CFLAGS) $< $(OBJ) $(LDFLAGS) -o $@

obj/%.o : %.cpp
	$(CPP) $(CFLAGS) $< $(INCLUDES) -c -o $@

clean:
	rm -f obj/*
	rm -f bin/*
	rm -f tags
