CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -fno-exceptions
SRC = src/grp_96_main.cpp src/grp_96_erp.cpp
HDR = src/grp_96_erp.h
.PHONY: all clean test
all: acaderp
acaderp: $(SRC) $(HDR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $@
clean:
	rm -f acaderp src/*.o
test: acaderp
	./run_tests.sh
