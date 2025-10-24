#include <fstream>
#include <iostream>
#include "execCmd.h"

using namespace std;

/*
 * This program is a C++ REPL to dynamically compile and execute code snippets.
 * It supports global and local modes, and allows for code testing and execution.
 */

 // Function prototypes
void greet();
void help();
string getCompilerPath(const string& configFilePath);

// Function pointers for dynamic DLL loading
typedef void (*fn_t)();
typedef int (*fn2_t)();

// Global constants
const string DEFAULT_PAYLOAD = R"(
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
})";

int main() {
    greet();

    // Open and write the default payload
    ofstream payloadFile("PayLoad.cpp");
    payloadFile << DEFAULT_PAYLOAD;
    payloadFile.close();

    string compilerPath = getCompilerPath("config.txt");

    // REPL loop
    string codeBuffer;
    size_t functionCount = 0;
    bool globalMode = false;

    cout << ">>>";
    while (true) {
        string cmd;
        char inputBuffer[1025];
        cin.getline(inputBuffer, 1024);
        cmd = string(inputBuffer);

        if (cmd.empty()) {
            help();
        }
        else if (cmd == "::EXIT") {
            break;
        }
        else if (cmd == "GLOBAL OFF") {
            globalMode = false;
            cout << "GLOBAL MODE OFF" << endl;
        }
        else if (cmd == "GLOBAL ON" || cmd == "GLOBAL") {
            globalMode = true;
            cout << "GLOBAL MODE ON" << endl;
            //TODO:Check here added but not compile and debug
            cout << "Code buffer: {\n" << codeBuffer << "\n}" << endl;
            cout << "Commit Current Change to Global? (Y/N)";
            for(int i=0;i<3;i++){
                char c;
                cin>>c;
                if(c=='Y'){
                    ofstream file2;
                    file2.open("Payload.cpp",ios::app);
                    file2<<codeBuffer<<endl;
                    file2.close();
                    codeBuffer="";
                }else if(c=='N'){
                    cout<< "Code buffer has been cleared" <<endl;
                    codeBuffer="";
                }else{
                    cout << "Please input (Y/N) (After "<<3-i<<" time, program will auto dispose code buffer"<< endl;
                }
            }
            if(codeBuffer.size())codeBuffer="";
        }
        else if (cmd == "::EXEC") {
            // Execute code in buffer
            ofstream file("PayLoad.cpp", ios::app);
            if (!file.is_open()) {
                cerr << "Cannot open Payload.cpp" << endl;
                cout << ">>>";
                continue;
            }

            if (!globalMode) {
                file << "\nDLLEXP void Func" << functionCount << "() {\n" << codeBuffer << "\n}" << endl;
                functionCount++;
            }

            file.close();
            string compileInfo = executeCommand("\"" + compilerPath + "clang++.exe\" -fansi-escape-codes -fdiagnostics-color=always -shared PayLoad.cpp -o PayLoad.dll -Wl,--out-implib,PayLoad.lib");

            if (!compileInfo.empty()) {
                cout << "CompileInfo={\n" << compileInfo << "\n}" << endl;
            }

            if (compileInfo.find("error generated") != string::npos|| compileInfo.find("errors generated") != string::npos) {
                ofstream resetFile("PayLoad.cpp");
                resetFile << DEFAULT_PAYLOAD;
                resetFile.close();
                codeBuffer.clear();
                cout << "Changes discarded due to compilation errors." << endl;
                cout << ">>>";
                continue;
            }

            HMODULE hMod = LoadLibrary(TEXT("PayLoad.dll"));
            if (!hMod) {
                cerr << "Failed to load DLL" << endl;
                cout << ">>>";
                continue;
            }

            if (!globalMode) {
                string funcName = "Func" + to_string(functionCount - 1);
                fn_t func = (fn_t)GetProcAddress(hMod, funcName.c_str());
                if (!func) {
                    cerr << "Failed to find function" << endl;
                }
                else {
                    func();
                }
            }
            else {
                fn2_t mainFunc = (fn2_t)GetProcAddress(hMod, "main");
                if (!mainFunc) {
                    cerr << "Failed to find main() entry point" << endl;
                }
                else {
                    int result = mainFunc();
                    cout << "Return value: " << result << endl;
                }
            }

            FreeLibrary(hMod);
            codeBuffer.clear();
        }
        else if (cmd == "::TEST") {
            // Run test function
            auto compileInfo = executeCommand("\"" + compilerPath + "clang++.exe\" -shared PayLoad.cpp -o PayLoad.dll -Wl,--out-implib,PayLoad.lib");
            if (!compileInfo.empty()) {
                cout << "CompileInfo={\n" << compileInfo << "\n}" << endl;
            }

            HMODULE hMod = LoadLibrary(TEXT("PayLoad.dll"));
            if (!hMod) {
                cerr << "Failed to load DLL" << endl;
                cerr << "Error code: " << GetLastError() << endl;
                cout << ">>>";
                continue;
            }

            fn_t helloWorldFunc = (fn_t)GetProcAddress(hMod, "HelloWorld");
            if (!helloWorldFunc) {
                cerr << "Failed to find HelloWorld function" << endl;
                cerr << "Error code: " << GetLastError() << endl;
            }
            else {
                helloWorldFunc();
            }

            FreeLibrary(hMod);
        }
        else if (cmd == "::CODE CLEAR") {
            // Clear code buffer
            ofstream resetFile("PayLoad.cpp");
            resetFile << DEFAULT_PAYLOAD;
            resetFile.close();
            codeBuffer.clear();
            cout << "Code buffer cleared." << endl;
        }
        else if (cmd == "::CODE") {
            // Display code buffer
            cout << "Code buffer: {\n" << codeBuffer << "\n}" << endl;
        }
        else if (cmd[0] == ':' && cmd[1] == ':') {
            cout << ">>>";
            continue;
        }
        else if(globalMode){
            //TODO:Check here, added but not compile and debug
            //Global mode will directly write into file
            //So that it won't be wrap by function head
            ofstream file2;
            file2.open("Payload2.cpp",ios::app);
            file2<<cmd<<endl;
            file2.close();
        }
        else {
            // Add to code buffer
            codeBuffer += cmd + "\n";
        }

        cout << ">>>";
    }

    return 0;
}

// Function definitions
void greet() {
    cout << "C++ REPL\n-------------------------\n";
}

void help() {
    cout << "::EXEC (Execute code)\n"
        << "::CODE (View code buffer)\n"
        << "::CODE CLEAR (Clear code buffer)\n"
        << "::TEST (Run test function)\n"
        << "::GLOBAL ON (Enable global mode)\n"
        << "::GLOBAL OFF (Disable global mode)\n";
}

string getCompilerPath(const string& configFilePath) {
    ifstream config(configFilePath);
    string path;

    if (!config.good()) {
        cout << "Please input compiler path: ";
        getline(cin, path);
        if (path.back() != '\\') path += '\\';

        ofstream configOut(configFilePath);
        configOut << path;
        configOut.close();
    }
    else {
        getline(config, path);
    }

    config.close();
    return path;

}
