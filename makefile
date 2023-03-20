all: bank

bank: bank.c
	gcc -pthread -std=gnu11 bank.c -g -o bank
clean:
	rm bank
