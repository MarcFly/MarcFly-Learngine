cmake_minimum_required(VERSION 3.1)

project("MarcFly Libs" VERSION 2022.1 LANGUAGES C CXX)

file(GLOB_RECURSE mfly_libs_src 
        ${CMAKE_CURRENT_SOURCE_DIR}/mfly_plugin/*.h 
        ${CMAKE_CURRENT_SOURCE_DIR}/mfly_plugin/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mfly_plugin/*.hpp )

set(ML_LIBS_SRC "${mfly_libs_src}" PARENT_SCOPE)