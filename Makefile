CC = gcc
FLAGS = -o

EXE = miniftp

all : clean $(EXE)

miniftp :
	$(CC) $(FLAGS) $(EXE) Main.c TCP.c TEA.c DH.c Common.c Request.h

clean : 
	rm -f miniftp