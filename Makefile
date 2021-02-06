GCC=g++
ARGS=-std=c++17

default: build

setup:
	@mkdir -pv build

build:
	$(GCC) $(ARGS) "./main.cpp" -o "./build/flitdb_test"

run:
	@cd build; ./flitdb_test

clean:
	@rm -fv build/flitdb_test