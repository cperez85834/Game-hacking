#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <iostream>

bool injectKey = false;
bool releaseInjectedKey = false;
double injectKeyDuration = 0;
int KeyToInject = 0;
LARGE_INTEGER tcounter;
LONGLONG tickInject, tickElapsed;
LONGLONG freq;

typedef HRESULT(WINAPI* IDirectInputDevice_GetDeviceData_t)(IDirectInputDevice8*, DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);
typedef HRESULT(WINAPI* IDirectInputDevice_GetDeviceState_t)(IDirectInputDevice8*, DWORD, LPVOID);
	
IDirectInputDevice_GetDeviceData_t fnGetDeviceData = NULL;
IDirectInputDevice_GetDeviceState_t fnGetDeviceState = NULL;
IDirectInputDevice8;
bool dinputClickVar = false;
bool downVar = false;
bool normalInput = true;

HRESULT WINAPI HookGetDeviceData(IDirectInputDevice8* pThis, DWORD a, LPDIDEVICEOBJECTDATA b, LPDWORD c, DWORD d)
{
	LPDIDEVICEOBJECTDATA uh = b;
	if (dinputClickVar == true){
		if (downVar == false){
			uh[0].dwData = 0x80;
			uh[0].dwOfs = 0x0c;
			downVar = true;
		}

		else{
			uh[0].dwData = 0x00;
			uh[0].dwOfs = 0x0c;
			dinputClickVar = false;
			downVar = false;
		}

		return DI_OK;
	}

	HRESULT ret = fnGetDeviceData(pThis, a, b, c, d);
	//std::cout << "yuh: " << std::hex << ret << std::endl;
	LPDIDEVICEOBJECTDATA lol;
	if (ret == DI_OK)
	{
		//std::cout << "yeet\n";
		for (DWORD i = 0; i < *c; i++)
		{
			//std::cout << "i: " << i << std::endl;
			//std::cout << "c: " << *c << std::endl;
			//std::cout << std::hex << "dwData: " << b[i].dwData << std::endl;
			//std::cout << std::hex << "dwOfs: " << b[i].dwOfs << std::endl << std::endl;
			//if (LOBYTE(b[i].dwData) > 0) // idk what this does
			if (1)
			{
				//std::cout << b[i].dwOfs << std::endl;
				switch (b[i].dwOfs)
				{
				case 12:
					/*std::cout << "**Left Click**";
					std::cout << "Input type: ";
					if (b[i].dwData == 0x80) std::cout << "Down\n";
					else std::cout << "Up\n";
					std::cout << "i: " << i << std::endl;
					std::cout << std::hex << "dwData: " << b[i].dwData << std::endl;
					std::cout << std::hex << "dwOfs: " << b[i].dwOfs << std::endl;
					std::cout << "**Left Click**\n\n";*/
					break;
				case DIMOFS_X:
					//b[i].dwOfs = DIMOFS_Y;
					//std::cout << "X?";
					break;
				case DIMOFS_Y:
					//std::cout << "Y?";
					break;
				/*case DIK_D:
					std::cout << "[D]";
					break;*/
				}
			}
		}
	}
	return ret;
}

