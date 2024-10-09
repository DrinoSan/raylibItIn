#!/bin/sh

set -xe

clang -std=c11 `pkg-config --cflags raylib` -o main main.c `pkg-config --libs raylib` -lm -lpthread -L/opt/homebrew/lib -lglfw -framework CoreFoundation -framework Cocoa -framework IOKit -framework OpenGL -Wall

