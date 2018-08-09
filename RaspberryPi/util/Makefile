CXX := g++
PROJECT := base_rotation_test
FLAGS := -g -O2
INCLUDE := ../../MotionTracking


all:
	$(CXX) -I $(INCLUDE) -O2 base_rotation_test.cpp ../../MotionTracking/arduino-serial-lib.c -o $(PROJECT)

clean:
	rm -f $(PROJECT)
