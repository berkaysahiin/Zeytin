mkdir -p build && 
cd build &&
cmake .. -G Ninja &&
cmake --build . -j &&
cp preparser ../../engine/

