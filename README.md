This is a minimal C++ build setup using IMGUI with SDL2. I'm
using SDL as the renderer as well (it's not recommended but it's
easy).

I'm simultaneously learning Dear ImGui and C++, so I'm
intentionally using C++ header files, but they are not required
to use Dear ImGui and Dear ImGui itself does not depend on any
C++ header files.

Dear ImGui just uses a few C++ language features: namespace,
constructors, and templates (see https://imgui-test.readthedocs.io/en/latest/#).

# Minimal Source

## Get a working C++ build

Make a `Makefile`. Leave it empty.

Make a `main.cpp`:

```c
int main()
{
    return 42;
}
```

Build (note there is no `main` target defined in the Makefile,
this just works!):

```bash
$ make main
g++     main.cpp   -o main
```

Run:

```bash
$ ./main.exe
$ echo $?
42
```

## Add the SDL lib

Get the C++ code to build with the SDL lib. Add flags to the
Makefile:

```make
CXXFLAGS = -std=c++11
CXXFLAGS += -Wall -Wpedantic
CXXFLAGS += `pkgconf --cflags sdl2`
LDLIBS = `pkgconf --libs sdl2`
```

And add SDL to `main.cpp`:

```c
#include <SDL.h>
int main(int argc, char *argv[])
{
    return 42;
}
```

Build and run:

```bash
$ make main
g++ -std=c++11 -Wall -Wpedantic `pkgconf --cflags sdl2`    main.cpp  `pkgconf --libs sdl2` -o main
$ ./main.exe
$ echo $?
42
```

- `SDL.h` must be included to avoid linker error 

    ```
    C:/msys64/mingw64/lib/libSDL2main.a(SDL_windows_main.c.obj):(.text+0x152):
    undefined reference to `SDL_main'
    ```

- `main` must have the `int` and `char**` args to match `SDL_main`

Do something with the args passed to main -- print them to stdout:

```c
#include <SDL.h>
int main(int argc, char *argv[])
{
    for(int i=0; i<argc; i++) puts(argv[i]);
    return 0;
}
```

Build and run:

```bash
$ make main
$ ./main.exe
C:\msys64\home\mike\gitrepos\cpp\min-cpp\main.exe
```

`puts()` is in `stdio.h`, but `SDL.h` already includes that, so I
didn't bother.

## First bit of C++

So far this has been regular C code but compiled with `g++`.

Print the args again, but C++ style:

```c
#include <SDL.h>
#include <iostream>
int main(int argc, char *argv[])
{
    for(int i=0; i<argc; i++) std::cout<<argv[i]<<std::endl;
    return 0;
}
```

Use this scoped form of namespace resolution to make the code
line a little shorter:

```c
#include <SDL.h>
#include <iostream>
int main(int argc, char *argv[])
{
    using namespace std;
    for(int i=0; i<argc; i++) cout<<argv[i]<<endl;
    return 0;
}
```

# Minimal Makefile

## Context

Dear ImGui library is built from source. I keep a copy of the
ImGui source in a parent directory, but the object files (output
by the compiler) go in this project folder (not the ImGui source
folder).

It takes a while to build those object files (a solid five
seconds or so), so I don't do my usual unity build (`make -B`).
Instead, I actually use `make` the way it was originally
intended: it doesn't rebuild stuff that hasn't changed.

I still force a rebuild of my `main.o` -- I do that by adding a
`make clean` step in my Vim build shortcut. The `clean` recipe
only erases my object file and the final `.exe`, it does not
touch the Dear ImGui object files.

## Implementation

EXE and SRC do *not* have to be the same name. But keeping it
simple, I name the source `main.cpp`, and the executable
`main.exe`, so both EXE and SRC are `main`.

Since `main` is the first target in the file, it's the default
target (I named it `default-target` to make this extra obvious),
so running `make` will build `main.exe`:

```make
EXE = main
default-target: $(EXE)
SRC = main
```

I put the `imgui` source in a parent directory because I am lazy:

```make
IMGUI_DIR = ../../imgui/
```

I dump all the `.o` files in the project folder:

```make
SOURCES = $(SRC).cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp
SOURCES += $(IMGUI_DIR)/imgui_draw.cpp
SOURCES += $(IMGUI_DIR)/imgui_demo.cpp
SOURCES += $(IMGUI_DIR)/imgui_tables.cpp
SOURCES += $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdlrenderer.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
```

Note when I eventually switch from the SDL Renderer to the
`OpenGL` renderer, the last line of `SOURCES +=` changes to this:

```make
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
```

I usually include `-Wextra` but that flags stuff in IMGUI, so I
leave that flag out.

I randomly picked C++11 as the standard. According to [the
docs](https://imgui-test.readthedocs.io/en/latest/#), Dear ImGui
does not even require C++11, so older standards should work too.

```make
CXXFLAGS = -std=c++11
CXXFLAGS += -Wall -Wpedantic
CXXFLAGS += -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS += `pkgconf --cflags sdl2`
LDLIBS = -lgdi32 -lopengl32 -limm32
LDLIBS += `pkgconf --libs sdl2`
```

Almost all of the build recipes are:

```make
	$(CXX) $(CXXFLAGS) -c -o $@ $<
```

`CXX` is `g++`.

Typical `gcc` (`g++`) build command goes like this:

`compiler  compiler-flags  processing-step-flags  outfile  infile`

- `-c` is compile only, no linking
    - so the output is an object file
- `-o outfile` tells `make` what file to output to
- `$@` is the outfile
    - `$@` is a `make` variable that holds the target
    - the target is the object file name, `blah.o`
- `$<` is the infile (or infiles)
    - `$<` is a list of all the prerequisites
    - there is only one prerequisite to build the object file
    - the prerequisite is one of the `.cpp` source files,
      `blah.cpp`

Here are the object file recipes:

```make
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: $(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: $(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
```

And now the final build.

The linker flags have to go at the end of the `EXE` recipe. If I
put them next to the compiler flags, I get a ton of linker
errors.

```make
$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)
```

## Extra make recipes

### clean

In my usual SDL C project I just rebuild everything. But here I'm
using the IMGUI library and it takes a while to compile, so I
only want to rebuild my `main` source file.

In Vim, I recognize the difference between a pure SDL project and
an IMGUI project by checking the filetype -- if it's C++, I
assume it's an IMGUI project, if it's C then it's pure SDL (it
does not use IMGUI).

To force a rebuild of `main.o` and `main.exe`, my Vim shortcut
calls `make clean` before calling `make`.

```make
.PHONY: clean
clean:
	rm $(EXE).exe
	rm $(SRC).o
```

This is just to see the names of the object files:

```make
.PHONY: list-objs
list-objs:
	@echo $(OBJS)
```

### tags

This prints human-readable tags to stdout. Since I invoke this
with `make`, Vim puts in the quickfix window and it becomes a
quick reference to variable names in my `main.cpp` and in my
local libs:

```make
.PHONY: show-tags
show-tags: tags
	@echo -e \n\# $(SRC)\n
	@ctags --c-kinds=+l 				--sort=no -x $(SRC).cpp
	@echo -e \n\# LIBS\n
	@ctags --c-kinds=+l -L headers.txt 	--sort=no -x
```

It requires `tags` so that I know the tags file and this print to
stdout are in sync. It also means that `headers.txt` (the list of
local libs) is up-to-date. That file is handy to check the path
of a header.

Here are the recipes for my `tags` files.

A typical `ctags` recipe is just `ctags -R .` -- run ctags on all
the files in this folder. Easy enough to run from the command
line.

Instead, I specify the files: it's all the headers listed in
`headers.txt` (a.k.a., my local libs), and my one-and-only source
file, `main.cpp`.

```make
.PHONY: tags
tags: $(SRC).cpp parse-headers.exe
	@$(CXX) $(CXXFLAGS) $< -M > headers-M.txt
	@./parse-headers.exe M
	@ctags --c-kinds=+l -L headers.txt $(SRC).cpp
```

Parse-headers is a simple C program that takes list of headers
output by the preprocessor with the `-M` flag and reformats it a
little so that `ctags` understands it.

I also generate tags for all the third-party libs. This is the
`lib-tags` recipe. When this recipe runs, `headers.txt` contains
a list of *all* headers, not just my local libs.

```make
.PHONY: lib-tags
lib-tags: $(SRC).cpp parse-headers.exe
	@$(CXX) $(CXXFLAGS) $< -M > headers-M.txt
	@./parse-headers.exe
	@ctags -f lib-tags --c-kinds=+p -L headers.txt
```

The point of `lib-tags` is to tag-preview (to see function
signatures without reading documentation) and tag-complete (to
auto-complete and save my wrists) for third-party libs, like SDL.

# Vim

`;r Space` - run

```vim
nnoremap <leader>r<Space> :call RunThis()<CR>
function RunThis()
    if (&filetype != 'c') && (&filetype != 'cpp')
        echomsg s:sad_kitty 'Filetype is not c or c++'
        return
    endif
    let l:this = expand("%:t:r")
    execute "!./"..l:this
endfunction
```

`;Space` - build

```vim
nnoremap <leader><Space> :call MakeThis()<CR><CR>
function MakeThis()
    if (&filetype != 'c') && (&filetype != 'cpp')
        echomsg s:sad_kitty 'Filetype is not c or c++'
        return
    endif
    let l:this = expand("%:t:r")
    " C : the recipe name is the root of the file name{{{
    "     - invoke the make recipe for the file name root
    "     - rebuild all}}}
    if (&filetype == 'c')
        execute "make -B "..l:this
    " C++ : invoke the default make recipe{{{
    "       - do not rebuild IMGUI .o files
    "       - instead, rm the main.o and main.exe and rebuild from there}}}
    elseif (&filetype == 'cpp')
        execute "make clean"
        execute "make"
    endif
    redraw!
    " Open the quickfix window if the build failed.
    let l:build_failed = CheckIfBuildFailed()
    if l:build_failed
        copen
        echomsg "Build Failed"
    else
        cclose
        echomsg "Build OK"
        endif
    endif
endfunction

function! CheckIfBuildFailed()
    " Return 0 if no valid errors, 1 if there are valid errors.
    let l:all_the_valids = []
    for qfdict in getqflist()
        call add(l:all_the_valids, qfdict['valid'])
    endfor
    return str2nr(join(l:all_the_valids,''))==0 ? 0 : 1
endfunction
```


# Minimal ACTUAL Source for SDL and Dear ImGui

Do all the setup and shutdown for an application using SDL and
Dear ImGui.

IMGUI recommends not using `SDL_Renderer`. Instead, IMGUI
recommends the `SDL_GL` API and using `opengl3`. I will do that
eventually, but for now I'll stick with the `SDL_Renderer`
backend because I know it well.

The IMGUI example is here: `imgui/examples/example_sdl_sdlrenderer/main.cpp`

TODO: do my usual setup/shutdown plus some debug overlay using
IMGUI.

## Setup

Include libraries:

- SDL
    - pre-compiled, see Makefile `$(LDFLAGS)`
- C standard libraries
    - `<stdio.h>`
- C++ standard libraries
    - `<iostream>`
- ImGui:
    - the ImGui libraries are compiled from the source files in
      the `$(IMGUI_DIR)` and `$(IMGUI_DIR)/backends` folders
- My libraries:
    - I make header-only libraries
    - example: `"window_info.h"`
        - application has one main window that it runs in
        - so I make a global struct for the window info
            - members : x, y, w, h, and flags
        - this is a small lib to give me:
            - that global struct
            - a `setup` function to init the struct, either:
                - use default window settings if I launched with
                  no args (Vim : `;r<Space>`)
                - fit the window inside the current Vim window
                  (and force the window to stay on top) if I
                  launched with x,y,w,h arg (Vim : `;2`)

```c
#include "imgui.h"
#include "imgui_impl_sdl.h" /* ImGui_ImplSDL2_InitForSDLRenderer -- tag hop */
/* #include "imgui_impl_opengl3.h" */
#include "imgui_impl_sdlrenderer.h" /* ImGui_ImplSDLRenderer_Init -- tag hop */
#include <SDL.h>
#include "window_info.h"

#include <stdio.h>
#include <iostream>
```

Globals:

```c
SDL_Window *win;
SDL_Renderer *ren;
```

Match SDL function signature for `main()`:

```c
int main(int argc, char *argv[])
```

My typical SDL setup:

```c
    // Setup
    WindowInfo wI; WindowInfo_setup(&wI, argc, argv);
    SDL_Init(SDL_INIT_VIDEO);
    win = SDL_CreateWindow(argv[0], wI.x, wI.y, wI.w, wI.h, wI.flags);
    Uint32 ren_flags = 0;                                       // SDL_RendererFlags
    ren_flags |= SDL_RENDERER_PRESENTVSYNC;                     // 60 FPS; No SDL_Delay()!
    ren_flags |= SDL_RENDERER_ACCELERATED;                      // Use hardware acceleration
    ren = SDL_CreateRenderer(win, -1, ren_flags);
```

And the Dear ImGui setup:

```c
    // ImGui Setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();                               
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;        // Enable Gamepad Controls
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(win, ren);
    ImGui_ImplSDLRenderer_Init(ren);
```

# Why no IMGUI submodule

## Proper way to use IMGUI repo

The usual way to use IMGUI is to enter the project top folder and
do this:

```bash
$ git submodule add https://github.com/ocornut/imgui.git
Cloning into '/home/mike/gitrepos/blah-project/imgui'...
...
$ git add .gitmodules imgui
$ git commit -m 'Add IMGUI as Git submodule'
```

Git creates a `.gitmodules` file:

```.gitmodules
[submodule "imgui"]
        path = imgui
        url = https://github.com/ocornut/imgui.git
```

Then when the project is cloned, the IMGUI submodule is cloned as
a separate step like this:

```bash
git submodule update --init
```

Then if I make lots of little projects, each using IMGUI, I have
a ton of copies of IMGUI taking up disk space. So I start doing
`git submodule deinit` to delete the files from disk when I'm not
actively working on a project, and then `git submodule init` to
restore the IMGUI files when I want to work on the project again.

In that "proper" workflow, the `Makefile` sets the IMGUI
directory like this:

```make
IMGUI_DIR = ./imgui/
```

## Lazy way to use IMGUI repo

But I am being lazy. I am just doing practice projects,
throw-away code. So I have a clone of `imgui` sitting two folder
levels above my `Makefile`. This clone is shared by all my
practice projects.

In this "lazy" workflow, the `Makefile` sets the IMGUI directory
like this:

```make
IMGUI_DIR = ../../imgui/
```

