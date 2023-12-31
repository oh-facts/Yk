#ifndef YK_APP_H
#define YK_APP_H

#include <yk.h>
#include <yk_api.h>

typedef void (*debug_app_start_fn)(struct YkDebugAppState* state);
typedef void (*debug_app_update_fn)(struct YkDebugAppState* state);
typedef int (*debug_app_is_running_fn)(struct YkDebugAppState* state);
typedef void (*debug_app_update_ref_fn)(struct YkDebugAppState* state);
typedef void (*debug_app_shutdown_fn)(struct YkDebugAppState* state);


/*
    Stores all app state and relevant function pointers.
    Exists for dll swapping, which is mainly a "debug"
    thing. Shipped builds will be static libraries.
*/

struct YkDebugAppState
{
    //ToDo(facts): Organize better later. Should renderer be part of "game" code? I do it rn so I can debug since the renderer isn't complete.
    YkWindow window;
    YkRenderer ren;
    render_object ro[2];

    HMODULE hModule;

    debug_app_update_fn update;
    debug_app_start_fn start;
    debug_app_is_running_fn is_running;
    debug_app_update_ref_fn update_ref;
    debug_app_shutdown_fn shutdown;

};

inline void reload_dll(struct YkDebugAppState* state)
{
    copy_file("yk.dll", "temp.dll");

    state->hModule = LoadLibraryA("temp.dll");
    if (state->hModule == NULL) {
        printf("Failed to load the DLL\n");
        exit(1);
    }

    state->start = (debug_app_start_fn)GetProcAddress(state->hModule, "_debug_app_start");
    state->update = (debug_app_update_fn)GetProcAddress(state->hModule, "_debug_app_update");
   
    state->is_running = (debug_app_is_running_fn)GetProcAddress(state->hModule, "_debug_app_is_running");
    state->update_ref = (debug_app_update_ref_fn)GetProcAddress(state->hModule, "_debug_app_update_references");
    state->shutdown = (debug_app_shutdown_fn)GetProcAddress(state->hModule, "_debug_app_shutdown");

    if (!state->update || !state->start || !state->is_running || !state->shutdown || !state->update_ref) {
        printf("Failed to find the function\n");
        FreeLibrary(state->hModule);
        exit(1);
    }
}

#endif // !YK_APP_H