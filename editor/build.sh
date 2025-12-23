#!/bin/bash

premake5 gmake && cd build && bear -- make -j && cd ..
