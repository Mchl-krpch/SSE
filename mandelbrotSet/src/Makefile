
# 	   MODE OF COMPILATION
#   There are 3 different modes to compile:

#   1. Debug mode to check compilation but not execute *.exe file.
#      Also this mode comprise deleting all *.o *.exe files after
#      comiling
#      TYPE   'DEBUG'   to select this mode

#    2. Execute mode to compile project and execute it. Also this
#       mode saves all *.exe, *.o files
#       TYPE   'RUN'     to select this mode

	MAKE_MODE = RUN

# Compiler settings.
	CC     = g++
	RES_CC = windres
	CFLAGS = -mavx -mavx512f -mfma
	LIBS   = -lsfml-graphics -lsfml-window -lsfml-system
	QUITE  = 

# Write here '-mwindows' and app will be open without cmd
	CONSOLE = -mwindows

# Folders with library on computer
	INCLUDE_PTR = "SFML-2.5.1\\include"
	LIBRARY_PTR = "SFML-2.5.1\\build\\lib"
	BUILD_LIB   = ..\\build\\

# Files to compile.
	TARGETS     = main.cpp
	TARGET_RES = res\\mandelbrotInfo.rc

# Output file name.
	EXECUTABLE = mandelbrotSet.exe

# Check different types of make process
ifeq ($(MAKE_MODE), DEBUG)
all: compile clean
endif
ifeq ($(MAKE_MODE), RUN)
QUITE += @
all: compile execute
endif

execute: $(BUILD_LIB)$(EXECUTABLE)
	$(QUITE)$(BUILD_LIB)$(EXECUTABLE)

compile: $(TARGETS) resFile
	$(QUITE) cls
	$(QUITE) $(CC) $(CFLAGS) -I$(INCLUDE_PTR) -c $(TARGETS)  \
		-o $(BUILD_LIB)$(TARGETS:.cpp=.o)
	$(QUITE) $(CC) $(CONSOLE) $(BUILD_LIB)$(TARGETS:.cpp=.o) \
		$(TARGET_RES:.rc=.res) -L$(LIBRARY_PTR) $(LIBS)      \
		-o $(BUILD_LIB)$(EXECUTABLE)

resFile: $(TARGET_RES)
	$(QUITE) $(RES_CC) $(TARGET_RES) -O coff \
		-o $(TARGET_RES:.rc=.res)

clean:
	del $(BUILD_LIB)$(EXECUTABLE) \
	$(BUILD_LIB)*.o \
	$(TARGET_RES:.rc=.res)