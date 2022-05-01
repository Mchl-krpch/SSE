
# 	MODE OF COMPILATION
# There are 3 different modes to compile:

#   1. Debug mode to check compilation but not execute *.exe file.
#      Also this mode comprise deleting all *.o *.exe files after
#      comiling
#      TYPE   'DEBUG'   to select this mode

#    2. Execute mode to compile project and execute it. Also this
#       mode saves all *.exe, *.o files
#       TYPE   'RUN'     to select this mode

#    3. Release mode make finished project which you can use on pc
#       TYPE   'RELEASE' to select this mode

	MAKE_MODE = RUN


# Compiler settings.
CC     = g++
RES_CC = windres
CFLAGS = -mavx -mavx512f -mfma
LIBS   = -lsfml-graphics -lsfml-window -lsfml-system
QUITE  = 

# Folders with library on computer
	INCLUDE_PTR = "C:\Users\Xiaomi\Documents\C\SSE\test\SFML-2.5.1\include"
	LIBRARY_PTR = "C:\Users\Xiaomi\Documents\C\SSE\test\SFML-2.5.1\build\lib"
	BUILD_LIB   = build

# Files to compile.
TARGET     = executor
TARGET_RES = my

# Output file name.
EXECUTABLE = mandelbrotSet


# Check different types of make process

ifeq ($(MAKE_MODE), DEBUG)
all: compile clean
endif

ifeq ($(MAKE_MODE), RUN)
QUITE += @
all: compile execute
endif

execute: $(BUILD_LIB)\\$(EXECUTABLE).exe
	$(QUITE)$(BUILD_LIB)\\$(EXECUTABLE)

compile: $(TARGET).cpp
	@cls
	$(QUITE) $(RES_CC) $(TARGET_RES).rc -O coff -o $(TARGET_RES).res
	$(QUITE) $(CC) $(CFLAGS) -I$(INCLUDE_PTR) -c $(TARGET).cpp -o $(BUILD_LIB)\\$(TARGET).o
	$(QUITE) $(CC) -mwindows $(BUILD_LIB)\\$(TARGET).o $(TARGET_RES).res -L$(LIBRARY_PTR) $(LIBS) -o $(BUILD_LIB)\\$(EXECUTABLE).exe

clean:
	del $(BUILD_LIB)\\$(EXECUTABLE).exe $(BUILD_LIB)\\*.o