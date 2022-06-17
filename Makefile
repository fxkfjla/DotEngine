ifeq ($(OS),WINDOWS_NT)
out=output\App.exe
src=src\*.cpp

iGLFW=dependencies\GLFW\include
lGLFW=dependencies\GLFW\lib\libglfw3dll.a
lGLFWWin=$(l)
iVK=dependencies\VULKAN\include
lVK=dependencies\VULKAN\lib\libvulkan.so
lVKWin=dependencies\VULKAN\lib\vulkan-1.dll
lGLM=dependencies\GLM\include

r=$(out)
CLEAN=rd /s /q output\*
else
out=output/App
src=src/*.cpp

iGLFW=dependencies/GLFW/include
lGLFW=dependencies/GLFW/lib/libglfw3.a
lGLFWWin=dependencies/GLFW/lib/libglfw3dll.a
iVK=dependencies/VULKAN/include
lVK=dependencies/VULKAN/lib/libvulkan.so
lVKWin=dependencies/VULKAN/lib/vulkan-1.dll
lGLM=dependencies/GLM/include

r=./$(out)
CLEAN=rm -f output/*
endif

CXX=g++
CFLAGS=
IDFLAGS=-I$(iGLFW) -I$(iVK) -I$(lGLM) 
LDFLAGS=$(lGLFW) $(lVK)
LDFLAGSWIN=$(lGLFWWin) $(lVKWin)
WINDEP=-static-libgcc -static-libstdc++ -lgdi32 -static -lpthread

all: clean compile run

all-win32: clean compile-win32 run-win32

run:
	@echo "Running app..."
	@$(r)
	@echo ""

run-win32:
	@echo "Running app..."
	@$(out).exe
	@echo ""

compile:
	@echo "Compiling app..."
	@$(CXX) -o $(out) $(src) $(IDFLAGS) $(LDFLAGS)

compile-win32:
	@echo "Compiling for win32..."
	@x86_64-w64-mingw32-g++ -o $(out).exe $(src) $(IDFLAGS) $(LDFLAGSWIN) $(WINDEP) 

clean:
	@echo "Cleaning ..."
	@$(CLEAN)
