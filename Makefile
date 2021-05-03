CC = g++
CFLAGS = -g -Wall -std=c++17

Main: main.cpp
	$(CC) $(CFLAGS) -o main.out main.cpp

Exec: main.out Main
	./main.out < ./in/0000.txt

ExecAll: main.out Main
	./exec_all.sh
