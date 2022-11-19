project "flecs"
    location "../submodules/flecs"
    kind "StaticLib"
    language "C"
    staticruntime "on"

    targetdir (wsbin .. "/%{prj.name}")
    objdir (wsbinint .. "/%{prj.name}")

    files{"../submodules/flecs/flecs.h", "../submodules/flecs/flecs.c"}