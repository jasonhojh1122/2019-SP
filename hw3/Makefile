default:
	gcc main.c -o main
	gcc scheduler.o hw3.c -o hw3


debug:
	gcc -DDEBUG main.c -o main
	gcc -DDEBUG scheduler.o hw3.c -o hw3

gdb:
	gcc -g main.c -o main
	gcc -g scheduler.o hw3.c -o hw3

	
clean:
	rm -f hw3 main