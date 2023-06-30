CHECKER_SRC = $(wildcard src/checkers/*.cpp)
CHECKER_BIN = $(patsubst src/checkers/%.cpp, bin/chk-%, $(CHECKER_SRC))

src/checkers/testlib.h.gch: src/checkers/testlib.h
	g++ src/checkers/testlib.h -std=c++17 -O2

bin/chk-%: src/checkers/%.cpp src/checkers/testlib.h.gch
	g++ $< -o $@ -std=c++17 -O2
	strip $@.exe

checkers: $(CHECKER_BIN)

CORE_HDR = $(wildcard src/core/*.h)
CORE_SRC = $(wildcard src/core/*.cpp)
CORE_OBJ = $(patsubst src/core/%.h, obj/%.o, $(CORE_HDR))

CXX_OPT = -std=c++17 -Isrc/include -Isrc/core -O2 -Wall

obj/cleaner.o: obj/options.o src/include/lib.hpp

obj/tester.o: src/include/lib.hpp src/include/runner.hpp

obj/%.o: src/core/%.cpp src/core/%.h
	g++ $< -o $@ -c $(CXX_OPT)

bin/stress: src/main.cpp $(CORE_OBJ)
	g++ src/main.cpp $(CORE_OBJ) -o bin/stress $(CXX_OPT)

all: bin/stress

clean:
	-rm -rf $(CHECKER_BIN) $(CORE_OBJ) bin/stress

.DEFAULT_GOAL := all