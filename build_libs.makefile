
#.PHONY: WGET GLFW GLEW GLM COPY clean

all: WGET GLFW GLEW GLM COPY
	$(info ~~)
	$(info ~<= FINISHED CRAFTING TOOLS =>~)
	$(info ~~)
	$(info If you see this, everything went according to plan.)
	$(info Have fun compiling space-lion by simply executing: make)
	$(info ~~)

WGET:
	$(info ~~)
	$(info ~<= BUILDING WGET =>~)
	$(info ~~)
	git clone git://git.savannah.gnu.org/wget.git wget-git
	cd wget-git && ./bootstrap
	cd wget-git && ./configure
	make -C wget-git


GLFW:
	$(info ~~)
	$(info ~<= BUILDING GLFW 2.7.7 =>~)
	$(info ~~)
	export PATH="${PWD}/lib/wget-git/src/:${PATH}" &&  \
	wget http://sourceforge.net/projects/glfw/files/glfw/2.7.7/glfw-2.7.7.zip
	unzip glfw-2.7.7.zip
	cd glfw-2.7.7/ && sh compile.sh
	make -C glfw-2.7.7 x11

GLEW:
	$(info ~~)
	$(info ~<= BUILDING GLEW =>~)
	$(info ~~)
	git clone git://glew.git.sourceforge.net/gitroot/glew/glew glew-git
	export PATH="${PWD}/lib/wget-git/src/:${PATH}" &&  \
	make -C glew-git extensions
	export PATH="${PWD}/lib/wget-git/src/:${PATH}" &&  \
	make -C glew-git glew.lib

GLM:
	$(info ~~)
	$(info ~<= DOWNLOADING GLM =>~)
	$(info ~~)
	git clone git://github.com/g-truc/glm.git glm-git

COPY:
	$(info ~~)
	$(info ~<= COPYING FILES =>~)
	$(info ~~)
	cp glfw-2.7.7/lib/x11/libglfw.so .
	cp -r glfw-2.7.7/include/GL .
	cp -r glm-git/glm .
	cp -r glew-git/include/GL .
	cp glew-git/lib/libGLEW.so .

clean: 
	rm -rf glfw-2.7.7.zip glfw-2.7.7 glew-git glm-git glm GL libglfw.so libGLEW.so

