
all: GLFW GLEW GLM
	$(info ~~)
	$(info ~<= FINISHED CRAFTING TOOLS =>~)
	$(info ~~)
	$(info If you see this, everything went according to plan.)
	$(info Have fun compiling space-lion by simply executing: make)
	$(info ~~)

WGET:
	rm -rf ./wget-git
	git clone git://git.savannah.gnu.org/wget.git wget-git
	cd wget-git && ./bootstrap
	cd wget-git && ./configure
	make -C wget-git
	$(info (export PATH="${PWD}/wget-git/src/:${PATH}"))

#GLFW = ./lib/libglfw.so ./include/glfw.h
# # # 
GLFW: 
	rm -rf ./glfw*
	wget http://sourceforge.net/projects/glfw/files/glfw/2.7.7/glfw-2.7.7.zip
	unzip glfw-2.7.7.zip
	cd glfw-2.7.7/ && sh compile.sh
	make -C glfw-2.7.7 x11
	mkdir -p ./lib
	mkdir -p ./include
	cp glfw-2.7.7/lib/x11/libglfw.so ./lib
	cp -r glfw-2.7.7/include/GL ./include

#GLEW = ./lib/libGLEW.so* ./include/glew.h
# # #
GLEW:
	rm -rf ./glew-git
	git clone git://glew.git.sourceforge.net/gitroot/glew/glew glew-git
	make -C glew-git extensions
	make -C glew-git glew.lib
	mkdir -p ./lib
	mkdir -p ./include
	cp -r glew-git/include/GL ./include
	cp glew-git/lib/libGLEW.so* ./lib

#GLM = ./include/glm
# # #
GLM:
	rm -rf ./glm-git
	git clone git://github.com/g-truc/glm.git glm-git
	mkdir -p ./include
	cp -r glm-git/glm ./include

clean: 
	rm -rf ./lib ./include ./glm* ./glew* ./glfw* ./wget*

