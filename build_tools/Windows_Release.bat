mkdir cmake_win_debug &
cd cmake_win_debug &
cmake ../../ -DCMAKE_BUILD_TYPE=Release

cmake --build . --config Release
cmd /k