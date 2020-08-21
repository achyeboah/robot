CC=g++
CC_cross = arm-linux-gnueabihf-g++
IDIR = include
IDIR2 = /home/sam/src/glm/

CFLAGS = -I$(IDIR) -I$(IDIR2)
OPTIONS = -O0 -g -g3 -fmessage-length=0 -Wall

ODIR = obj
ODIR_cross = obj_cross

# LIBS = -lm -lncurses -lpthread -lGLU -lGL -lglfw3 -ldl -L/home/sam/src/openGLrobot/lib/ -lGLEW
LIBS = -lm -lncurses -lpthread -lGLU -lGL -lglfw3 -ldl -lGLEW

_DEPS = defs.h i2cdev.h GPIO.h mpu6050.h mpu9250.h ak8963.h opengltest.h robotCurses.h robotGL.h
DEPS = $(patsubst %, $(IDIR)/%,$(_DEPS))

_OBJ = robotViewer.o i2cdev.o GPIO.o mpu6050.o mpu9250.o ak8963.o opengltest.o robotCurses.o robotGL.o
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
