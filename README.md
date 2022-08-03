This is a minimal C++ build setup using IMGUI with SDL2
~and OpenGL3~.

# Minimal Source

## Get build to work

The `main.cpp` file looks like a regular C file.

```c
#include <SDL.h>
int main(int argc, char *argv[])
{
    for(int i=0; i<argc; i++) puts(argv[i]);
    return 0;
}
```

- `SDL.h` must be included to avoid linker error 

    ```
    C:/msys64/mingw64/lib/libSDL2main.a(SDL_windows_main.c.obj):(.text+0x152):
    undefined reference to `SDL_main'
    ```

- `main` must have the `int` and `char**` args to match `SDL_main`

## Minimal actual source

Do all the SDL setup and shutdown.

IMGUI recommends not using `SDL_Renderer`. Instead, IMGUI
recommends the `SDL_GL` API and using `opengl3`. I will do that
eventually, but for now I'll stick with the `SDL_Renderer`
backend because I know it well.

The IMGUI example is here: `imgui/examples/example_sdl_sdlrenderer/main.cpp`

TODO: do my usual setup/shutdown plus some debug overlay using
IMGUI.

# Minimal Makefile

EXE and SRC do not have to be the same name. Keeping it simple,
the source is `main.cpp`, and the executable is `main.exe`, so
both EXE and SRC are `main`. And since `main` is the first target
in the file, it's the default target, so running `make` will
build `main.exe`:

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
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
```

I usually include `-Wextra` but that flags stuff in IMGUI. I
randomly picked C++11 as the standard.

```make
CXXFLAGS = -std=c++11
CXXFLAGS += -Wall -Wpedantic
CXXFLAGS += -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS += `pkgconf --cflags sdl2`
LDLIBS = -lgdi32 -lopengl32 -limm32
LDLIBS += `pkgconf --libs sdl2`
```

`CXX` is `g++`. Typical `gcc` (`g++`) command goes:

`compiler  compiler-flags  processing-step-flags  outfile  infile`

```make
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: $(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: $(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
```

Everything follows that except the final build. The linker flags
have to go at the end of the recipe. If I put them next to the
compiler flags, I get a ton of linker errors.

```make
$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)
```

## Extra make recipes

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

It requires `tags` so that I know the tags file and this print
out are in sync. It also means that `headers.txt` (the list of
local libs) is up-to-date.

Here are the recipes for my `tags`. A typical `ctags` recipe is
just `ctags -R .` -- run ctags on all the files in this folder.
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

# Why no IMGUI submodule

## Proper way to use IMGUI

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

## Lazy way to use IMGUI

But I am being lazy. I am just doing practice projects,
throw-away code. So I have a clone of `imgui` sitting two folder
levels above my `Makefile`. This clone is shared by all my
practice projects.

In this "lazy" workflow, the `Makefile` sets the IMGUI directory
like this:

```make
IMGUI_DIR = ../../imgui/
```
