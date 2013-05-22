CC = g++
CPPFLAGS = -Wall -g -I./external/include/
LFLAGS = -lGLEW -lGL -lglfw -L./external/lib/ -Wl,-R./../external/lib/
BIN = ./x64/main
SRC = ./space-lion/src
OBJ = \
		$(SRC)/framebufferObject.o \
		$(SRC)/ftvTestbench.o \
		$(SRC)/GLSLProgram.o \
		$(SRC)/material.o \
		$(SRC)/postProcessor.o \
		$(SRC)/renderHub.o \
		$(SRC)/renderParser.o \
		$(SRC)/scene.o \
		$(SRC)/sceneCamera.o \
		$(SRC)/sceneEntity.o \
		$(SRC)/sceneLightSource.o \
		$(SRC)/staticSceneObject.o \
		$(SRC)/texture2D.o \
		$(SRC)/texture3D.o \
		$(SRC)/vertexGeometry.o \
		$(SRC)/volumetricSceneObject.o

DEPENDFILE = .depend

.PHONY: dep extclean

all: $(BIN)
	$(info ~~)
	$(info ~<= SPACE-LION COMPILED =>~)
	$(info ~~)

run: all
	cd ./x64 && ./main

ext: 
	mkdir -p ./external
	cp build_external.makefile external/Makefile
	make -C ./external

extclean: 
	rm -rf ./external

$(BIN): $(OBJ)
	mkdir -p ./x64
	$(CC) $(CPPFLAGS) $(OBJ) space-lion/main.cpp -o $(BIN) $(LFLAGS) $(LIB) 

dep:
	$(CC) -MM $(CPPFLAGS) space-lion/main.cpp space-lion/src/*.cpp space-lion/src/*.h > $(DEPENDFILE)

-include $(DEPENDFILE)

.PHONY: clean
clean:
	rm -rf $(OBJ) $(BIN) $(BIN).o
