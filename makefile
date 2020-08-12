CC_cross = arm-linux-gnueabihf-g++
CC=g++
IDIR = include
CFLAGS = -I$(IDIR)
OPTIONS = -O0 -g -g3 -fmessage-length=0

ODIR = obj
ODIR_cross = obj_cross
LDIR = ../lib

LIBS = -lm -lncurses -lpthread

_DEPS = defs.h i2cdev.h GPIO.h mpu6050.h mpu9250.h ak8963.h robotWin.h
DEPS = $(patsubst %, $(IDIR)/%,$(_DEPS))

_OBJ = robotViewer.o i2cdev.o GPIO.o mpu6050.o mpu9250.o ak8963.o robotWin.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))
OBJ_cross = $(patsubst %,$(ODIR_cross)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) $(OPTIONS) -c -o $@ $< $(CFLAGS)

$(ODIR_cross)/%.o: %.cpp $(DEPS)
	$(CC_cross) $(OPTIONS) -c -o $@ $< $(CFLAGS)

all: robot cross

robot: $(OBJ) 
	$(CC) $(OPTIONS) -o $@ $^ $(CFLAGS) $(LIBS)

cross: $(OBJ_cross)
	$(CC_cross) $(OPTIONS) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ robot
	rm -f $(ODIR_cross)/*.o *~ core $(INCDIR)/*~ cross

run: robot
	./robot
