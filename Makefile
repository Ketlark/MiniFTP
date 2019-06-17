CC = gcc
FLAGS = -o

EXE = miniftp

all : clean $(EXE)

miniftp :
	$(CC) $(FLAGS) $(EXE) -fstack-protector Main.c TCP.c TEA.c DH.c Common.c Request.h -lm

clean : 
	rm -f miniftp