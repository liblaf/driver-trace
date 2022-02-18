PREFIX      ?= $(PWD)
BIN_PATH     = $(PREFIX)/bin
LLVM_CONFIG ?= llvm-config-13

CC              =   clang-13
CXX             =   clang++-13
CXXFLAGS        +=  -O0 -g -Wall `$(LLVM_CONFIG) --cxxflags` -fno-rtti -fpic -std=c++17
LDFLAGS         +=  `$(LLVM_CONFIG) --ldflags` `$(LLVM_CONFIG) --libs bitreader bitwriter interpreter core irreader mcjit native option support`

PROGS = plain-clang-wrapper

all: before_build $(PROGS)
	@echo $(GREEN)[+] All done!"\e[0m"

before_build:
	mkdir --parents $(BIN_PATH)

plain-clang-wrapper: plain-clang-wrapper.cc
	$(CXX) $(CXXFLAGS) $^ -o $(BIN_PATH)/$@ $(LDFLAGS)

.NOTPARALLEL: clean

clean:
	rm -rf $(BIN_PATH)
