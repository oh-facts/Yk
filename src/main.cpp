#include <yk_debug_app.h>
#include <yk_memory.h>

#define mem_leak_msvc 0

#if mem_leak_msvc
    #define _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <crtdbg.h>
    #include <iostream>
#endif

int main(int argc, char *argv[])
{
    struct YkDebugAppState state = { };
    reload_dll(&state);

   // engine_memory_innit(&state.engine_memory);

    state.ren.clock = clock();

    yk_innit_window(&state.window);
    state.start(&state);
 
    LARGE_INTEGER start_counter = {};
    QueryPerformanceCounter(&start_counter);

    LARGE_INTEGER perf_freq = {};
    QueryPerformanceFrequency(&perf_freq);
    i64 counter_freq = perf_freq.QuadPart;

    f64 total_time_elapsed = 0;
    f64 dt = 0;
    
    while (state.is_running(&state))
    {
        f64 last_time_elapsed = total_time_elapsed;

        //game loop start--------
        if (!state.window.win_data.is_minimized)
        {
            state.update(&state, dt);

            if (yk_input_is_key_tapped(&state.window.keys,YK_KEY_ESC)) {
               // state.shutdown(&state);
                //engine_memory_cleanup(&state.engine_memory);
                
                FreeLibrary(state.game_dll);
                reload_dll(&state);
                state.update_once(&state);

                //engine_memory_innit(&state.engine_memory);
               // state.start(&state);
            }
        }

        yk_window_update(&state.window);
        yk_window_poll();

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

        #define num_frames_for_avg 60
        #define print_stats_time 5

        yk_local_persist f64 total_frame_rate;
        yk_local_persist f64 total_frame_time;
        yk_local_persist u32 frame_count;
        yk_local_persist f32 time_since_print;

        total_frame_rate += 1 / dt;
        total_frame_time += dt;
        frame_count++;
        time_since_print += dt;

        if (time_since_print > print_stats_time)
        {
            f64 avg_frame_rate = total_frame_rate / frame_count;
            f64 avg_frame_time = total_frame_time / frame_count;

            printf("\n     perf stats     \n");
            printf("\n--------------------\n");

            //print instantaneous frame rate/time
#if 0
            printf("frame time : %.3f ms\n", dt * 1000.f);
            printf("frame rate : %.0f \n", 1/ dt);
#endif
            printf("average frame time : %.3f ms\n", avg_frame_time * 1000.f);
            printf("average frame rate : %.0f \n", avg_frame_rate);

            printf("---------------------\n");

            total_frame_rate = 0;
            total_frame_time = 0;
            frame_count = 0;

            time_since_print = 0;
        }
#endif    

    }

    state.shutdown(&state);
    
    yk_free_window(&state.window);
  //  engine_memory_cleanup(&state.engine_memory);

    FreeLibrary(state.game_dll);
    remove("temp.dll");

#if mem_leak_msvc
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtDumpMemoryLeaks();
#endif
    return 0;
}