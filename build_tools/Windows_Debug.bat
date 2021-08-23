mkdir cmake_win_debug &
cd cmake_win_debug &
cmake ../../ -DCMAKE_BUILD_TYPE=Debug

cmake --build . --config Debug
cmd /k