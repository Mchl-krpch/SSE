TARGET = main

# opengl32.lib
# sfml-system-d.lib
# sfml-window-d.lib
# sfml-graphics-d.lib
# sfml-main-d.lib

all: execute

execute: $(TARGET).exe
	$(TARGET)

$(TARGET).exe: $(TARGET).cpp
	g++ -mavx -mavx512f -mfma -I"C:\Users\Xiaomi\Documents\C\SSE\test\SFML-2.5.1\include" -c $(TARGET).cpp -DSFML_STATIC
	g++ $(TARGET).o -L"C:\Users\Xiaomi\Documents\C\SSE\test\SFML-2.5.1\build\lib" -lopengl32 -lsfml-system -lsfml-window -lsfml-graphics -lsfml-main -o $(TARGET).exe

clean:
	del *.a mandelbrot.o *.exe