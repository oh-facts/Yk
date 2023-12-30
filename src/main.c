
#include <app.h>
#include <win32_window.h>
//ToDo(facts):  Better Debug profiles.
// 12/23 1758
// ToDo(facts): Fix flickering triangle (sync problem I think)
//              Fixed.
//                  1. I was handling cmds incorrectly in win32
//                  2. My rendering attatchment info was outside of loop
// ToDo(facts): Destroy resources
//              Fixed.
// ToDo(facts): Posix window so my linux friends can see my triangle
// ToDo(facts): Window Abstraction
//              Fixed.
// ToDo(facts): Renderer Abstraction :skull:
//              Fixed.
// ToDo(facts): VkAssertResult profiles?
// 
// 12/24 0825
// ToDo(facts): Fix all those warnings
//              Fixed.
// ToDo(facts): Window callbacks instead of globals
//              Fixed.
// 12/24 1010
// ToDo(facts): Clean up triangle renderer
//
// 12/24 1834: Port to C++? I want to do this because I want to be able to use a physics engine in the future. 
//             This is the only reason. constexpr and templates are just nice to have.
//             I have decided against it. I will just whip my own physics engine out. Thats what source 2 did.
//
//       1854: Work on dll reloading.
//
//             Fix swapchain recreation bugs. Handle all window events. fix validation errors
//             Done
// 
// 12/24 2122: One translational unit. Maybe better to have multiple "single translational units".
//             
//             Math
//             Model loading
//
// This is me from the future. I am a git hopper. I travel between parallel git commits.
// The abstraction you made was dogshit. So I am back. I spent 4 days fucking about. I have aged in real life.
// 
// 12/30 0027: Work on hot reloading. It is pain to rapidly iterate otherwise.
// 12/39 1340: Completed. Platform layer needs clean up. But its works exactly as expected with no errors even after pervasive testing.
//             My biggest takeaway is - "FUCK YOU MICROSOFT! (John Malkovitch voice)"
//
//ToDo(facts): Clean up platform layer. Remember. It should be easy to be able to statically build without all that hot reloading. Hot
//             reloading is purely a debug thing. Final build needs to be as fast as possible so all indirections / debug features must
//             be removed.
//Note(facts): I have been listening to Erika. Fuck it, I'm rolling my own physics engine. I am great at physics and programming so it
//             wont be too hard to roll a very efficient one in C. I dislike C++ too much to port my code. Also, my physics needs wont
//             be that much if I go the voxel route. Obviously, I will still use 3rd party libraries like cimgui and STB header family.
//             I am not a pyscopath, I just hate C++ so so much. Once constexpr comes to C, C will be feature complete. I rolled out 
//             this dll hot code reload all by myself and figured out vulkan from the spec. I can easily read relevant physics engine
//             books and repositories (like jolt) to make my own. Remember, the only reason why I am rolling my own is because one 
//             doesnt already exist in C. If Jolt had a C api, I wouldn't do this.
//


struct YkMemory
{
    int is_initialized;
    u64 perm_storage_size;
    void *perm_storage;
    u64 temp_storage_size;
    void *temp_storage;
};

typedef struct YkMemory YkMemory;

#if DEBUG
    LPVOID base_address = (LPVOID)Terabytes(2);
#else
    LPVOID base_address = 0;
#endif

HMODULE hModule;
typedef void (*UpdateFunc)(struct state* state);
typedef void (*StartFunc)(struct state* state);
typedef int (*IsRunningFunc)(struct state* state);
typedef void (*UpdateRefFunc)(struct state* state);
typedef void (*FreeApp)(struct state* state);
UpdateFunc Update;
StartFunc Start;
IsRunningFunc IsRunning;
UpdateRefFunc update_ref;
FreeApp free_app;

void reload_dll()
{
    copy_file("yk.dll", "temp.dll");

    hModule = LoadLibraryA("temp.dll");
    if (hModule == NULL) {
        printf("Failed to load the DLL\n");
        exit(1);
    }

    Update = (UpdateFunc)GetProcAddress(hModule, "update");
    Start = (StartFunc)GetProcAddress(hModule, "start");
    IsRunning = (IsRunningFunc)GetProcAddress(hModule, "is_running");
    update_ref = (UpdateRefFunc)GetProcAddress(hModule, "update_references");
    free_app = (FreeApp)GetProcAddress(hModule, "freeApp");

    if (!Update || !Start || !IsRunning || !free_app || !update_ref) {
        printf("Failed to find the function\n");
        FreeLibrary(hModule);
        exit(1);
    }
}

int main(int argc, char *argv[])
{

    //ToDo(facts): Actually make use of this lmao
    YkMemory engine_memory = {0};
    engine_memory.perm_storage_size = Megabytes(64);
    engine_memory.temp_storage_size = Megabytes(64);

    u64 total_size = engine_memory.perm_storage_size + engine_memory.temp_storage_size;

    engine_memory.perm_storage = VirtualAlloc(base_address, total_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    engine_memory.temp_storage = (u8*)engine_memory.perm_storage + engine_memory.perm_storage_size;
    reload_dll();

    time_t start, now;
    double elapsed;
    time(&start);
    struct state state = { 0 };
    struct state state2 = { 0 };
    state.ren.clock = clock();
 
    yk_innit_window(&state.window);
    Start(&state);
    

    while (IsRunning(&state))
    {
        yk_window_poll();

        if (!state.window.win_data.is_minimized)
        {
            Update(&state);

            //ToDo(facts): Do this every time a key is pressed
            time(&now);
            elapsed = difftime(now, start);
            if (state.window.test == 1) {
                state.window.test = 0;
                free_app(&state);

                FreeLibrary(hModule);
                reload_dll();

                Start(&state);
                //update_ref(&state);

                time(&start);
            }
        }

    }

    yk_free_window(&state.window);

    FreeLibrary(hModule);
    remove("temp.dll");
    return 0;
}