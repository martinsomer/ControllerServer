/*
 * This is the main program.
 *
 * It checks if ScpVBus driver is installed, then creates
 * a virtual controller, and finally starts the server.
 * The controller is disconnected upon program termination.
 */

#include "pch.hpp"
#include "server.hpp"

UINT device = 1;

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

