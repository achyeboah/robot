CC=g++
IDIR = include
IDIR2 = /home/sam/src/glm/

CFLAGS = -I$(IDIR) -I$(IDIR2)
OPTIONS = -O0 -g -g3 -fmessage-length=0 -Wall

ODIR = obj

# LIBS = -lm -lncurses -lpthread -lGLU -lGL -lglfw3 -ldl -L/home/sam/src/openGLrobot/lib/ -lGLEW
LIBS = -lm -lncurses -lpthread -lGLU -lGL -lglfw3 -ldl -lGLEW

_DEPS = defs.h i2cdev.h GPIO.h imu.h robotGL.h robotCurses.h robotSeg.h stb_image.h utils.h
DEPS = $(patsubst %, $(IDIR)/%,$(_DEPS))

_OBJ = robot.o i2cdev.o GPIO.o imu.o robotGL.o robotCurses.o robotSeg.o utils.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) $(OPTIONS) -c -o $@ $< $(CFLAGS)

$(ODIR_cross)/%.o: %.cpp $(DEPS)
	$(CC_cross) $(OPTIONS) -c -o $@ $< $(CFLAGS)

all: robot 

robot: $(OBJ) 
	$(CC) $(OPTIONS) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ robot

run: robot
	./robot
