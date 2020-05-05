#include "pch.hpp"
#include "server.hpp"

uint32_t device;

int main(void) {
	if (!isVBusExists()) {
		std::cout << "ScpVBus driver not installed." << std::endl;
		return 1;
	}
	
	for (device = 1; device < 5; device++) {
		if (PlugIn(device)) {
			break;
		} else if (device == 4) {
			std::cout << "Failed to plug in device." << std::endl;
			return 1;
		}
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

