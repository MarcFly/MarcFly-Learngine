---
title: "Hot Reloading and Plugin Systems"
description: Making a flexible C++ program
---

# Unorganized Notes

A plugin system basically provides known or volatile functions or symbols that the main program will make use of, which come from a previously known base system like derivations of an already known class. Internal state will be handled by the plugin and any other information will be deciphered to other internal data structures.

In that case, the plugin writer has a specific set of rules to take, use and give back data so that the main program knows how to deal with it at all times without knowledge of the plugin internal structure. For example: 
If you would write an animation curve system to ease state of animations between keyframes, the base program provides a list of keyframes and their data points. The plugin then will return back a usable value that the program also provided as a return.

That is a way of extending your program, the next part is Hot Reloading.
This requires a known internal state, thus the plugin will allocate, send to the main program and then get back when the plugin is reloaded. Simple enough.

At first the main problem lies in the generation of symbols and usability for not internally known things. I would assume that would be better handled similarly to the OpenGL loaders such as GLAD. If i recall, they provide a list of empty symbols, or function pointers as well as data pointers in a header.
The program will have those "declarations" and then at runtime load them so when the use calls the function pointers in execution they will be relayed to the OpenGL Dynamic Library symbols.

All in all feels like 3 ordered problems to solve:
 - Load Symbols for a module which has no internal implementation of
 - Plugin Base Implementation Template
 - Plugin Internal State - Save Load for Hot Reload (Not prioritary)

# Loading Unknown Symbols

I knew that something like `dlopen` existed and that could be used to load libraries at runtime. It comes with another set of tools for dealing with such occurrence but it works on Linux. I can't find any abstraction that also works with Windows own `LoadLibrary` or whatever crap there is, so let's build a simple one.

```c++
//Windows
#include <windows.h>
#include <stdio.h>

typedef int (__cdecl *FunName)(ARG_TYPE);
// * feels bad for functors, just use it later?

void load_a_lib(const char* lib_path)) {
    HINSTANCE lib;
    FunName fun;
    bool free, success = false;

    lib = LoadLibrary(lib_path);
    
    if (lib != NULL){
        fun = (FunName) GetProcAddress(lib, "funName");
        if (fun != NULL)
        {
            success = true;
            (fun)("String Argument sent to function in dll.");
        }

        free = FreeLibrary(lib);
    }
}


// Linux / Unix
#include <dlfcn.h>
#include <iostream>

typedef int (__cdecl *FunName)(ARG_TYPE);
// * feels bad for functors, just use it later?

void load_a_lib(const char* lib_path)) {
    void* lib;
    FunName fun;
    bool free, success = false;

    lib = dlopen(lib_path, RTLD_LAZY); // Similar to Async?
    
    if (lib != NULL){
        fun = (FunName) dlsym(lib, "funName");
        if (fun != NULL)
        {
            success = true;
            (fun)("String Argument sent to function in dll.");
        }

        free = dlclose(lib);
    }
}
```

Simple Abstraction:
```c++
#ifdef _WIN32
    #include <Windows.h> // Granular window headers somewhere?
    typedef HINSTANCE libhandle;
    #define DLLoadLib(relative_path) LoadLibrary(relative_path)
    #define DLFreeLib(lib_handle) FreeLibrary(lib_handle)
    #define DLLoadFun(lib_handle, fun_name) (StateFun)GetProcAddress(lib_handle, "funName")
#else if defined(__linux__)
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

```
## Loading KNOWN Symbols

Here our main program already defines a base class, which then the plugin implements. Thus, we can call things in the class/struct easily.

```c++

```


# Trinkets
https://stackoverflow.com/questions/3511868/whats-the-point-of-this-kind-of-macros
 - `TEXT` is a Windows macro that substitutes for 16 or 8 byte character in string, or using `L` before a string for wide strings.

# Webgraphy
https://github.com/xbanks/dynamic-loading-example
https://stackoverflow.com/questions/11741580/dlopen-loadlibrary-on-same-application
https://github.com/ahota/zoo/blob/master/src/zoo.cpp
https://ourmachinery.com/post/dll-hot-reloading-in-theory-and-practice/
https://developpaper.com/loading-dynamic-libraries-using-dlopen/
https://man7.org/linux/man-pages/man3/dlopen.3.html