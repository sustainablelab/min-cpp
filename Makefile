EXE = main
default-target: $(EXE)
SRC = main
IMGUI_DIR = ../../imgui/
SOURCES = $(SRC).cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp
SOURCES += $(IMGUI_DIR)/imgui_draw.cpp
SOURCES += $(IMGUI_DIR)/imgui_demo.cpp
SOURCES += $(IMGUI_DIR)/imgui_tables.cpp
SOURCES += $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl.cpp
# SOURCES += $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdlrenderer.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

CXXFLAGS = -std=c++11
CXXFLAGS += -Wall -Wpedantic
CXXFLAGS += -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS += `pkgconf --cflags sdl2`
LDLIBS = -lgdi32 -lopengl32 -limm32
LDLIBS += `pkgconf --libs sdl2`

# gcc [-c|-S|-E] [-std=standard] [-g] [-pg] [-Olevel]
#     [-Wwarn...] [-Wpedantic] [-Idir...] [-Ldir...]
#     [-Dmacro[=defn]...] [-Umacro] [-foption...] [-mmachine-option...]
#     [-o outfile] [@file] infile...
#
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: $(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: $(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	rm $(EXE).exe
	rm $(SRC).o

.PHONY: list-objs
list-objs:
	@echo $(OBJS)

.PHONY: show-tags
show-tags: tags
	@echo -e \n\# $(SRC)\n
	@ctags --c-kinds=+l 				--sort=no -x $(SRC).cpp
	@echo -e \n\# LIBS\n
	@ctags --c-kinds=+l -L headers.txt 	--sort=no -x

.PHONY: tags
tags: $(SRC).cpp parse-headers.exe
	@$(CXX) $(CXXFLAGS) $< -M > headers-M.txt
	@./parse-headers.exe M
	@ctags --c-kinds=+l -L headers.txt $(SRC).cpp

.PHONY: lib-tags
lib-tags: $(SRC).cpp parse-headers.exe
	@$(CXX) $(CXXFLAGS) $< -M > headers-M.txt
	@./parse-headers.exe
	@ctags -f lib-tags --c-kinds=+p -L headers.txt

parse-headers.exe: parse-headers.c
	$(CC) -Wall $< -o $@
