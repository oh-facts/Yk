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
// Next day:
// Note(facts): I found a C api for jolt. It is up to date with 4.02 atm. Which is almost latest version. It gets updated with new releases.
//              I can stick to C forever.
//              Wait I misread. 
//              Hi,
//              Joltc is updated with latest 4.0.2 stable release of jolt physics, thus lot of bindings are missing into joltc, I add stuff when needed.
//
//              Feel free to contribute.
// 
//              Updated with latest 4.0.2 stable release? But lots bindings are missing? So is it updated or not?
//              Fucks sake. FUCK FUCK FUCK. I can't make my own physics engine c'mon be serious. AHHHH
//              I will port to C++. More specificlly, make my code "C++ compliant". AHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH. 
//              I will use no templates. No namespaces. No operator overloading, not C++ stl. My only instances of C++ code will be when I am 
//              using the physics engine to make porting back and forth easy, because once I have a team of devs. They will be porting this back to C
//              and working on physics.
//
// 
// Note(facts): I have completed porting to C++. Takes longer to compile, but if I don't port now, it will be harder to port later. Atleast now I can use
//              jolt. It is so fucking weird that jolt themselves didn't make a C api so other langugages could use. So So weird.
//
//
//
//              Work on submitting render data to the renderer


/*
    Absolutely no idea why I am having some crashes. Made new branches to investigate later.
    Considering sdl because windows.h is a fuckfest
*/

/*
    Anyways work on making the renderer not require win layer. I don't want dogshit windows.h to meddle with the rest of my project. FUCK MICROSOFT (John Malkovwitch voice)
    Done
*/

/*
    time works now. I can only imagine I was having some buffer overflow, since I am calculating time very differently now and it works. Made branches just in case someone 
    wants to investigate.
*/

/*
    Work on the renderer now
*/

//ToDo(facts): compile vma as a dll so I don't have to compile it again
//             false alarm. It doesn't take time anymore. idky gabe hates me so much lmao
//

//  I haven't gotten much work done. I understand vulkan a little better though. Also, I started following vkguide and its been helpful. My code wasn't too far off from what he did
//  (Except sync2, descr indexing and buffer address. Dynamic rendering was practicially identical though
//  
//  The dll hotreloading has easily been the best decision so far. It works wonderfully rn. Idk how I managed other engines w/o it. Tbf I didn't
// 
// ToDo(facts 1703 1/5/24): Work through vkguide
//
// ToDo(facts): Make a py script to compile shaders. Something like py ./shaderc "foo/main.frag". This will compile and put main.frag.spv inside res/shaders/foo/
//  
// ToDo(facts): Make the debug application entry handler thing.
//
// ToDo(facts 0445 1/11/24): organize renderer functions in different files. It is a pain to scroll around 2000 locs where most functions are complete. 
//                           Also, remove all obsolete code
// ToDo(facts):              Set up imgui
// 
//
// ToDo(facts): Make a 3d debug camera
// ToDo(facts): Abstract making pipelines.
//  


#include <yk_debug_app.h>
#include <yk_memory.h>


#if DEBUG
LPVOID base_address = (LPVOID)Terabytes(2);
#else
LPVOID base_address = 0;
#endif



int main(int argc, char *argv[])
{

    //ToDo(facts): Actually make use of this lmao
    

    struct YkDebugAppState state = { };
    reload_dll(&state);

    state.engine_memory.perm_storage_size = Megabytes(64);
    state.engine_memory.temp_storage_size = Megabytes(64);

    u64 total_size = state.engine_memory.perm_storage_size + state.engine_memory.temp_storage_size;

    state.engine_memory.perm_storage = VirtualAlloc(base_address, total_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    state.engine_memory.temp_storage = (u8*)state.engine_memory.perm_storage + state.engine_memory.perm_storage_size;

    state.ren.clock = clock();

    yk_innit_window(&state.window);
    state.start(&state);

 
    //ToDo (facts 8:07 1/2/24): Calculate average
    LARGE_INTEGER start_counter = {};
    QueryPerformanceCounter(&start_counter);

    LARGE_INTEGER perf_freq = {};
    QueryPerformanceFrequency(&perf_freq);
    i64 counter_freq = perf_freq.QuadPart;



    f64 total_time_elapsed = 0;

    f32 time_since_print = 0;
    
    constexpr u32 print_stats_time = 1;

    while (state.is_running(&state))
    {
        f64 last_time_elapsed = total_time_elapsed;

        yk_window_poll();
        
        //game loop--------
        if (!state.window.win_data.is_minimized)
        {
            state.update(&state);

            if (state.window.test == 1) {
                state.window.test = 0;
                state.shutdown(&state);

                FreeLibrary(state.hModule);
                reload_dll(&state);

                state.start(&state);
            }
        }
        //-------game loop

        LARGE_INTEGER end_counter = {};
        QueryPerformanceCounter(&end_counter);

        i64 counter_elapsed = end_counter.QuadPart - start_counter.QuadPart;
        total_time_elapsed = (1.f * counter_elapsed) / counter_freq;
        
        f64 difference = total_time_elapsed - last_time_elapsed;
        time_since_print += difference;

        if (time_since_print > print_stats_time)
        {
            f64 frame_time = total_time_elapsed - last_time_elapsed;

            printf("\n     perf stats     \n");
            printf("\n--------------------\n");
            printf("frame time : %.3f ms\n", frame_time * 1000.f);
            printf("frame rate : %.0f \n", 1/frame_time);
            printf("---------------------\n");

            time_since_print = 0;
        }
     
    }
    state.shutdown(&state);
    yk_free_window(&state.window);

    FreeLibrary(state.hModule);
    remove("temp.dll");
    return 0;
}