
#include "public.h"
//#include "declarations.h"
typedef struct kbdData {
	char key;
	HANDLE file;
	BOOL stop;
	int duration;
} keydat;
void SendKey(char key, HANDLE file){
	DeviceIoControl(file,
		IOCTL_KBFILTR_SEND_INPUT,
		&key, 1,
		NULL, 0,
		NULL, NULL);
}

void SpamKey(char key, HANDLE file, LONGLONG Duration){
	LARGE_INTEGER tcounter_outer;
	LONGLONG ticknow_outer, ticknew_outer, freq_outer;

	QueryPerformanceFrequency(&tcounter_outer);
	freq_outer = tcounter_outer.QuadPart;

	QueryPerformanceCounter(&tcounter_outer);
	ticknow_outer = tcounter_outer.QuadPart;
	ticknew_outer = tcounter_outer.QuadPart;

	LARGE_INTEGER tcounter_inner;
	LONGLONG ticknow_inner, ticknew_inner, freq_inner;

	QueryPerformanceFrequency(&tcounter_inner);
	freq_inner = tcounter_inner.QuadPart;

	QueryPerformanceCounter(&tcounter_inner);
	ticknow_inner = tcounter_inner.QuadPart;
	ticknew_inner = tcounter_inner.QuadPart;

	while (((ticknew_outer - ticknow_outer) * (1000000)) / freq_outer <= (Duration * 1000000)){


		while (((ticknew_inner - ticknow_inner) * (1000000)) / freq_inner <= (1 * 100000)){
			DeviceIoControl(file,
				IOCTL_KBFILTR_HOLD_INPUT,
				&key, 1,
				NULL, 0,
				NULL, NULL);
			Sleep(100);

			QueryPerformanceCounter(&tcounter_inner);
			ticknew_inner = tcounter_inner.QuadPart;
		}
		Sleep(50);

		DeviceIoControl(file,
			IOCTL_KBFILTR_RELEASE_INPUT,
			&key, 1,
			NULL, 0,
			NULL, NULL);

		Sleep(100);
		QueryPerformanceCounter(&tcounter_outer);
		QueryPerformanceCounter(&tcounter_inner);
		ticknow_inner = tcounter_inner.QuadPart;
		ticknew_outer = tcounter_outer.QuadPart;

	}
}

void SendSpecial(char key, HANDLE file){
	DeviceIoControl(file,
		IOCTL_KBFILTR_SEND_SPECIAL,
		&key, 1,
		NULL, 0,
		NULL, NULL);
}

/*DWORD WINAPI Hold(void *key) {
keydat *lol = (keydat*)(key);
LARGE_INTEGER tcounter;
LONGLONG ticknow, ticknew, freq;

QueryPerformanceFrequency(&tcounter);
freq = tcounter.QuadPart;

QueryPerformanceCounter(&tcounter);
ticknow = tcounter.QuadPart;
ticknew = tcounter.QuadPart;

while ((ticknew - ticknow) / (freq / 1000000) <= lol->duration * 1000) {
printf("%d\n :: %s", ticknew - ticknow, lol->key);
DownKey(lol->key, lol->file);
Sleep(100);

QueryPerformanceCounter(&tcounter);
ticknew = tcounter.QuadPart;
}

ReleaseKey(lol->key, lol->file);
return 0;
}

HANDLE HoldKey(keydat *Data, char key, int duration, HANDLE file){
Data->file = file;
Data->key = key;
Data->stop = FALSE;
Data->duration = duration;

return CreateThread(NULL, 0, Hold, (LPVOID *)Data, 0, NULL);
}*/

void DownKey(char key, HANDLE file) {

	DeviceIoControl(file,
		IOCTL_KBFILTR_HOLD_INPUT,
		&key, 1,
		NULL, 0,
		NULL, NULL);
}

void DownSpecial(char key, HANDLE file) {
	DeviceIoControl(file,
		IOCTL_KBFILTR_HOLD_SPECIAL,
		&key, 1,
		NULL, 0,
		NULL, NULL);
}

