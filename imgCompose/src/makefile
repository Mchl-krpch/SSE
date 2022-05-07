# Makefile for simple application based
# on SDL2 library for windows 10.
# Because SFML I used in Mandelbrot app

# Compilers in project.
# Standarg compiler for files *.cpp
	CC         = g++

# Compiler for *.rc files
	RES_CC     = windres

# Compilation flags for std c++ compiler.
	CC_FLAGS   = -mavx -mavx512f -mfma -O0

	RES_TARGET = res.rc
# 	TARGETS    = main.cpp compose.cpp
	TARGETS    = benchmark.cpp compose.cpp
	OBJS       = $(TARGETS:.cpp=.o)

# Name of output built program.
	EXECUTABLE = comp.exe

# Costomize compilation process.
# Add flag '-mwindows' if you dont need
# to write in console. 
	CONSOLE =

	# Path to folders.
	INCLUDE_FOLDER    =  ../include/SDL2/include/
	LIBRARY_FOLDER    =  ../include/SDL2/lib/x64/

	LIBS              = -lSDL2 -lSDL2main
	IMAGES_TO_COMPOSE = resources\\LPRIMO.bmp resources\\mipt.bmp

	BUILD_FOLDER      =  ..\\build\\
	RESOURCES         =  resources\\

# Compilation.
all: res execute

execute: $(BUILD_FOLDER)$(EXECUTABLE)
	@=$(BUILD_FOLDER)$(EXECUTABLE) $(IMAGES_TO_COMPOSE)

res: $(RESOURCES)$(RES_TARGET)
	@$(RES_CC) $(RESOURCES)$(RES_TARGET) -O coff -o $(RESOURCES)$(RES_TARGET:.rc=.res)

$(BUILD_FOLDER)$(EXECUTABLE): $(OBJS)
	@$(CC) $(CONSOLE) $(OBJS) $(RESOURCES)$(RES_TARGET:.rc=.res) -L$(LIBRARY_FOLDER) $(LIBS) -o$(BUILD_FOLDER)$(EXECUTABLE)

$(OBJS): %.o:%.cpp
	@$(CC) $(CC_FLAGS) -I$(INCLUDE_FOLDER) -c $< -o $@

clean:
	del $(BUILD_FOLDER)$(EXECUTABLE) *.o *.res