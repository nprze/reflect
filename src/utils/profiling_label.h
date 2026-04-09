#pragma once
#ifdef NVTX_ENABLE_SPECIAL
    #include <iostream>
    #include "nvtx/nvtx3.hpp"
    #include "reflect_directories.h"
    #define RFCT_PROFILE_SCOPE(name) \
        nvtx3::scoped_range_in<GET_NVTX_DOMAIN(__FILE__)> scopedRange(name);
    #define RFCT_PROFILE_FUNCTION() \
        nvtx3::scoped_range_in<GET_NVTX_DOMAIN(__FILE__)> scopedRange(__func__);
    #define RFCT_MARK(name) \
        nvtx3::mark_in<GET_NVTX_DOMAIN(__FILE__)>(name);
#else
    #ifdef NVTX_ENABLE
    #include <iostream>
    #include "nvtx/nvtx3.hpp"
    #define RFCT_PROFILE_SCOPE(name) \
        nvtx3::scoped_range scopedRange(name);
    #define RFCT_PROFILE_FUNCTION() \
        nvtx3::scoped_range scopedRange(__func__);
    #define RFCT_MARK(name) \
        nvtx3::mark_in<GET_NVTX_DOMAIN(__FILE__)>(name);
    #else
        #define RFCT_PROFILE_SCOPE(name)
        #define RFCT_PROFILE_FUNCTION()
        #define RFCT_MARK(name)
    #endif // NVTX_ENABLE
#endif // NVTX_ENABLE_SPECIAL