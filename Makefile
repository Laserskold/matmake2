# Bootstrap makefile
# Guess compiler and build if anything is changed

build/matmake2: src/*.cpp src/*.h src/main/*.cpp
	@git submodule update --init
	@./build-linux.sh

install: build/matmake2
	sudo cp -i build/matmake2 /usr/bin/matmake2 

install-link: build/matmake2
	sudo ln -i build/matmake2 /usr/bin/matmake2

uninstall:
	sudo rm /usr/bin/matmake2
