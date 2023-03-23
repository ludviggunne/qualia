CXX = g++
CFLAGS = -Wall
OUTPUT_DIR = build/

# SOURCE FILES
MAIN_SRC = main.cpp\
			shaders.cpp\
			buffer.cpp\
			texture.cpp\
			gl.cpp

IMGUI_SRC = imgui.cpp\
			imgui_draw.cpp\
			imgui_tables.cpp\
			imgui_widgets.cpp

BACKENDS_SRC = 	imgui_impl_glfw.cpp\
				imgui_impl_opengl3.cpp
GLAD_SRC = glad.c
SRC = $(MAIN_SRC) $(addprefix extern/imgui/, $(IMGUI_SRC)) $(addprefix extern/imgui/backends/, $(BACKENDS_SRC)) $(addprefix extern/glad/src/, $(GLAD_SRC))

# LIBRARIES AND INCLUDES
INCLUDE_DIRS = extern/glad/include/ extern/imgui/ extern/glm/
LIBS = glfw

OBJECTS = $(MAIN_SRC:.cpp=.o) $(IMGUI_SRC:.cpp=.o) $(BACKENDS_SRC:.cpp=.o) $(GLAD_SRC:.c=.o)

default:
	mkdir -p $(OUTPUT_DIR)
	$(CXX) -c $(SRC) $(addprefix -I, $(INCLUDE_DIRS)) $(CFLAGS)
	$(CXX) $(OBJECTS) $(addprefix -l, $(LIBS)) -o $(OUTPUT_DIR)main

clean:
	rm $(OBJECTS) $(OUTPUT_DIR) -r -f