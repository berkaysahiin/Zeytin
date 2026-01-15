#!/bin/bash

mkdir -p build-standalone
cd build-standalone
cmake .. -DENGINE_CONFIG=STANDALONE -G Ninja
cmake --build . -j
