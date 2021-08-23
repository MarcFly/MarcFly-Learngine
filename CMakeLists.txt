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
set(ML_EXT ${CMAKE_CURRENTS_SOURCE_DIR}/external_libs)

# Engine Source Files
set(ML_SRC
    ${ML_INC}/main.cpp    
)

#------------------------------------------------------------------------------------------------
# OWN LIBRARIES
#------------------------------------------------------------------------------------------------

#------------------------------------------------------------------------------------------------
# EXTERNAL LIBRARIES
#------------------------------------------------------------------------------------------------

#------------------------------------------------------------------------------------------------
# BUILD
#------------------------------------------------------------------------------------------------


add_executable(${ML_TARGET} "${ML_SRC}")

# Windows basics
# How do I make sure it is compiling with Mingw or something like that?
set_target_properties(${ML_TARGET} PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
)

target_include_directories(${ML_TARGET} PUBLIC 
    "${ML_INC}"
)


#------------------------------------------------------------------------------------------------
# GROUP SOURCING FOR IDEs
#------------------------------------------------------------------------------------------------

