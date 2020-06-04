/*
 * This is the emulator program.
 *
 * When receiving data from the server, corresponding
 * buttons are pressed on the virtual controller.
 */

#include "pch.hpp"

extern UINT device;

void emulate(char *data) {
	SetTriggerL(device, data[0] ? 255 : 0);
	SetTriggerR(device, data[1] ? 255 : 0);

	SetBtnLB(device, data[2]);
	SetBtnRB(device, data[3]);

	SetBtnBack(device, data[4]);
	SetBtnStart(device, data[5]);

	INT flags = 0;
	if (data[6]) flags |= XINPUT_GAMEPAD_DPAD_UP;
	if (data[7]) flags |= XINPUT_GAMEPAD_DPAD_LEFT;
	if (data[8]) flags |= XINPUT_GAMEPAD_DPAD_RIGHT;
	if (data[9]) flags |= XINPUT_GAMEPAD_DPAD_DOWN;
	SetDpad(device, flags);

	SetBtnY(device, data[10]);
	SetBtnB(device, data[11]);
	SetBtnX(device, data[12]);
	SetBtnA(device, data[13]);

	SetAxisX(device, (SHORT) (data[14] / 127.0 * 32767));
	SetAxisY(device, (SHORT) (data[15] / 127.0 * 32767));

	SetAxisRx(device, (SHORT) (data[16] / 127.0 * 32767));
	SetAxisRy(device, (SHORT) (data[17] / 127.0 * 32767));
}

