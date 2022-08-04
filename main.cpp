#include "imgui.h"
#include "imgui_impl_sdl.h" /* ImGui_ImplSDL2_InitForSDLRenderer -- tag hop */
/* #include "imgui_impl_opengl3.h" */
#include "imgui_impl_sdlrenderer.h" /* ImGui_ImplSDLRenderer_Init -- tag hop */
#include <stdio.h>
#include <SDL.h>
#include "window_info.h"

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

SDL_Window *win;
SDL_Renderer *ren;

int main(int argc, char *argv[])
{
    for(int i=0; i<argc; i++) puts(argv[i]);
    { // Print the version of SDL
        SDL_version ver;
        SDL_GetVersion(&ver);
        printf("SDL Version: %d.%d.%d\n", ver.major, ver.minor, ver.patch);
    }

    // Setup
    WindowInfo wI; WindowInfo_setup(&wI, argc, argv);
    SDL_Init(SDL_INIT_VIDEO);
    win = SDL_CreateWindow(argv[0], wI.x, wI.y, wI.w, wI.h, wI.flags);
    Uint32 ren_flags = 0;                                       // SDL_RendererFlags
    ren_flags |= SDL_RENDERER_PRESENTVSYNC;                     // 60 FPS; No SDL_Delay()!
    ren_flags |= SDL_RENDERER_ACCELERATED;                      // Use hardware acceleration
    ren = SDL_CreateRenderer(win, -1, ren_flags);

    // ImGui Setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(win, ren);
    ImGui_ImplSDLRenderer_Init(ren);

    bool quit = false;
    bool show_demo_window = false;
    bool show_metrics_window = true;
    while(  quit == false  )
    {
        // Update state
        SDL_GetWindowSize(win, &wI.w, &wI.h);

        // UI
        { // Polled
            SDL_Event e;
            while(  SDL_PollEvent(&e)  )
            {
                if(  e.type == SDL_KEYDOWN  )
                {
                    switch(  e.key.keysym.sym  )
                    {
                        case SDLK_ESCAPE: quit = true; break;
                        default: break;
                    }
                }
                ImGui_ImplSDL2_ProcessEvent(&e);                // ImGui react to mouse/keys
            }
        }

        { // ImGui
            ImGui_ImplSDLRenderer_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();                                  // Start using ImGui in loop
        }
        if(  show_demo_window  ) ImGui::ShowDemoWindow(&show_demo_window);
        if(  show_metrics_window  ) ImGui::ShowMetricsWindow(&show_metrics_window);

        // Render
        { // Grey background
            SDL_SetRenderDrawColor(ren, 50, 50, 50, 0);         // Alpha doesn't matter
            SDL_RenderClear(ren);
        }
        { // ImGui
            ImGui::Render();
            ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        }
        { // Display to screen
            SDL_RenderPresent(ren);
            /* SDL_Delay(10); */
        }

    }

    // ImGui Shutdown
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // Shutdown
    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(ren);
    SDL_Quit();
    return 0;
}
