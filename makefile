CC = gcc
exec = echip
flags = -Wall

echip:
	cc $(flags) -o build/$(exec) src/echip.c -lX11

.PHONY: clean
clean:
	rm build/$(exec) src/*.o