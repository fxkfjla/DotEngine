srcGLFW=dependencies/GLFW/src/libglfw3.a
inGLFW=dependencies/GLFW/include

all: clean compile run

run:
	@echo "Running app..."
	@./output/App
	@echo ""

compile:
	@g++ -o output/App src/*.cpp -I$(inGLFW) $(srcGLFW)
	@echo "Compiling..."

clean:
	@rm -f output/App
	@echo "Cleaning..."
