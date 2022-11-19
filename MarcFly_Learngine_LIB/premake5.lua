--include "premakefiles/imgui.lua"
--include "premakefiles/mathgeolib.lua"
--include "premakefiles/parson.lua"
--include "premakefiles/pcg.lua"
--include "premakefiles/spdlog.lua"
include "premakefiles/flecs.lua"

IncludeDirs["flecs"] = "submodules/flecs"

IncludeDirs["SPDLOG"] = "%{LibName}/vendor/spdlog/include"
IncludeDirs["DearIMGui"] = "%{LibName}/vendor/imgui"
IncludeDirs["MathGeoLib"] = "%{LibName}/vendor/mathgeolib/src"
IncludeDirs["Parson"] = "%{LibName}/vendor/parson"
IncludeDirs["PCG"] = "%{LibName}/vendor/pcg"

project "MarcFly_Learngine_LIB"
    location "."
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir (wsbin .. "/%{prj.name}")
    objdir (wsbinint .. "/%{prj.name}")

    --flecs files

    files {
        "src/**.h",
        "src/**.cpp",
        "src/**.hpp",
        "src/**.cxx"
    }

    includedirs {
        "./src",
        "%{IncludeDirs.flecs}"
    }

    links {"flecs"}

    defines {
        "MFL_BUILD_STATIC"
    }