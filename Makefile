# makefile filecopy
# Candussi Ricardo
# 4.4.2016
CC=gcc
CFLAGS=-std=c99 -Wall -pedantic
DFLAGS=-g
RMFLAGS=-R 

OBJECTS_RCV=filecopy_rcv.o
OBJECTS_SND=filecopy_snd.o


%.o : %.c
	$(CC) $(CFLAGS) -c $<
	
all: filecopy_snd filecopy_rcv

rcv: $(OBJECTS_RCV)
	$(CC) $(DFLAGS) $(CFLAGS) $^ -o filecopy_rcvD
	
snd: $(OBJECTS_SND)
	$(CC) $(DFLAGS) $(CFLAGS) $^ -o filecopy_sndD
	
clean:
	$(RM) $(RMFLAGS) *.o *~ filecopy_snd filecopy_rcv filecopy_sndD filecopy_rcvD *.dSYM
