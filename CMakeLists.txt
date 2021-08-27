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
)

#------------------------------------------------------------------------------------------------
# BUILD
#------------------------------------------------------------------------------------------------

# CrossWindow testing
#add_executable(${ML_TARGET} "${ML_SRC}")
add_subdirectory(${ML_EXT}/CrossWindow)
xwin_add_executable(${ML_TARGET} "${ML_SRC}")
target_link_libraries(${ML_TARGET} PUBLIC CrossWindow)

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
)

#------------------------------------------------------------------------------------------------
# EXTERNAL LIBRARIES
#------------------------------------------------------------------------------------------------

# enkiTS - Multithreading
set(enkiTS_INC "${ML_EXT}/enkiTS/src")
target_include_directories(${ML_TARGET} PUBLIC ${enkiTS_INC})
target_sources(${ML_TARGET} PUBLIC "${enkiTS_INC}/TaskScheduler.cpp")

#------------------------------------------------------------------------------------------------
# GROUP SOURCING FOR IDEs
#------------------------------------------------------------------------------------------------

#source_group()