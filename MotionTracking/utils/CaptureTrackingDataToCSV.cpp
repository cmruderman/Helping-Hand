#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <chrono>
#include "Leap.h"

#include "../ms_sleep.c"


using namespace Leap;

std::ofstream output_file;

class EventListener : public Listener {
public:
    virtual void onConnect(const Controller&);
    virtual void onFrame(const Controller&);
};

void EventListener::onConnect(const Controller& controller) {
    std::cout << "Connected" << std::endl;
}

void EventListener::onFrame(const Controller& controller) {
    const Frame frame = controller.frame();
    //    std::cout << "Frame id: " << frame.id()
    //        << ", timestamp: " << frame.timestamp()
    //        << ", hands: " << frame.hands().count()
    //        << ", fingers: " << frame.fingers().count();
    HandList hands = frame.hands();
    Hand hand = hands[0];
    
    if (!hand.isValid())
        return;
    
    Vector handCenter = hand.palmPosition();
    
    auto time_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    
    output_file << handCenter.x << ", " << handCenter.y << ", " << handCenter.z /*<< ", " << time_milliseconds*/ << "\n";
    
//    sleep_ms(1);
}

int main(int argc, char** argv) {
    
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " ./path/to/savedata.csv" << std::endl;
        return -1;
    }
    
    output_file.open(argv[1], std::ofstream::out | std::ofstream::app);
    
    if (!output_file.is_open()) {
        std::cerr << "Error: could not open the file '" << argv[1] << "' for writing." << std::endl;
        return -2;
    }
    
    std::cout << "Begin" << std::endl;
    
    EventListener listener;
    Controller controller;
    
    controller.addListener(listener);
    
    // Keep this process running until Enter is pressed
    std::cout << "Press Enter to quit..." << std::endl;
    std::cin.get();
    
    controller.removeListener(listener);
    
    output_file << std::flush;
    output_file.close();
    
    return 0;
}

