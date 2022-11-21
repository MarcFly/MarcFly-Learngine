outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outreleasedir = "Release-%{cfg.system}-%{cfg.architecture}"
outdebugdir = "Debug-%{cfg.system}-%{cfg.architecture}"
wsbin = path.join(_WORKING_DIR, "./build")
wsbinint = path.join(_WORKING_DIR, "./bin-int")
IncludeDirs = {}

-- CMAKE GENERAL DEFINES
cmake_platform = ""
cmake_generator = ""
cmake_static_lib_ext = ""
cmake_dyn_lib_ext = ""

filter {"system:windows"}
    cmake_platform = "x64"
    cmake_generator = "Visual Studio 16 2019"
    cmake_static_lib_ext = ".lib"
    cmake_dyn_lib_ext = ".dll"