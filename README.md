# reflect is an open source game engine

## before building

- make sure you have cmake installed ( min. v3.16 ).
- make sure you have vulkan SDK installed.
- modify reflect_settings.cmake to suit your needs!

## building for windows
- manually modify reflect_settings.cmake and set BUILD_FOR_PLATFORM to "WINDOWS".
- run 'cmake .' in folder



## building for android
- manually modify reflect_settings.cmake and set BUILD_FOR_PLATFORM to "ANDROID".
- open android/ in android studio and build the project (using gradle).


## reflect usage

- DO NOT INCLUDE SPACE IN THE NAMES OF ANY CREATED FILE/FOLDER. REFLECT MIGHT BREAK.
- leave app by closing the window, not by closing the terminal. Closing the app by closing the terminal results in memory leaks.
- in order to run reflect from nvidia nsystems, you have to set VULKAN_DEBUG_UTILS_ENABLE to "n" in setting (and rebuild).