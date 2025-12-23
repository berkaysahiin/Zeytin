#!/bin/bash

rm -r bin build && premake5 gmake && cd build && bear -- make -j && cd ..
