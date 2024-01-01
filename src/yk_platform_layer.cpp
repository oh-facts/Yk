#include <yk_platform_layer.h>
/*
struct _YkTime
{
    LARGE_INTEGER freq;
    LARGE_INTEGER start;
};

void yk_time_innit(YkTime* time)
{
    _YkTime* _time = (_YkTime*)time;
    LARGE_INTEGER now = {};
    QueryPerformanceCounter(&now);

    _time->start = now;
    QueryPerformanceFrequency(&_time->freq);
}

f32 yk_get_time(YkTime* time)
{
    _YkTime* _time = (_YkTime*)time;
    LARGE_INTEGER now = {};
    QueryPerformanceCounter(&now);

    return ((f32)now.QuadPart - _time->start.QuadPart) / _time->freq.QuadPart;
}
*/