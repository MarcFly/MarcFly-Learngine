#ifndef _MFLY_DLLOADING_H_
#define _MFLY_DLLOADING_H_

typedef unsigned int32_t uint32_t;

#ifdef _WIN32
    #include <Windows.h> // Granular window headers somewhere?
    typedef HINSTANCE libhandle;
    #define DLLoadLib(relative_path) LoadLibrary(relative_path)
    #define DLFreeLib(lib_handle) FreeLibrary(lib_handle)
    #define DLLoadFun(lib_handle, fun_name) (StateFun)GetProcAddress(lib_handle, "funName")
#else if UNIX_PLATFORMS
    #include <dlfcn.h>
    typedef void* libhandle;
    
    #ifdef DLOpenFlag
        #define DLLoadLib(relative_path) dlopen(relative_path, DLOpenFlag)
    #else
        #define DLLoadLib(relative_path) dlopen(relative_path, RTLD_NOW)
    #endif

    #define DLFreeLib(lib_handle) dlclose(lib_handle)
    #define DLLoadFun(lib_handle, fun_name) dlsym(lib_handle, "funName")
#endif

typedef void (*StateFun)(void*, uint64_t);

struct dllibrary {
    char name[64] = "unknown";
    libhandle handle = NULL;
    bool loaded = false;
    
    void* state = NULL;
    uint64_t state_size;
    StateFun save_state; // Moves state from internal to library_held here
    StateFun load_state; // Leaves state blank until save_state
};

#include <vector>
static std::vector<dllibrary> libraries;

uint32_t DLoadLib(const char* relative_path)
{
    dllibrary lib; 
    lib.handle = DLLoadLib(relative_path);
    if (lib.handle != NULL)
    {
        lib.loaded = true;
        memcpy(&lib.name[0], relative_path, sizeof(lib.name));
        lib.save_state = DLLoadFun(lib.handle, "SaveInternalState");
        lib.load_state = DLLoadFun(lib.handle, "LoadInternalState");
        // Error Check state funs?
    }
    libraries.push_back(lib);
    return libraries.size()-1; // Simple Handle system
}

bool DFreeLib(uint32_t lib_pos){
    dllibrary& lib = libraries[lib_pos];

    if (lib.loaded = !DLFreeLib(lib.handle)) {
        lib.handle = NULL;
    }
    // Do not erase lib name, as it may be loaded in the future
    // Don't clean state, it is managed only by reload

    return lib.loaded;
}

bool DReloadLib(uint32_t lib_pos){
    dllibrary& lib = libraries[lib_pos];
    
    lib.save_state(lib.state, lib.state_size);
    
    DFreeLib(lib_pos);
    
    DLoadLib(lib.name);

    lib.load_state(lib.state, lib.state_size);
}

//=====================================================================================
// Example plugin system with this dlloading project
#ifdef USE_MFLY_PLUGIN

#endif

#endif // _MFLY_DLLOADING_H_