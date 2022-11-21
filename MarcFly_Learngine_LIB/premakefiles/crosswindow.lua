

os.executef("cmake ../submodules/crosswindow -A %s -G \"%s\" -B %s/crosswindow", cmake_platform, cmake_generator, wsbin)
os.executef("cmake --build %s/crosswindow --config debug", wsbin)
os.executef("cmake --build %s/crosswindow --config release", wsbin)

project "*"
    filter {"system:windows"}
        defines "XWIN_WIN32"

-- Copy results

crosswin_dbg = "%{wsbin}/crosswindow/Debug/CrossWindow%{cmake_static_lib_ext}"
crosswin_rls = "%{wsbin}/crosswindow/Release/CrossWindow%{cmake_static_lib_ext}"

-- Define the dirs for the project...
