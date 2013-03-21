CC = g++
CPPFLAGS = -Wall -g -Llib/ -Ilib/ -Wl,-Rlib/
LFLAGS = -lGLEW -lGL -lglfw
BIN = ./x64/main
SRC = space-lion/src
OBJ = $(SRC)/GLSLProgram.o \
		$(SRC)/material.o \
		$(SRC)/renderHub.o \
		$(SRC)/sceneCamera.o \
		$(SRC)/sceneEntity.o \
		$(SRC)/scene.o \
		$(SRC)/sceneLightSource.o \
		$(SRC)/staticSceneObject.o \
		$(SRC)/texture.o \
		$(SRC)/vertexGeometry.o \
		$(SRC)/renderParser.o
DEPENDFILE = .depend

.PHONY: dep lib

all: $(BIN)
	$(info ~~)
	$(info ~<= SPACE-LION COMPILED =>~)
	$(info ~~)

lib: 
	mkdir -p lib
	export PATH="${PWD}/lib/wget-git/src:${PATH}"
	cp build_libs.makefile lib/Makefile
	make -C lib

libclean: 
	rm -rf lib

.PHONY: help
help:
	$(info ~~)
	$(info ~<= HELP =>~)
	$(info ~~)
	$(info If you are using a Linux computer of which you are not)
	$(info 	sure whether it has installed the required libraries)
	$(info 	first try simply compiling space-lion itself. )
	$(info 	If compiling space-lion fails (there is no 'main' in ./x86))
	$(info 	to download and compile needed libraries)
	$(info 	execute: 	)
	$(info 				make lib)
	$(info )
	$(info To build space-lion )
	$(info 	execute:)
	$(info 				make)
	$(info )
	$(info To clear all compiled space-lion objects, )
	$(info 	execute: )
	$(info 				make clean)
	$(info )
	$(info To delete built libraries)
	$(info 	execute: )
	$(info 				make libclean)
	$(info )

$(BIN): $(OBJ)
	mkdir -p ./x64
	$(CC) $(CPPFLAGS) $(OBJ) space-lion/main.cpp -o $(BIN) $(LFLAGS) $(LIB) 

dep:
	$(CC) -MM $(CPPFLAGS) space-lion/main.cpp space-lion/src/*.cpp space-lion/src/*.h > $(DEPENDFILE)

-include $(DEPENDFILE)

.PHONY: clean
clean:
	rm -rf $(OBJ) $(BIN) $(BIN).o
