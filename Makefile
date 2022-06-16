c=g++

ifeq ($(OS),WINDOWS_NT)
o=output\App.exe
s=src\*.cpp
i=dependencies\GLFW\include
l=dependencies\GLFW\src\libglfw3-win32.a
lWin=$(l)
lad=-lgdi32
r=$(o)
orm=rd /s /q $(o)
else
o=output/App
s=src/*.cpp
i=dependencies/GLFW/include
l=dependencies/GLFW/src/libglfw3.a
lWin=dependencies/GLFW/src/libglfw3-win32.a
lad=
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
	@$(c) -o $(o) $(s) -I$(i) $(l)

win32:
	@echo "Compiling for win32..."
	@x86_64-w64-mingw32-gcc -o $(o).exe $(s) -I$(i) $(lWin) -lgdi32 

clean:
	@echo "Cleaning ..."
	@$(orm)
