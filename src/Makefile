default: build/main

CC     ?= clang
CFLAGS ?= -Wall -Wextra -pedantic -I./include/ -g

RAYLIB_DIR ?= ./include/raylib-5.5_linux_amd64
RAYLIB  = -I$(RAYLIB_DIR)/include -L$(RAYLIB_DIR)/lib -l:libraylib.a -lm 

HEADERS =$(wildcard ./include/*.h)
SOURCES =$(wildcard ./*.c ./include/*.c)

build/cuda.so: cuda.cu
	@mkdir -p build
	nvcc -shared --compiler-options '-fPIC' $(RAYLIB) $^ -o $@

build/main: $(SOURCES) $(HEADERS) build/cuda.so
	mkdir -p build
	$(CC) $(CFLAGS) -xc $(SOURCES) -o $@ $(RAYLIB) -lm

.PHONY: clean
clean:
	rm -fr build

