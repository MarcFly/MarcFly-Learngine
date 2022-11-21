include "defaults.lua"

--include "premakefiles/imgui.lua"
--include "premakefiles/mathgeolib.lua"
--include "premakefiles/parson.lua"
--include "premakefiles/pcg.lua"
--include "premakefiles/spdlog.lua"
include "premakefiles/flecs.lua"
include "premakefiles/crosswindow.lua"




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

    targetdir (wsbin .. "/" ..  outputdir .. "/%{prj.name}")
    objdir (wsbinint .. "/" ..  outputdir .. "/%{prj.name}")

    --flecs files

    files {
        "src/**.h",
        "src/**.cpp",
        "src/**.hpp",
        "src/**.cxx"
    }
    

    includedirs {
        "./src",
        "%{IncludeDirs.flecs}",
        "%{IncludeDirs.CrossWindow}",
        "%{IncludeDirs.CrossWindowGFX}"
    }

    links {
        "flecs", "%{crosswin_rls}"
    }

    defines {
        "MFL_BUILD_STATIC"
    }