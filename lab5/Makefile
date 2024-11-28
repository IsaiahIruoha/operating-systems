CFLAGS=-g -fno-stack-protector


all: selfcomp exploit.lst client

exploit.lst: exploit.nasm
	nasm -l exploit.lst -f bin exploit.nasm

selfcomp: selfcomp.o
	cc -g -z execstack -o selfcomp selfcomp.o

client: client.o
	cc -g -o client client.o
