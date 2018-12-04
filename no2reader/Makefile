EXE = hidr

CC = gcc
CFLAG = -Wall -O0 -g3 -gdwarf-2
LD = -ludev -lpthread

all: $(EXE)

hidr: hidr.c hidr.h readpodac.c readpodac.h datamap.c datamap.h hidapi.c hidapi.h
	$(CC) $(CFLAG) -o $@ $^ $(LD)

clean:
	rm -f $(EXE)

