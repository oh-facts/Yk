#ifndef YK_API_H
#define YK_API_H

#ifndef YK_BUILD_SHARED
    #define YK_BUILD_SHARED 1
#endif

#if defined _WIN32
    #if YK_BUILD_SHARED
        #define YK_API  extern "C" __declspec(dllexport)
    #else
        #define YK_API
    #endif
#else
    #define YK_API
#endif

#endif