OBJS	= main.o
SOURCE	= main.c
HEADER	=
OUT	= lxdemowin
CC	 = gcc
FLAGS	 = -g -c -Wall -fomit-frame-pointer -mfpmath=387 -mfancy-math-387 -fmerge-all-constants -fsingle-precision-constant  -fno-math-errno -Wall -ffast-math  -fno-unroll-loops -Os -nodefaultlibs
LFLAGS	 = -lpthread -lGL -lX11 -lc -ldl


all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS) -nostartfiles -e _start
	strip -s -R .comment -R .gnu.version $(OUT)
	sstrip $(OUT)
	./bettergz.sh $(OUT)



main.o: main.c
	$(CC) $(FLAGS) main.c -lcuni


clean:
	rm -f $(OBJS) $(OUT)

run: $(OUT)
	./$(OUT)

