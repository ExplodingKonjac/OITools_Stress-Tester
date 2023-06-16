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

obj/core.o: src/core.cpp src/core.h src/include/lib.hpp
	g++ $< -o $@ -c -std=c++17 -Isrc/include/ -O2

obj/cleaner.o: src/cleaner.cpp src/cleaner.h src/include/lib.hpp
	g++ $< -o $@ -c -std=c++17 -Isrc/include/ -O2

obj/helper.o: src/helper.cpp src/helper.h src/include/lib.hpp
	g++ $< -o $@ -c -std=c++17 -Isrc/include/ -O2

obj/tester.o: src/tester.cpp src/tester.h src/include/lib.hpp src/include/runner.hpp
	g++ $< -o $@ -c -std=c++17 -Isrc/include/ -O2

bin/stress: src/main.cpp src/include/lib.hpp src/include/runner.hpp $(MODULES_OBJ)
	g++ src/main.cpp $(MODULES_OBJ) -o bin/stress -std=c++17 -O2 -Wall -Isrc/include/

all: bin/stress

clean:
	-rm -rf $(CHECKER_BIN) $(MODULES_OBJ) bin/stress

.DEFAULT_GOAL := all