void ReleaseKey(char key, HANDLE file){
	DeviceIoControl(file,
		IOCTL_KBFILTR_RELEASE_INPUT,
		&key, 1,
		NULL, 0,
		NULL, NULL);
}

void ReleaseSpecial(char key, HANDLE file) {
	DeviceIoControl(file,
		IOCTL_KBFILTR_RELEASE_SPECIAL,
		&key, 1,
		NULL, 0,
		NULL, NULL);
}

void SendString(CHAR *string, HANDLE file){

	printf("size of string is %d\n", strlen(string));
	DeviceIoControl(file,
		IOCTL_KBFILTR_SEND_INPUT,
		string, (DWORD)strlen(string),
		NULL, 0,
		NULL, NULL);
}

void TravelTo(float x, float y){
	cout << "travelling to " << x << " " << y << endl;
	float prevYaw = *yaw, preX = *playerX, preY = *playerY;
	int counter = 0;
	*yaw = 0;
	Sleep(100);
	if (*playerX > x){
		//DownKey('a', file_kbd);
		injectDInput(DIK_A, -1);
		while (*playerX - x > 1){
			preX == *playerX;
			if (characterSelect == true) break;
			if (*playerX == preX) counter++;
			else counter = 0;
			if (counter == 50){
				counter = 0;
				releaseInjectedKey = true;
				while (releaseInjectedKey == true) Sleep(10);
				Sleep(300);
				injectDInput(DIK_A, -1);
			}
			Sleep(10);
		}
		//ReleaseKey('a', file_kbd);
		releaseInjectedKey = true;
		while (releaseInjectedKey == true) Sleep(10);
		Sleep(100);
		counter = 0;
	}

	else if (*playerX < x){
		//DownKey('d', file_kbd);
		injectDInput(DIK_D, -1);
		while (*playerX - x < -1){
			preX == *playerX;
			if (characterSelect == true) break;
			if (*playerX == preX) counter++;
			else counter = 0;
			if (counter == 50){
				counter = 0;
				releaseInjectedKey = true;
				while (releaseInjectedKey == true) Sleep(10);
				Sleep(300);
				injectDInput(DIK_D, -1);
			}
			Sleep(10);
		}
		//ReleaseKey('d', file_kbd);
		releaseInjectedKey = true;
		while (releaseInjectedKey == true) Sleep(10);
		Sleep(100);
		counter = 0;
	}

	if (*playerY > y){
		//DownKey('s', file_kbd);
		injectDInput(DIK_S, -1);
		while (*playerY - y > 1){
			preY == *playerY;
			if (characterSelect == true) break;
			if (*playerY == preY) counter++;
			else counter = 0;
			if (counter == 50){
				counter = 0;
				releaseInjectedKey = true;
				while (releaseInjectedKey == true) Sleep(10);
				Sleep(300);
				injectDInput(DIK_S, -1);
			}
			Sleep(10);
		}
		//ReleaseKey('s', file_kbd);
		releaseInjectedKey = true;
		while (releaseInjectedKey == true) Sleep(10);
		Sleep(100);
		counter = 0;
	}

	else if (*playerY < y){
		//DownKey('w', file_kbd);
		injectDInput(DIK_W, -1);
		while (*playerY - y < -1){
			preY == *playerY;
			if (characterSelect == true) break;
			if (*playerY == preY) counter++;
			else counter = 0;
			if (counter == 50){
				counter = 0;
				releaseInjectedKey = true;
				while (releaseInjectedKey == true) Sleep(10);
				Sleep(300);
				injectDInput(DIK_W, -1);
			}
			Sleep(10);
		}
		//ReleaseKey('w', file_kbd);
		releaseInjectedKey = true;
		while (releaseInjectedKey == true) Sleep(10);
		Sleep(100);
		counter = 0;
	}

	*yaw = prevYaw;
	Sleep(100);
}