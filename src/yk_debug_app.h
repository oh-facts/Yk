#ifndef YK_APP_H
#define YK_APP_H

#include <yk.h>
#include <yk_api.h>
#include <renderer/ykr_model_loader.h>
typedef void (*debug_app_start_fn)(struct YkDebugAppState* state);
typedef void (*debug_app_update_fn)(struct YkDebugAppState* state, f64 dt);
typedef int (*debug_app_is_running_fn)(struct YkDebugAppState* state);

/*
    I sometimes use this as a utility function to update stuff once
    right after reloading dll.
*/
typedef void (*debug_app_update_once_fn)(struct YkDebugAppState* state);

typedef void (*debug_app_shutdown_fn)(struct YkDebugAppState* state);


/*
    Stores all app state and relevant function pointers.
    Exists for dll swapping, which is mainly a "debug"
    thing. Shipped builds will be static libraries.
*/

/*
    Sandbox entries should be able to define their own functions.
    So each sandbox entry could be treated as a different app.
    I am not saying main.cpp entries. That would involve too much 
    irrelevant work to set up engine work. Every sandbox entry would have
    a start, update and shutdown.
*/

struct YkDebugAppState
{
    //ToDo(facts): Organize better later. Should renderer be part of "game" code? I do it rn so I can debug since the renderer isn't complete.
    YkWindow window;
    YkRenderer ren;

    YkMemory engine_memory;
    HMODULE game_dll;

    /*
    Only update matters here. The rest are here because I was trying random stuff.
    */

    debug_app_update_fn update;
    debug_app_start_fn start;
    debug_app_is_running_fn is_running;
    debug_app_update_once_fn update_once;
    debug_app_shutdown_fn shutdown;

    
};

inline void reload_dll(struct YkDebugAppState* state)
{
    //sometimes you'll get an error that dll not found
    //its because you need to pass YK_BUILD_SHARED = 1
    //to cmake to build as a dll. so, 
    // -DYK_BUILD_SHARED=1
    Assert(yk_clone_file("yk.dll", "temp.dll"), "yk.dll couldn't be cloned");

    state->game_dll = LoadLibraryA("temp.dll");
    if (state->game_dll == NULL) {
        printf("Failed to load the DLL\n");
        exit(1);
    }

    state->start = (debug_app_start_fn)GetProcAddress(state->game_dll, "_debug_app_start");
    state->update = (debug_app_update_fn)GetProcAddress(state->game_dll, "_debug_app_update");
   
    state->is_running = (debug_app_is_running_fn)GetProcAddress(state->game_dll, "_debug_app_is_running");
    state->update_once = (debug_app_update_once_fn)GetProcAddress(state->game_dll, "_debug_app_update_once");
    state->shutdown = (debug_app_shutdown_fn)GetProcAddress(state->game_dll, "_debug_app_shutdown");

    if (!state->update || !state->start || !state->is_running || !state->shutdown || !state->update_once) {
        printf("Failed to find one of the dll functions. You probably forgot to build this as a dll. Did\
            you pass -DYK_BUILD_SHARED=1 to cmake? We don't build statically yet. Only shared so I can hot\
            reload code (developer feature)\n");
        FreeLibrary(state->game_dll);
        exit(1);
    }

    printf("dll reload successful\n");
}

#endif // !YK_APP_H