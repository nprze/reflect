# reflect is an open source game engine

## running reflect

make sure you have cmake installed ( min. v3.16 ).\
make sure you have vulkan SDK installed with glm headers.

- modify reflect_settings.cmake to suit your needs!
- run 'cmake .' in folder

## reflect usage

- DO NOT INCLUDE SPACE IN THE NAMES OF ANY CREATED FILE/FOLDER. REFLECT MIGHT BREAK.
- leave app by closing the window, not by closing the terminal. Closing the app by closing the terminal results in memory leaks.
- in order to run reflect from nvidia nsystems, you have to set VULKAN_DEBUG_UTILS_ENABLE to "n" in setting (and rebuild).