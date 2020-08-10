IDIR = include
CC = armgcc
CFLAGS = -I$(IDIR)

ODIR = obj
LDIR = ../lib

LIBS = -lm -lncurses -lpthread

_DEPS = defs.h
DEPS = $(pathsubst %, $(IDIR)/%,(%_DEPS))

_OBJ = main.o
OBJ = $(pathsubst %, %(IDIR)/%, (%_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

robot: $(OBJ)
	$(CC) -g -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~

run: robot
	./robot
