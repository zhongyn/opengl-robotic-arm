
LDLIBS=-lglut -lSOIL -lGLEW -lGL -lm
all: robotArm
clean:
	rm -f *.o arm
robotArm: shader_utils.o
.PHONY: all clean
