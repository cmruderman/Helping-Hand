CXX := g++
PROJECT := rasp_pi_main
FLAGS := --std=c++11 -g -O2
INCLUDE := ../MotionTracking


all:
	$(CXX) -I $(INCLUDE) $(FLAGS) rasp_pi_to_arduino.cpp ../MotionTracking/arduino-serial-lib.c -o $(PROJECT)

clean:
	rm -f $(PROJECT)
