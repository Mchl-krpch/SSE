TARGET = mandelbrot

all: execute

execute: $(TARGET)
	./$(TARGET)

$(TARGET): $(TARGET).cpp
	g++ -mavx512f -mfma -o $@ $< -lsfml-graphics -lsfml-system -lsfml-window

clean:
	*.a
