# Makefile for simple application based on SDL2 library
# for windows 10.

# Because SFML I used in Mandelbrot app

CC     = g++
RES_CC = windres

CC_FLAGS = -mavx -mavx512f -mfma

RES_TARGET = res.rc
TARGETS    = main.cpp compose.cpp
EXECUTABLE = comp.exe
OBJS       = $(TARGETS:.cpp=.o)

# Add flag '-mwindows' if you dont need to write in console 
CONSOLE = -mwindows

# Path to folders.
	INCLUDE_FOLDER =  SDL2/include/
	LIBRARY_FOLDER =  SDL2/lib/x64/
	LIBS           = -lSDL2 -lSDL2main
	BUILD_FOLDER   =  build

all: clean res execute

execute: $(BUILD_FOLDER)\\$(EXECUTABLE)
	$(BUILD_FOLDER)\\$(EXECUTABLE)

res: $(RES_TARGET)
	$(RES_CC) $(RES_TARGET) -O coff -o $(RES_TARGET:.rc=.res)

$(BUILD_FOLDER)\\$(EXECUTABLE): $(OBJS)
	$(CC) $(CONSOLE) $(OBJS) $(RES_TARGET:.rc=.res) -L$(LIBRARY_FOLDER) $(LIBS) \
				  -o $(BUILD_FOLDER)\\$(EXECUTABLE)

$(OBJS): %.o:%.cpp
	@$(CC) $(CC_FLAGS) -I$(INCLUDE_FOLDER) -c $< -o $@

clean:
	del $(BUILD_FOLDER)\\$(EXECUTABLE) *.o *.res
	cls