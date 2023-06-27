CHECKER_SRC = $(wildcard src/checkers/*.cpp)
CHECKER_BIN = $(patsubst src/checkers/%.cpp, bin/chk-%, $(CHECKER_SRC))

src/checkers/testlib.h.gch: src/checkers/testlib.h
	g++ src/checkers/testlib.h -std=c++17 -O2

bin/chk-%: src/checkers/%.cpp src/checkers/testlib.h.gch
	g++ $< -o $@ -std=c++17 -O2
	strip $@.exe

checkers: $(CHECKER_BIN)

MODULES_HDR = $(wildcard src/*.h)
MODULES_SRC = $(patsubst src/%.h, src/%.cpp, $(MODULES_HDR))
MODULES_OBJ = $(patsubst src/%.h, obj/%.o, $(MODULES_HDR))

CXX_OPT = -std=c++17 -Isrc/include -O2 -Wall

obj/options.o: src/options.cpp src/options.h src/include/lib.hpp
	g++ $< -o $@ -c $(CXX_OPT)

obj/cleaner.o: src/cleaner.cpp obj/options.o src/cleaner.h src/include/lib.hpp
	g++ $< -o $@ -c $(CXX_OPT)

obj/helper.o: src/helper.cpp obj/options.o src/helper.h src/include/lib.hpp
	g++ $< -o $@ -c $(CXX_OPT)

obj/tester.o: src/tester.cpp obj/options.o src/tester.h src/include/lib.hpp src/include/runner.hpp
	g++ $< -o $@ -c $(CXX_OPT)

bin/stress: src/main.cpp $(MODULES_OBJ)
	g++ src/main.cpp $(MODULES_OBJ) -o bin/stress $(CXX_OPT)

all: bin/stress

clean:
	-rm -rf $(CHECKER_BIN) $(MODULES_OBJ) bin/stress

.DEFAULT_GOAL := all