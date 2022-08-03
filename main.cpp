#include "imgui.h"
#include "imgui_impl_sdl.h" /* ImGui_ImplSDL2_InitForSDLRenderer -- tag hop */
/* #include "imgui_impl_opengl3.h" */
#include "imgui_impl_sdlrenderer.h" /* ImGui_ImplSDLRenderer_Init -- tag hop */
#include <stdio.h>
#include <SDL.h>

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

int main(int argc, char *argv[])
{
    for(int i=0; i<argc; i++) puts(argv[i]);
    { // Print the version of SDL
        SDL_version ver;
        SDL_GetVersion(&ver);
        printf("SDL Version: %d.%d.%d", ver.major, ver.minor, ver.patch);
    }
    return 0;
}
