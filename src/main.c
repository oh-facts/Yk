
#include <app.h>
#include <win32_window.h>
//ToDo(facts): Better Debug profiles.
// 12/23 1758
// ToDo(facts): Fix flickering triangle (sync problem I think)
//             Fixed.
//                  1. I was handling cmds incorrectly in win32
//                  2. My rendering attatchment info was outside of loop
// ToDo(facts): Destroy resources
//             Fixed.
// ToDo(facts): Posix window so my linux friends can see my triangle
// ToDo(facts): Window Abstraction
//             Fixed.
// ToDo(facts): Renderer Abstraction :skull:
//             Fixed.
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
#define WIN_SIZE_X 800
#define WIN_SIZE_Y 600

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch (msg)
    {
    case WM_CREATE:
    {
        YkWindow* win = (YkWindow*)((CREATESTRUCT*)lParam)->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)win);
    }

    break;
    case WM_SIZE:
    {
        YkWindow* win = (YkWindow*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        win->win_data.size_x = LOWORD(lParam);
        win->win_data.size_y = HIWORD(lParam);
        win->win_data.is_resized = true;
        if (wParam == SIZE_MINIMIZED)
        {
            if (win)
                win->win_data.is_minimized = true;
        }
        else if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED)
        {
            if (win)
                win->win_data.is_minimized = false;
        }
    } break;
    case WM_DESTROY:
    {
        YkWindow* win = (YkWindow*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        if (win)
            win->win_data.is_running = false;
        PostQuitMessage(0);
    }

    break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}


void yk_innit_window(YkWindow* window)
{
    window->win_data.is_running = true;
    window->win_data.is_minimized = false;
    window->hinstance = GetModuleHandle(0);
    window->win_data.size_x = WIN_SIZE_X;
    window->win_data.size_y = WIN_SIZE_Y;

    WNDCLASS wc = { 0 };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = window->hinstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "MainWindowClass";

    if (!RegisterClass(&wc))
        exit(-1);

    window->win_handle = CreateWindowA(wc.lpszClassName, "yekate", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, WIN_SIZE_X, WIN_SIZE_Y,
        NULL, NULL, wc.hInstance, window);

    if (!window->win_handle)
    {
        printf("Win32 window Creation failure");
        exit(-1);
    }

    ShowWindow(window->win_handle, SW_SHOWNORMAL);
    UpdateWindow(window->win_handle);
}

void yk_window_poll()
{
    MSG message;
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessageA(&message);
}
}

#if DEBUG
    LPVOID base_address = (LPVOID)Terabytes(2);
#else
    LPVOID base_address = 0;
#endif

int copy_file(const char* sourcePath, const char* destinationPath) {
    FILE* sourceFile, * destinationFile;
    char buffer[4096];
    size_t bytesRead;

    // Open the source file for reading
    fopen_s(&sourceFile, sourcePath, "rb");
    if (sourceFile == NULL) {
        perror("Error opening source file");
        return 1; // Return an error code
    }

    // Open the destination file for writing
    fopen_s( &destinationFile, destinationPath, "wb");
    if (destinationFile == NULL) {
        perror("Error opening destination file");
        fclose(sourceFile);
        return 1; // Return an error code
    }

    // Copy data from the source file to the destination file
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), sourceFile)) > 0) {
        fwrite(buffer, 1, bytesRead, destinationFile);
    }

    // Close the files
    fclose(sourceFile);
    fclose(destinationFile);

    return 0; // Return 0 for success
}

HMODULE hModule;
typedef void (*UpdateFunc)(struct state* state);
typedef void (*StartFunc)(struct state* state);
typedef int (*IsRunningFunc)(struct state* state);
typedef void (*UpdateRefFunc)(struct state* state);
UpdateFunc Update;
StartFunc Start;
IsRunningFunc IsRunning;
UpdateRefFunc update_ref;

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

    if (!Update || !Start || !IsRunning) {
        printf("Failed to find the function\n");
        FreeLibrary(hModule);
        exit(1);
    }
}

int main(int argc, char *argv[])
{


    YkMemory engine_memory = {0};
    engine_memory.perm_storage_size = Megabytes(64);
    engine_memory.temp_storage_size = Megabytes(64);

    u64 total_size = engine_memory.perm_storage_size + engine_memory.temp_storage_size;

    engine_memory.perm_storage = VirtualAlloc(base_address, total_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    engine_memory.temp_storage = (u8*)engine_memory.perm_storage + engine_memory.perm_storage_size;
    /*
    YkWindow win = { 0 };
    yk_innit_window(&win);

    const vertex vertices[] = {
        {{-0.5f, -0.5f},{163 / 255.f, 163 / 255.f, 163 / 255.f} },   
        {{0.5f, -0.5f}, {0.0f, 0.0f, 0.0f} },    
        {{0.5f, 0.5f}, {1.f, 1.f, 1.f}},     
        {{-0.5f, 0.5f}, {128 / 255.f, 0.f, 128 / 255.f}}      
    };

    const vertex vertices2[] = {
       {{-0.5f, -0.5f}, {1.0f, 33/255.0f, 140/255.0f}},    
       {{0.5f, -0.5f}, {1.f, 216 / 255.f, 0}},    
       {{0.5f, 0.5f}, {33/255.f, 177/255.f, 1.f}},      
       {{-0.5f, 0.5f}, {1.0f, 33 / 255.0f, 140 / 255.0f}}      
    };

    const u16 indices[] = {
        0, 1, 2, 2, 3, 0
    };

    YkRenderer ren = { 0 };
    yk_renderer_innit(&ren, &win);
    
    render_object ro = { 0 };
    ro.id = 0;
    yk_renderer_innit_model(&ren, vertices, indices, &ro);

    
    render_object ro2 = { 0 };
    ro2.id = 1;
    yk_renderer_innit_model(&ren, vertices2, indices, &ro2);
  
    render_object ros[] = { ro,ro2 };
    */

    reload_dll();

    time_t start, now;
    double elapsed;
    time(&start);
    struct state state = { 0 };
    struct state state2 = { 0 };
    state.ren.clock = clock();
 //   memset(state, 0, sizeof(struct state));
    yk_innit_window(&state.window);
    Start(&state);
    
   // struct state* state2 = malloc(sizeof(struct state));

    while (IsRunning(&state))//win.is_running)
    {
        yk_window_poll();
        Update(&state);

        //ToDo(facts): Do this every time a key is pressed
        time(&now);
        elapsed = difftime(now, start);
        if (elapsed >= 3.0) {
           // state2 = state;
            FreeLibrary(hModule);
            reload_dll();
           // state = state2;
            Start(&state);
           // update_ref(state);
            time(&start);
        }

       // yk_window_poll();
      //  yk_renderer_draw_model(&ren, ros, 2);
    }

   // yk_renderer_wait(&ren);

   // yk_destroy_model(&ren, &ro);
  //  yk_destroy_model(&ren, &ro2);
 //   yk_free_renderer(&ren);

 //   yk_free_window(&win);

    FreeLibrary(hModule);
    remove("temp.dll");
    return 0;
}