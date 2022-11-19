LibName = "MarcFly_Learngine_LIB"
AppName = "Distributable"
LibInclude = "%{LibName}/src"

workspace "MarcFly_Learngine"
    location "./"
    configurations { "Debug", "Release", "Distribution" }
    architecture "x64"
    startproject "%{AppName}"
    toolset "clang"

    filter "configurations:Debug"
        defines "MFL_DEBUG"
        symbols "ON"
    
    filter "configurations:Release"
        defines "MFL_RELEASE"

    filter "configurations:Distribution"
        defines "MFL_DISTRIBUTION"

    filter "configurations: Release, Distribution"
        optimize "On"
    
    filter {"system:windows", "configurations:Release, Distribution"}
        buildoptions {"/MT"}
    
    filter {"system:windows", "configurations:Debug"}
        buildoptions {"/MTd"}
    
    filter "system:windows"
        systemversion "latest"
        defines {"_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS", "MFLY_WINDOWS"}


include "defaults.lua"

include "MarcFly_Learngine_LIB"