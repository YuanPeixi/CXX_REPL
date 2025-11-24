
#include <windows.h>
#include "stdcpp.h"
using namespace std;

extern "C" __declspec(dllexport) void HelloWorld();
extern "C" __declspec(dllexport) int AddNumbers(int a, int b);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: break;
        case DLL_THREAD_ATTACH: break;
        case DLL_THREAD_DETACH: break;
        case DLL_PROCESS_DETACH: break;
    }
    return TRUE;
}

#define DLLEXP extern "C" __declspec(dllexport)

void HelloWorld() {
    MessageBox(NULL, TEXT("Hello from DLL!"), TEXT("DLL Message"), MB_OK);
}void f(){
cout<<"Hello World"<<endl;
}

DLLEXP void Func1() {
f();

}
