default: build/main

CC     ?= clang
CFLAGS ?= -Wall -Wextra -I./include/ -g

RAYLIB_DIR ?= ./include/raylib-5.5_linux_amd64
RAYLIB  = -I$(RAYLIB_DIR)/include -L$(RAYLIB_DIR)/lib -l:libraylib.a -lm 

HEADERS =$(wildcard ./include/*.h)
SOURCES =$(wildcard ./*.c ./include/*.c)

build/cuda.so: cuda.cu
	@mkdir -p build
	nvcc -shared --compiler-options '-fPIC -O3' $(RAYLIB) $^ -o $@

build/sync.so: sync.c 
	mkdir -p build
	$(CC) $(CFLAGS) -shared -fPIC -O0 -xc $^ -o $@ $(RAYLIB) -lm

build/main: main.c include/graph.c $(HEADERS) build/cuda.so build/sync.so circle.data
	mkdir -p build
	$(CC) $(CFLAGS) -O0 -xc main.c include/graph.c -o $@ $(RAYLIB) -lm

.PHONY: clean
clean:
	rm -fr build

