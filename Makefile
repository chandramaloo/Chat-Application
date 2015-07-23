all:
	gcc -w server1.c -o server1
	gcc -w server2.c -o server2

clean:
	rm -f server1 server2
