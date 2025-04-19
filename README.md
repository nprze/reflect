# reflect is a small game engine
for now reflect can only run in debug configuration

## before building

- make sure you have cmake installed ( min. v3.16 ).
- make sure you have vulkan SDK (min. vulkan 1.2) installed.
- modify reflect_settings.cmake to set the correct platform you will be building for

## building for windows

#### before building:
- note that reflect for windows can only be build by msvc compiler
- modify reflect_settings.cmake to suit your needs
#### build:
- run 'cmake .' in folder
- open .sln file 

## building for android
#### before building:
- note that reflect for android can only be build by clang compiler (in android studio)
- note: on android, settings are overridden (no modification to reflect_settings.cmake is needed)
- make sure you have android NDK installed
#### build:
- open android/ in android studio and build the project using gradle.


## reflect usage

- DO NOT INCLUDE SPACE IN THE NAMES OF ANY CREATED FILE/FOLDER. REFLECT MIGHT BREAK.
- leave app by closing the window, not by closing the terminal. Leaving the app by closing the terminal results in memory leaks.
- in order to run reflect from nvidia nsight systems, set VULKAN_DEBUG_UTILS_ENABLE to "n" in settings (and rebuild).


## author notes

- currently reflect has memory leaks (408bytes), which I can't seem to fix
- in case of bugs feel free to drop an issue 