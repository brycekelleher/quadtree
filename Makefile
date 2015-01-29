INCLUDES	= -I/usr/X11R6/include
CPPFLAGS	= -g $(INCLUDES)
LDFLAGS		= -L/usr/X11R6/lib
LDLIBS		= -lGL -lglut

quadtree: quadtree.o

clean:
	rm -rf *.o
	rm -rf quadtree

rebuild: clean quadtree
