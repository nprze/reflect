#pragma once
#ifdef NVTX_ENABLE
#include "nvtx/nvtx3.hpp"
#include <iostream>
#include "reflect_directories.h"
#define RFCT_PROFILE_SCOPE(name) \
    nvtx3::scoped_range_in<GET_NVTX_DOMAIN(__FILE__)> scopedRange(__func__);
#define RFCT_PROFILE_FUNCTION() \
    nvtx3::scoped_range_in<GET_NVTX_DOMAIN(__FILE__)> scopedRange(__func__);
#define RFCT_EVENT(name) nvtx3::event { name };
#else
#define RFCT_PROFILE_SCOPE(name)
#define RFCT_PROFILE_FUNCTION()
#endif // NVTX_ENABLE