c=g++
CFLAGS=
LDFLAGS=

ifeq ($(OS),WINDOWS_NT)
o=output\App.exe
s=src\*.cpp

iGLFW=dependencies\GLFW\include
lGLFW=dependencies\GLFW\lib\libglfw3dll.a
lGLFWWin=$(l)
iVK=dependencies\VULKAN\include
lVK=dependencies\VULKAN\lib\libvulkan.so
lVKWin=dependencies\VULKAN\lib\vulkan-1.dll

r=$(o)
orm=rd /s /q $(o)
else
o=output/App
s=src/*.cpp

iGLFW=dependencies/GLFW/include
lGLFW=dependencies/GLFW/lib/libglfw3.a
lGLFWWin=dependencies/GLFW/lib/libglfw3dll.a
iVK=dependencies/VULKAN/include
lVK=dependencies/VULKAN/lib/libvulkan.so
lVKWin=dependencies/VULKAN/lib/vulkan-1.dll

r=./$(o)
orm=rm -f $(o)
endif

all: clean compile run

run:
	@echo "Running app..."
	@$(r)
	@echo ""

compile:
	@echo "Compiling app..."
	@$(c) -o $(o) $(s) -I$(iGLFW) -I$(iVK) $(lGLFW) $(lVK)

compile-win32:
	@echo "Compiling for win32..."
	@x86_64-w64-mingw32-gcc -o $(o).exe $(s) -I$(iGLFW) -I$(iVK) $(lGLFWWin) $(lVKWin) -lgdi32

clean:
	@echo "Cleaning ..."
	@$(orm)
