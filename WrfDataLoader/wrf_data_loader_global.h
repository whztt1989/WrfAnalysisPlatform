#ifndef WRF_DATA_LOADER_GLOBAL_H_
#define WRF_DATA_LOADER_GLOBAL_H_

#ifdef WRF_DATA_LOADER_BUILD
    #define WRF_DATA_EXPORT __declspec(dllexport)
#else
    #define WRF_DATA_EXPORT __declspec(dllimport)
#endif

#endif