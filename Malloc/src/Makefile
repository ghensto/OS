all: packet_sender packet_receiver main_mm malloc_mm
.PHONY : all

packet_sender: packet_sender.c mm.o
	gcc -o packet_sender packet_sender.c mm.o

packet_receiver: packet_receiver.c mm.o
	gcc -o packet_receiver packet_receiver.c mm.o

main_mm: main_mm.c mm.o
	gcc -o main_mm main_mm.c mm.o

malloc_mm: main_malloc.c mm.o
	gcc -o malloc_mm main_malloc.c mm.o

mm.o: mm.c
	gcc -o mm.o -c mm.c

clean:
	rm -rf mm.o main_mm malloc_mm packet_sender packet_receiver
