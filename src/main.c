
#include <yk.h>
#include <app.h>


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
    void* perm_storage;
    u64 temp_storage_size;
    void* temp_storage;
};

typedef struct YkMemory YkMemory;

#if DEBUG
LPVOID base_address = (LPVOID)Terabytes(2);
#else
LPVOID base_address = 0;
#endif

HMODULE coreModule = NULL;
FILETIME lastWriteTime;

Update update_func = NULL;
void loadDLL() {
    if (coreModule) {
        FreeLibrary(coreModule);
        coreModule = NULL;
    }

    HANDLE hFile = CreateFile("yk.dll", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Error opening DLL file\n");
        return;
    }

    HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hMapping == NULL) {
        printf("Error creating file mapping\n");
        CloseHandle(hFile);
        return;
    }

    coreModule = (HMODULE)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (coreModule == NULL) {
        printf("Error mapping view of file\n");
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }

    WIN32_FIND_DATAA findFileData;
    HANDLE fileHandle = FindFirstFile("yk.dll", &findFileData);
    if (fileHandle != INVALID_HANDLE_VALUE) {
        GetFileTime(fileHandle, NULL, NULL, &lastWriteTime);
        FindClose(fileHandle);
    }

    update_func = (Update)GetProcAddress(coreModule, "update");
    if (!update_func) {
        printf("Error loading update function from DLL\n");
        UnmapViewOfFile(coreModule);
        coreModule = NULL;
    }

    CloseHandle(hMapping);
    CloseHandle(hFile);
}

int main(int argc, char* argv[]) {
    YkMemory engine_memory = { 0 };
    engine_memory.perm_storage_size = Megabytes(64);
    engine_memory.temp_storage_size = Megabytes(64);

    u64 total_size = engine_memory.perm_storage_size + engine_memory.temp_storage_size;

    engine_memory.perm_storage = VirtualAlloc(base_address, total_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    engine_memory.temp_storage = (u8*)engine_memory.perm_storage + engine_memory.perm_storage_size;

    YkWindow win = { 0 };
    yk_innit_window(&win);

    const vertex vertices[] = {
        {{-0.5f, -0.5f},{163 / 255.f, 163 / 255.f, 163 / 255.f} },
        {{0.5f, -0.5f}, {0.0f, 0.0f, 0.0f} },
        {{0.5f, 0.5f}, {1.f, 1.f, 1.f}},
        {{-0.5f, 0.5f}, {128 / 255.f, 0.f, 128 / 255.f}}
    };

    const vertex vertices2[] = {
       {{-0.5f, -0.5f}, {1.0f, 33 / 255.0f, 140 / 255.0f}},
       {{0.5f, -0.5f}, {1.f, 216 / 255.f, 0}},
       {{0.5f, 0.5f}, {33 / 255.f, 177 / 255.f, 1.f}},
       {{-0.f, 0.5f}, {1.0f, 33 / 255.0f, 140 / 255.0f}}
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

    loadDLL();

    while (win.is_running) {
        if (update_func) {
            update_func(&ren, &win, ros);
        }

        WIN32_FIND_DATAA findFileData;
        HANDLE fileHandle = FindFirstFile("yk.dll", &findFileData);
        const DWORDLONG FILETIME_THRESHOLD = 10000000;

        if (CompareFileTime(&findFileData.ftLastWriteTime, &lastWriteTime) > 0 &&
            CompareFileTime(&findFileData.ftLastWriteTime, &lastWriteTime) > FILETIME_THRESHOLD) {
            printf("Reloading DLL...\n");
            loadDLL();
        }

        // Your existing code...
    }

    if (coreModule) {
        FreeLibrary(coreModule);
    }

    yk_renderer_wait(&ren);
    yk_destroy_model(&ren, &ro);
    yk_destroy_model(&ren, &ro2);
    yk_free_renderer(&ren);
    yk_free_window(&win);

    return 0;
}