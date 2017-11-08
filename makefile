main: mem_sim.c main.c
	gcc -Wall main.c -o main
clean:
	rm main swap
