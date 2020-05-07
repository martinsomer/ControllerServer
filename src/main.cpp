#include "pch.hpp"
#include "server.hpp"

uint32_t device = 1;

int main(void) {
	if (!isVBusExists()) {
		std::cout << "ScpVBus driver not installed." << std::endl;
		return 1;
	}
	
    while (!PlugIn(device)) {
        if (device == 4) {
            std::cout << "Failed to plug in device." << std::endl;
			return 1;
        }
        
        device++;
    }
    
	startServer();
	
	if (!UnPlug(device)) {
		if (!UnPlugForce(device)) {
			std::cout << "Failed to unplug device." << std::endl;
			return 1;
		}
	}
	
	return 0;
}

