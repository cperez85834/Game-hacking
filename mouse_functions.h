#include "public_mouse1.h"

void LeftClick(HANDLE file);
void RightClick(HANDLE file);
void MoveMouseAbsolute(short x, short y, HANDLE file);
void MoveMouseRelative(short x, short y, HANDLE file);
void MoveMouseRelativeX(short x, HANDLE file);
void MoveMouseRelativeY(short y, HANDLE file);
void HoldMouseLeft(HANDLE file);
void HoldMouseRight(HANDLE file);
void ReleaseMouseLeft(HANDLE file);
void ReleaseMouseRight(HANDLE file);

void LeftClick(HANDLE file){
	char key = 'a';
	DeviceIoControl(file,
		IOCTL_MOUFILTR_LEFT_CLICK,
		&key, 1,
		NULL, 0,
		NULL, NULL);
}

void ScrollUp(HANDLE file){
	char key = 'a';
	DeviceIoControl(file,
		IOCTL_MOUFILTR_SCROLLUP,
		&key, 1,
		NULL, 0,
		NULL, NULL);
}

void ScrollDown(HANDLE file){
	char key = 'a';
	DeviceIoControl(file,
		IOCTL_MOUFILTR_SCROLLDOWN,
		&key, 1,
		NULL, 0,
		NULL, NULL);
}

void RightClick(HANDLE file){
	char key = 'a';
	DeviceIoControl(file,
		IOCTL_MOUFILTR_RIGHT_CLICK,
		&key, 1,
		NULL, 0,
		NULL, NULL);
}

void MoveMouseAbsolute(short x, short y, HANDLE file){
	int Information[4] = { x, y, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
	DeviceIoControl(file,
		IOCTL_MOUFILTR_MOVE_ABSOLUTE,
		&Information, sizeof(Information),
		NULL, 0,
		NULL, NULL);
}

void MoveMouseRelative(short x, short y, HANDLE file){
	int Information[4] = { x, y, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
	DeviceIoControl(file,
		IOCTL_MOUFILTR_MOVE_RELATIVE,
		&Information, sizeof(Information),
		NULL, 0,
		NULL, NULL);
}

void MoveMouseRelativeX(short x, HANDLE file){
	int Information[2] = { x, GetSystemMetrics(SM_CXSCREEN) };
	DeviceIoControl(file,
		IOCTL_MOUFILTR_MOVE_X_RELATIVE,
		&Information, sizeof(Information),
		NULL, 0,
		NULL, NULL);
}

void MoveMouseRelativeY(short y, HANDLE file){
	int Information[2] = { y, GetSystemMetrics(SM_CYSCREEN) };
	printf("location: %d", sizeof(Information));
	DeviceIoControl(file,
		IOCTL_MOUFILTR_MOVE_Y_RELATIVE,
		&Information, sizeof(Information),
		NULL, 0,
		NULL, NULL);
}

void HoldMouseLeft(HANDLE file){
	char key = 'a';
	DeviceIoControl(file,
		IOCTL_MOUFILTR_HOLD_LEFT,
		&key, 1,
		NULL, 0,
		NULL, NULL);
}

void HoldMouseRight(HANDLE file){
	char key = 'a';
	DeviceIoControl(file,
		IOCTL_MOUFILTR_HOLD_RIGHT,
		&key, 1,
		NULL, 0,
		NULL, NULL);
}

void ReleaseMouseLeft(HANDLE file){
	char key = 'a';
	DeviceIoControl(file,
		IOCTL_MOUFILTR_RELEASE_LEFT,
		&key, 1,
		NULL, 0,
		NULL, NULL);
}

void ReleaseMouseRight(HANDLE file){
	char key = 'a';
	DeviceIoControl(file,
		IOCTL_MOUFILTR_RELEASE_RIGHT,
		&key, 1,
		NULL, 0,
		NULL, NULL);
}