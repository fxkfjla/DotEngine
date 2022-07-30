all: configure build run

build: build_debug

run: run_debug

configure:
	cmake -S . -B build/release -DCMAKE_BUILD_TYPE=RELEASE
	cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=DEBUG

build_debug:
	cmake --build build/debug

run_debug:
	build/debug/app/App

build_release:
	cmake --build build/release

run_release:
	build/release/app/App
