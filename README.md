# reflect is a small game engine

## before building

- make sure you have cmake installed ( min. v3.16 ).
- make sure you have vulkan SDK installed.
- modify reflect_settings.cmake to suit your needs!

## building for windows
- run 'cmake .' in folder

## building for android
- note: on android, settings are overridden (no modification to reflect_settings.cmake is needed)
- open android/ in android studio and build the project using gradle.


## reflect usage

- DO NOT INCLUDE SPACE IN THE NAMES OF ANY CREATED FILE/FOLDER. REFLECT MIGHT BREAK.
- leave app by closing the window, not by closing the terminal. Closing the app by closing the terminal results in memory leaks.
- in order to run reflect from nvidia nsight systems, you have to set VULKAN_DEBUG_UTILS_ENABLE to "n" in setting (and rebuild).


## author notes
- currently reflect has memory leaks (408bytes), which I can't seem to fix
- in case of bugs feel free to drop an issue 