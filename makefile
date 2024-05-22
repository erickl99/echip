CC = gcc
exec = echip
flags = -Wall

echip:
	cc $(flags) -o build/$(exec) src/echip.c src/vm.c src/graphics.c src/audio.c -lX11 -lasound

.PHONY: clean
clean:
	rm build/$(exec) src/*.o
