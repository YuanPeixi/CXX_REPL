
#ifndef CROSS_PLATFORM_COMMAND_H
#define CROSS_PLATFORM_COMMAND_H

#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

/**
 * @brief 跨平台命令行执行函数
 * @param command 要执行的命令
 * @return 命令执行结果的字符串输出
 */
std::string executeCommand(const std::string& command) {
#ifdef _WIN32
    // Windows平台实现
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    HANDLE hStdoutRd, hStdoutWr;
    CreatePipe(&hStdoutRd, &hStdoutWr, &sa, 0);
    SetHandleInformation(hStdoutRd, HANDLE_FLAG_INHERIT, 0);

    PROCESS_INFORMATION pi;
    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(STARTUPINFOA));
    si.cb = sizeof(STARTUPINFOA);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hStdoutWr;
    si.hStdError = hStdoutWr;

    std::string result;
    if (CreateProcessA(NULL, (LPSTR)command.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(hStdoutWr);

        char buffer[4096];
        DWORD bytesRead;
        while (ReadFile(hStdoutRd, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead) {
            buffer[bytesRead] = '\0';
            result += buffer;
}

        CloseHandle(hStdoutRd);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    return result;
#else
    // Linux/Unix平台实现
    std::string result;
    char buffer[4096];
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return "";

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);
    return result;
#endif
}


// 平台检测宏
#ifdef _WIN32
#define PLATFORM_WINDOWS 1
#define PLATFORM_LINUX 0
#else
#define PLATFORM_WINDOWS 0
#define PLATFORM_LINUX 1
#endif

#endif // CROSS_PLATFORM_COMMAND_H
