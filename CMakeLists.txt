cmake_minimum_required(VERSION 3.1)

#------------------------------------------------------------------------------------------------
# HELPER MODULES
#------------------------------------------------------------------------------------------------
set(ML_CMAKE ${CMAKE_CURRENT_SOURCE_DIR}/cmake PARENT_SCOPE)

#------------------------------------------------------------------------------------------------
# SETUP CURRENT PROJECT
#------------------------------------------------------------------------------------------------
project("MarcFly Learngine" VERSION 2021.1
                            DESCRIPTION "C++ Personal Game Engine for learning and trying new things"
                            LANGUAGES C CXX)
set(ML_TARGET "MarcFly_Learngine")

# C / CXX Config

# Base Include Directiories
set(ML_INC ${CMAKE_CURRENT_SOURCE_DIR}/engine)
set(ML_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/libs)
set(ML_EXT ${CMAKE_CURRENT_SOURCE_DIR}/submodules)

# Engine Source Files
set(ML_SRC
    ${ML_INC}/main.cpp
    ${ML_INC}/mfly_window/mfly_window.cpp
    "${ML_EXT}/enkiTS/src/TaskScheduler.cpp"
    ${ML_LIBS}/mfly_globals.h
    ${ML_LIBS}/sque_timer/sque_timer.h
    ${ML_INC}/mfly_gpu/mfly_gpu.cpp
    ${ML_INC}/mfly_shaders/mfly_shaders.cpp
)

# Libraries
#file(GLOB_RECURSE ML_LIBS_SRC ${ML_LIBS}/*.cpp ${ML_LIBS}/*.h ${ML_LIBS}/.hpp)

add_subdirectory(${ML_LIBS})
message("Printing mfly libs: ${ML_LIBS_SRC}")
#set(ML_LIBS_SRC
#    ${ML_LIBS}/mfly_plugin/mfly_dlloading.h
#    ${ML_LIBS}/mfly_plugin/mfly_plugin.h)
#------------------------------------------------------------------------------------------------
# BUILD
#------------------------------------------------------------------------------------------------

# CrossWindow testing
set(XGFX_API VULKAN CACHE STRING "API TO USE, currently vulkan only, will depend on backed if there are more in the future")
set_property(
    CACHE
    XGFX_API PROPERTY
    STRINGS NOOP VULKAN OPENGL DIRECTX12 METAL
)

add_subdirectory(${ML_EXT}/CrossWindow)
set_property(TARGET CrossWindow PROPERTY FOLDER "Dependencies")

add_subdirectory(${ML_EXT}/CrossWindow-Graphics ${CMAKE_BINARY_DIR}/crosswindow-graphics)

xwin_add_executable(${ML_TARGET} "${ML_SRC}")
target_link_libraries(${ML_TARGET} PUBLIC CrossWindow CrossWindowGraphics)
message("${ML_LIBS_SRC}")
target_sources(${ML_TARGET} PUBLIC "${ML_LIBS_SRC}")
message("${ML_LIBS_SRC}")

# Windows basics
if(RUNTIME)
    set_target_properties(${ML_TARGET} PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )
elseif()
    set_target_properties(${ML_TARGET} PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
endif()

target_include_directories(${ML_TARGET} PUBLIC 
    "${ML_INC}"
    "${ML_EXT}"
    ${ML_LIBS}/sque_timer
)

#------------------------------------------------------------------------------------------------
# EXTERNAL LIBRARIES
#------------------------------------------------------------------------------------------------

# enkiTS - Multithreading
set(enkiTS_INC "${ML_EXT}/enkiTS/src")
target_include_directories(${ML_TARGET} PUBLIC ${enkiTS_INC})
target_sources(${ML_TARGET} PUBLIC "${enkiTS_INC}/TaskScheduler.cpp")


# VULKAN THINGIES
# Must setup Vulkan_LIBRARY environment variable for user, not system
# Vulkan SDK installer does not set it, same with VULKAN_INCLUDE_DIR ideally
message("Looking for Vulkan")
set(Vulkan_LIBRARY "$ENV{VULKAN_SDK}\\Lib") # Have to set it in the env... so bad
set(ENV{Vulkan_LIBRARY} "$ENV{VULKAN_SDK}\\Lib") # Have to set it in the env... so bad

find_package(Vulkan REQUIRED)
message("${VULKAN_INCLUDE_DIRS}")
message("Linking with Vulkan")
target_include_directories(${ML_TARGET} PUBLIC
    "${Vulkan_INCLUDE_DIRS}"
    "$ENV{VULKAN_SDK}/include")
target_link_directories(${ML_TARGET} PUBLIC
    "${Vulkan_LIBRARIES}")
target_link_libraries(${ML_TARGET} PUBLIC
    "${Vulkan_LIBRARIES}/vulkan-1.lib"# How annoying
    "${Vulkan_LIBRARIES}/shaderc_combinedd.lib")
message("Libs hello? ${Vulkan_LIBS_LIST}")

# XWIN-GRAPHICS Vulkan Linking
if(XGFX_API STREQUAL "VULKAN")
    find_path(VULKAN_INCLUDE_DIR NAMES vulkan/vulkan.h HINTS
        "$ENV{VULKAN_SDK}/include"
        "$ENV{VULKAN_SDK}/Include"
        "$ENV{VK_SDK_PATH}/Include")
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        find_library(XGFX_LIBRARY
            NAMES vulkan-1 vulkan vulkan.1
            HINTS
            "$ENV{VULKAN_SDK}/lib"
            "$ENV{VULKAN_SDK}/Lib"
            "$ENV{VULKAN_SDK}/Bin"
            "$ENV{VK_SDK_PATH}/Bin")
    else()
        find_library(XGFX_LIBRARY
                    NAMES vulkan-1 vulkan vulkan.1
                    HINTS
            "$ENV{VULKAN_SDK}/Lib32"
            "$ENV{VULKAN_SDK}/Bin32"
            "$ENV{VK_SDK_PATH}/Bin32")
    endif()
elseif(XGFX_API STREQUAL "OPENGL")
    add_subdirectory(../../external/glad ${CMAKE_BINARY_DIR}/glad)
    target_include_directories(
        Glad
        PUBLIC ../../external/opengl-registry/api
    )
    set(XGFX_LIBRARY Glad)
    set_property(TARGET Glad PROPERTY FOLDER "Dependencies")
elseif(XGFX_API STREQUAL "METAL")
    find_library(XGFX_LIBRARY Metal)
endif()


#------------------------------------------------------------------------------------------------
# GROUP SOURCING FOR IDEs
#------------------------------------------------------------------------------------------------

#source_group()