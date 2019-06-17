CC = gcc
FLAGS = -o
CFLAGS = -c

NAME_EXE_CLT = miniftp
NAME_EXE_SRV = miniftpd
EXE = miniftp miniftpd

OFILES = TEA.o \
		 DH.o \
		 Main.o \
		 TCP.o \
		 Common.o \


all : $(EXE)

#O Files

DH.o : DH.c DH.h
	$(CC) -c DH.c -o DH.o

TEA.o : TEA.c TEA.h
	$(CC) -c TEA.c -o TEA.o

Main.o : Main.c 
	$(CC) -c Main.c -o Main.o

TCP.o : TCP.c TCP.h
	$(CC) -c TCP.c -o TCP.o

Common.o : Common.c Common.h
	$(CC) -c Common.c -o Common.o


#executables

miniftp : $(OFILES)
	$(CC) -o $(NAME_EXE_CLT) $(OFILES) -lm

miniftpd : $(OFILES)
	$(CC) -o $(NAME_EXE_SRV) $(OFILES) -lm

clean : 
	rm -f miniftp