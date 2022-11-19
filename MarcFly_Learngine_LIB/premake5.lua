--include "premakefiles/imgui.lua"
--include "premakefiles/mathgeolib.lua"
--include "premakefiles/parson.lua"
--include "premakefiles/pcg.lua"
--include "premakefiles/spdlog.lua"

IncludeDirs["SPDLOG"] = "%{LibName}/vendor/spdlog/include"
IncludeDirs["DearIMGui"] = "%{LibName}/vendor/imgui"
IncludeDirs["MathGeoLib"] = "%{LibName}/vendor/mathgeolib/src"
IncludeDirs["Parson"] = "%{LibName}/vendor/parson"
IncludeDirs["PCG"] = "%{LibName}/vendor/pcg"

project "MarcFly_Learngine_LIB"
    location "."
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir (wsbin .. "/%{prj.name}")
    objdir (wsbinint .. "/%{prj.name}")

    files {
        "%{prj.name}/**.h",
        "%{prj.name}/**.cpp",
        "%{prj.name}/**.hpp",
        "%{prj.name}/**.cxx"
    }

    includedirs {
        "./src"
    }

    defines {
        "MFL_BUILD_STATIC"
    }