#!/bin/bash

c++ main.cpp `pkg-config --cflags --libs sdl2` -o dist/build/augen
