#include "imgui.h"
#include "imgui_impl_sdl.h" /* ImGui_ImplSDL2_InitForSDLRenderer -- tag hop */
/* #include "imgui_impl_opengl3.h" */
#include "imgui_impl_sdlrenderer.h" /* ImGui_ImplSDLRenderer_Init -- tag hop */
#include <stdio.h>
#include <SDL.h>
#include "window_info.h"

#include <iostream>

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
        { // print version : C++ style
            using namespace std;
            cout << "SDL Version: ";
            cout << static_cast<int>(ver.major) << ".";
            cout << static_cast<int>(ver.minor) << ".";
            cout << static_cast<int>(ver.patch) << endl;
        }
        { // print version : C style
            printf("SDL Version: %d.%d.%d\n", ver.major, ver.minor, ver.patch);
        }
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
    { // Enable keyboard and gamepad controls (See demo -> Help -> USER GUIDE)
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
    }
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(win, ren);
    ImGui_ImplSDLRenderer_Init(ren);

    // Initial State

    bool quit = false;
    bool show_demo_window = true;
    bool show_metrics_window = false;
    bool show_user_guide = false;
    while(  quit == false  )                                    // Game loop
    {
        // Update state
        SDL_GetWindowSize(win, &wI.w, &wI.h);
        SDL_GetWindowPosition(win, &wI.x, &wI.y);

        // UI
        { // Polled
            SDL_Event e;
            SDL_Keymod kmod = SDL_GetModState();                // kmod : OR'd modifiers
            while(  SDL_PollEvent(&e)  )
            {
                if(  e.type == SDL_KEYDOWN  )
                {
                    switch(  e.key.keysym.sym  )
                    {
                        // Dear ImGui uses Escape to exit text input.
                        // So don't use Escape to quit the program.
                        // And Escape with modifier keys is not recognized.
                        // Just use Alt-q / Ctrl-q.
                        case SDLK_q:                            // quit : Alt-q or Ctrl-q
                            if(  (kmod&KMOD_ALT) || (kmod&KMOD_CTRL)  ) quit = true;
                            break;
                        default: break;
                    }
                }
                ImGui_ImplSDL2_ProcessEvent(&e);                // ImGui react to mouse/keys
            }
        }

        { // Start using ImGui
            ImGui_ImplSDLRenderer_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();
        }
        { // Make ImGui Windows
            using namespace ImGui;
            if(  show_demo_window  ) ShowDemoWindow(&show_demo_window);
            if(  show_metrics_window  ) ShowMetricsWindow(&show_metrics_window);
            if(  show_user_guide  ) ShowUserGuide();

            ImGuiIO& io = GetIO();                              // Mouse Position
            SetNextWindowBgAlpha(0.35f);                        // Transparent bgnd
            Begin("Alt-Q to quit");                             // Create window
            { // Display frame rate
                Text("Application average %.3f ms/frame (%.1f FPS)",
                        1000.0f / GetIO().Framerate,         // ms per frame
                        GetIO().Framerate);                  // FPS
            }
            { // Display Window size
                Text("Window: %dx%d at (%d,%d)", wI.w, wI.h, wI.x, wI.y);
            }
            { // Display ImGui window locations
                ImVec2 wp = GetWindowPos();
                Text("Window Position: (%.1f,%.1f)", wp.x, wp.y);
            }
            { // Display Mouse position
                ImVec2 mp = io.MousePos;
                Text("Mouse Position: (%.1f,%.1f)", mp.x, mp.y);
            }
            End();
        }

        // Render
        { // Grey background
            SDL_SetRenderDrawColor(ren, 50, 50, 50, 0);         // Alpha doesn't matter
            SDL_RenderClear(ren);
        }
        { // Draw ImGui Windows
            ImGui::Render();
            ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        }
        { // Display to screen
            SDL_RenderPresent(ren);
        }

    }

    // ImGui Shutdown
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // Shutdown
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
