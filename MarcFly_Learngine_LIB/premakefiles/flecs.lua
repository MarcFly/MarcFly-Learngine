project "flecs"
    location "../submodules/flecs"
    kind "StaticLib"
    language "C"
    staticruntime "on"

    targetdir (wsbin .. "/" .. outputdir .. "/%{prj.name}")
    objdir (wsbinint .. "/" .. outputdir .. "/%{prj.name}")

    files{"../submodules/flecs/flecs.h", "../submodules/flecs/flecs.c"}

IncludeDirs["flecs"] = "submodules/flecs"