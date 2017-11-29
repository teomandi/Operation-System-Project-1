all:
	gcc -o main main.c
	./main -i randomtext.txt -n 6 -c 20
