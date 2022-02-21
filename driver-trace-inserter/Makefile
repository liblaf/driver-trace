PREFIX      ?= $(PWD)
BIN_PATH     = $(PREFIX)/bin
LLVM_CONFIG ?= llvm-config-12

CC              =   clang-12
CXX             =   clang++-12
CXXFLAGS        +=  -O0 -g -Wall `$(LLVM_CONFIG) --cxxflags` -fno-rtti -fpic -std=c++17
LDFLAGS         +=  `$(LLVM_CONFIG) --ldflags` `$(LLVM_CONFIG) --libs bitreader bitwriter interpreter core irreader mcjit native option support`

PROGS = driver-trace-inserter

all: before_build $(PROGS)
	@echo $(GREEN)[+] All done!"\e[0m"

before_build:
	mkdir --parents $(BIN_PATH)

driver-trace-inserter: main.cc utils.cc instrument.cc
	$(CXX) $(CXXFLAGS) $^ -o $(BIN_PATH)/$@ $(LDFLAGS)

.NOTPARALLEL: clean

clean:
	rm -rf $(BIN_PATH)
