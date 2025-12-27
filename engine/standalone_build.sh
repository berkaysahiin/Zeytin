rmdir bin build
mkdir build
cd build
cmake .. -DENGINE_CONFIG=standalone -G Ninja
cmake --build . -j
