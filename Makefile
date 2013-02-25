CC = g++
CPPFLAGS = -Wall -g -lGL -lglfw
LFLAGS = -Il /usr/lib/libGLEW.so
BIN = ./x64/main
OBJ = space-lion/src/GLSLProgram.o space-lion/src/material.o space-lion/src/renderHub.o space-lion/src/sceneCamera.o space-lion/src/sceneEntity.o space-lion/src/scene.o space-lion/src/sceneLightSource.o space-lion/src/staticSceneObject.o space-lion/src/texture.o space-lion/src/vertexGeometry.o space-lion/src/renderParser.o
DEPENDFILE = .depend

all: $(BIN)

$(BIN): $(OBJ)
	mkdir -p ./x64
	$(CC) $(CPPFLAGS) $(OBJ) space-lion/main.cpp -o $(BIN) $(LFLAGS)

.PHONY: dep
dep:
	$(CC) -MM $(CPPFLAGS) space-lion/main.cpp space-lion/src/*.cpp space-lion/src/*.h > $(DEPENDFILE)

-include $(DEPENDFILE)

.PHONY: clean
clean:
	rm -rf $(OBJ) $(BIN) $(BIN).o
