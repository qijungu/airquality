objs = opcread.o usbissspi.o opcn2.o
exe = opcread

#MODE=debug

ifndef MODE
MODE = release
endif
ifeq ($(MODE), debug)
FLAGS = -Wall -O0 -g -std=gnu99 -DDEBUG
else 
FLAGS = -Wall -O3 -std=gnu99
endif
LIBS = -lpthread

all: $(exe)

$(exe): $(objs)
	gcc -o $(exe) $(objs) $(LIBS)

$(objs): %.o: %.c %.h
	gcc $(FLAGS) -o $@ -c $<

clean:
	rm -rf $(objs) $(exe) $(testobjs)
	
testobjs = opcread.test.o usbissspi.o opcn2.o

opcread.test.o: opcread.test.c opcread.h
	gcc $(FLAGS) -o $@ -c $<

test: $(testobjs)
	gcc -o $(exe).test $(testobjs)

