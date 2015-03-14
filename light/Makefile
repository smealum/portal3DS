.PHONY: all clean

all: light.exe

clean:
	@rm build/*
	@rm light.exe

light.exe: build/light_main.o build/light_room.o build/light_room_io.o build/light_math.o
	gcc -o $@ $^

build/%.o: source/%.c
	gcc -c -o $@ $^
