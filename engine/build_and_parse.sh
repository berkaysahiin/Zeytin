mkdir -p build &&
cd build &&
cmake .. -G Ninja && 
cmake --build . -j &&
cd .. &&
./preparser build && 
cd build &&
cmake --build . -j
