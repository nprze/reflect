# DO NOT REMOVE ANY SETTINGS FROM THIS FILE EVEN IF UNUSED. IF YOU DO NOT WANT TO USE A FEATURE, SET IT TO "n".

set(BUILD_FOR_PLATFORM "ANDROID") # ANDROID or WINDOWS

set(VLD_ENABLE "n") # y or n, enable or disable visual leak detector
set(VLD_PATH "C:/Program Files (x86)/Visual Leak Detector") # path to where visual leak detector is installed. Leave empty if VLD is disabled.

set(NVTX_ENABLE "n") # y or n, enable or disable nvtx in order to help profiling with Nvidia Nsight Systems
set(NVTX_FOLDER_REGEXP "^.*_p$") # regexp for the folders which are supposed to be labeled by nvtx. Basically if you call RFCT_PROFILE_ macros, reflect will find the nearest folder which satisfies that regexp and select the nvtx domain of the same name as the folder. Leave empty if NVTX is disabled.

set(VULKAN_DEBUG_UTILS_ENABLE "n") # y or n, enable or disable vulkan validation layers


set(VULKAN_SDK $ENV{VULKAN_SDK})

if(ANDROID)
    set(VULKAN_INCLUDE_DIRS ${VULKAN_SDK}/include)
    set(VULKAN_LIBRARY_DIRS ${VULKAN_SDK}/lib)
    set(Vulkan_INCLUDE_DIR ${VULKAN_SDK}/include)
    set(Vulkan_LIBRARY ${VULKAN_SDK}/lib/vulkan.so)  # Android uses shared Vulkan library
endif()
