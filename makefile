CC = arm-linux-gnueabihf-g++
IDIR = include
CFLAGS = -I$(IDIR)
OPTIONS = -O0 -g3 -fmessage-length=0

ODIR = obj
LDIR = ../lib

LIBS = -lm -lncurses -lpthread

_DEPS = defs.h i2cdev.h GPIO.h mpu6050.h mpu9250.h ak8963.h
DEPS = $(pathsubst %, $(IDIR)/%,$(_DEPS))

_OBJ = test.o i2cdev.o GPIO.o mpu6050.o mpu9250.o ak8963.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJ)
	$(CC) $(OPTIONS) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~

run: all
	./all
