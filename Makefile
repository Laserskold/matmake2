# Bootstrap makefile
# Guess compiler and build if anything is changed

build/matmake2: src/*.cpp src/*.h src/main/*.cpp
	@git submodule update --init
	@./build-linux.sh

  
