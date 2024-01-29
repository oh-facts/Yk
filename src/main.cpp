#include <yk_debug_app.h>
#include <yk_memory.h>


#if DEBUG
LPVOID base_address = (LPVOID)Terabytes(2);
#else
LPVOID base_address = 0;
#endif

void engine_memory_innit(YkMemory* engine_memory)
{
    size_t perm_storage_size = Megabytes(64);
    size_t temp_storage_size = Gigabytes(1);

    u64 total_size = perm_storage_size + temp_storage_size;

    yk_memory_arena_innit(&engine_memory->perm_storage, perm_storage_size, VirtualAlloc(base_address, total_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    yk_memory_arena_innit(&engine_memory->temp_storage, temp_storage_size, (u8*)engine_memory->perm_storage.base + perm_storage_size);

    engine_memory->is_initialized = 1;
}

void engine_memory_cleanup(YkMemory* engine_memory)
{
    yk_memory_arena_reset(&engine_memory->perm_storage);
    yk_memory_arena_reset(&engine_memory->temp_storage);
    engine_memory->is_initialized = 0;
    VirtualFree(base_address, 0, MEM_RELEASE);
}

int main(int argc, char *argv[])
{

    struct YkDebugAppState state = { };
    reload_dll(&state);

    engine_memory_innit(&state.engine_memory);

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

    f64 dt = 0;
    
    constexpr u32 print_stats_time = 1;

    while (state.is_running(&state))
    {
        f64 last_time_elapsed = total_time_elapsed;


        
        //game loop start--------
        if (!state.window.win_data.is_minimized)
        {
            state.update(&state, dt);

            if (yk_input_is_key_tapped(&state.window.keys,YK_KEY_ESC)) {
                state.shutdown(&state);
                engine_memory_cleanup(&state.engine_memory);
                
                FreeLibrary(state.hModule);
                reload_dll(&state);

                engine_memory_innit(&state.engine_memory);
                state.start(&state);
            }
        }

      //testing input
#if 0    
        if (yk_input_is_key_tapped(&state.window.keys, 'A'))
        {
            printf("A tapped");
        }


        if (yk_input_is_key_held(&state.window.keys, 'A'))
        {
            printf("A held");
        }

        if (yk_input_is_key_released(&state.window.keys, 'A'))
        {
            printf("A released");
        }

        if (yk_input_is_key_released(&state.window.keys, 'V'))
        {
            printf("V released");
        }

        if (yk_input_is_click(&state.window.clicks, YK_MOUSE_BUTTON_RIGHT))
        {
            printf("right Clicked");
        }
#endif
        //-------game loop end

       
        LARGE_INTEGER end_counter = {};
        QueryPerformanceCounter(&end_counter);

        i64 counter_elapsed = end_counter.QuadPart - start_counter.QuadPart;
        total_time_elapsed = (1.f * counter_elapsed) / counter_freq;
        
        dt = total_time_elapsed - last_time_elapsed;

        //perf stats
#if 1
        time_since_print += dt;

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
#endif    
        yk_window_update(&state.window);
        yk_window_poll();

    }

    state.shutdown(&state);
    
    yk_free_window(&state.window);
    engine_memory_cleanup(&state.engine_memory);

    FreeLibrary(state.hModule);
    remove("temp.dll");
    return 0;
}