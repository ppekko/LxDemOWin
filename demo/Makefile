OBJS	= main.o
SOURCE	= main.c
OUT	= 0
CC	 = clang
FLAGS	 = -nostartfiles  -fomit-frame-pointer  -fmerge-all-constants -fno-math-errno -Wall -ffast-math  -fno-unroll-loops -Os -nodefaultlibs
LFLAGS	 = -lpthread -lGL -lX11 -lc -ldl


all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS) $(FLAGS)
	strip -s -R .comment -R .gnu.version $(OUT)
	sstrip $(OUT)
	./bettergz.sh $(OUT)



main.o: main.c
	$(CC) -g -c $(FLAGS) main.c

clean:
	rm -f $(OBJS) $(OUT)

run: $(OUT)
	./$(OUT)

