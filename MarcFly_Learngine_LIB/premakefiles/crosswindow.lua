-- CROSSWINDOW
os.executef("cmake ../submodules/crosswindow -DXGFX_API=VULKAN -A %s -G \"%s\" -B %s/crosswindow", cmake_platform, cmake_generator, wsbin)
os.executef("cmake --build %s/crosswindow --config debug", wsbin)
os.executef("cmake --build %s/crosswindow --config release", wsbin)

-- CROSSWINDOW GFX

os.executef("cmake ../submodules/crosswindow_gfx -DXGFX_API=VULKAN -A %s -G \"%s\" -B %s/crosswindow_gfx", cmake_platform, cmake_generator, wsbin)
os.executef("cmake --build %s/crosswindow_gfx --config debug", wsbin)
os.executef("cmake --build %s/crosswindow_gfx --config release", wsbin)

-- Library files

crosswin_dbg = "%{wsbin}/crosswindow/Debug/CrossWindow%{cmake_static_lib_ext}"
crosswin_rls = "%{wsbin}/crosswindow/Release/CrossWindow%{cmake_static_lib_ext}"

crosswin_gfx_dbg = "%{wsbin}/crosswindow/Debug/CrossWindowGFX%{cmake_static_lib_ext}"
crosswin_gfx_rls = "%{wsbin}/crosswindow/Release/CrossWindowGFX%{cmake_static_lib_ext}"


-- Crosswindow Defines forward from CMAKE
project "*"
    defines "XGFX_VULKAN"
    filter {"system:windows"}
        defines "XWIN_WIN32"
    
    links { "$(VULKAN_SDK)/lib/vulkan-1.lib" }
    includedirs { "$(VULKAN_SDK)/include" }

IncludeDirs["CrossWindow"] = "submodules/crosswindow/src"
IncludeDirs["CrossWindowGFX"] = "submodules/crosswindow_gfx/src"