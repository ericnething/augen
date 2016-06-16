#!/bin/bash

c++ main.cpp -std=c++11 `pkg-config --cflags --libs sdl2` -o dist/build/augen