void injectDInput(int key, double duration){

	KeyToInject = key;
	injectKeyDuration = duration * 1000 ;
	QueryPerformanceCounter(&tcounter);
	tickInject = tcounter.QuadPart;

	injectKey = true;
}
HRESULT WINAPI HookGetDeviceState(IDirectInputDevice8* pThis, DWORD a, LPVOID b)
{
	//return DI_OK;
	BYTE *diKeys = (BYTE*)b;
	if (injectKey == true){
		normalInput = false;
		//int duration = 1000 * injectKeyDuration;
		QueryPerformanceFrequency(&tcounter);
		freq = tcounter.QuadPart;

		QueryPerformanceCounter(&tcounter);
		tickElapsed = tcounter.QuadPart;
		*(diKeys + KeyToInject) = 0x80;
		//std::cout << std::hex << &diKeys << std::endl;
		if (injectKeyDuration >= 0){
			if ((tickElapsed - tickInject) / (freq / 1000) > injectKeyDuration){
				*(diKeys + KeyToInject) = 0x00;
				KeyToInject = 0;
				injectKeyDuration = 0;
				injectKey = false;
			}
		}

		if (injectKeyDuration < 0){
			if (releaseInjectedKey == true){
				*(diKeys + KeyToInject) = 0x00;
				KeyToInject = 0;
				injectKeyDuration = 0;
				injectKey = false;
				releaseInjectedKey = false;
			}
		}

		return DI_OK;
	}

	HRESULT ret = fnGetDeviceState(pThis, a, b);
	//std::cout << "yuh: " << std::hex << ret << std::endl;
	bool lrRev = false;
	bool udRev = false;


	if (ret == DI_OK)
	{
		//BYTE *diKeys = (BYTE*)b;

		/*if (*(diKeys + DIK_W) & 0x80){
			std::cout << "Value is: " << std::hex << *(diKeys + DIK_W) << std::endl;
		}
		if (*(diKeys + DIK_A) & 0x80){
			*((BYTE*)(b)+DIK_A) = 0;
			*((BYTE*)(b)+DIK_D) = 0x80;
			lrRev = true;
		}
		if (*(diKeys + DIK_S) & 0x80 && udRev == false){
			*((BYTE*)(b)+DIK_S) = 0;
			*((BYTE*)(b)+DIK_W) = 0x80;
		}
		if (*(diKeys + DIK_D) & 0x80 && lrRev == false){
			*((BYTE*)(b)+DIK_D) = 0;
			*((BYTE*)(b)+DIK_A) = 0x80;
		}*/
	}
	return ret;
}
// By Timb3r
// Source: https://gamephreakers.com/2018/08/introduction-to-vtable-hooking/
void* HookVTableFunction(void* pVTable, void* fnHookFunc, int nOffset)
{
	intptr_t ptrVtable = *((intptr_t*)pVTable); // Pointer to our chosen vtable
	intptr_t ptrFunction = ptrVtable + sizeof(intptr_t)* nOffset; // The offset to the function (remember it's a zero indexed array with a size of four bytes)
	intptr_t ptrOriginal = *((intptr_t*)ptrFunction); // Save original address

	// Edit the memory protection so we can modify it
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery((LPCVOID)ptrFunction, &mbi, sizeof(mbi));
	VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &mbi.Protect);

	// Overwrite the old function with our new one
	*((intptr_t*)ptrFunction) = (intptr_t)fnHookFunc;

	// Restore the protection
	VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &mbi.Protect);

	// Return the original function address incase we want to call it
	//std::cout << std::hex << "pointer is: " << ptrOriginal << std::endl;
	return (void*)ptrOriginal;
}

int dinput_main(){
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	IDirectInput8 *pDirectInputKbd = NULL;
	IDirectInput8 *pDirectInputMou = NULL;

	if (DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8A, (LPVOID*)&pDirectInputKbd, NULL) != DI_OK){
		std::cout << "DirectInput8Create kbd failed!\n";
		return -1;
	}
	if (DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8A, (LPVOID*)&pDirectInputMou, NULL) != DI_OK){
		std::cout << "DirectInput8Create mou failed!\n";
		return -1;
	}
	std::cout << "DirectInput8Create success!\n";

	LPDIRECTINPUTDEVICE8 lpdiKeyboard, lpdiMouse;
	if (pDirectInputKbd->CreateDevice(GUID_SysKeyboard, &lpdiKeyboard, NULL) != DI_OK){
		std::cout << "CreateDevice kbd failed!\n";
		pDirectInputKbd->Release();
		return -1;
	}
	if (pDirectInputMou->CreateDevice(GUID_SysMouse, &lpdiMouse, NULL) != DI_OK){
		std::cout << "CreateDevice mou failed!\n";
		pDirectInputMou->Release();
		return -1;
	}

	std::cout << "CreateDevices success!\n";

	lpdiKeyboard->SetDataFormat(&c_dfDIKeyboard);
	lpdiKeyboard->SetCooperativeLevel(GetActiveWindow(), DISCL_NONEXCLUSIVE);
	lpdiMouse->SetDataFormat(&c_dfDIMouse);
	lpdiMouse->SetCooperativeLevel(GetActiveWindow(), DISCL_NONEXCLUSIVE);

	//DIPROPWORD
	fnGetDeviceData = (IDirectInputDevice_GetDeviceData_t)HookVTableFunction(lpdiMouse, HookGetDeviceData, 10);
	fnGetDeviceState = (IDirectInputDevice_GetDeviceState_t)HookVTableFunction(lpdiKeyboard, HookGetDeviceState, 9);

	return 0;
}

