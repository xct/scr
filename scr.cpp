#include <iostream>
#include <windows.h>
#include <atlimage.h>
#include <wtsapi32.h>
#include <tlhelp32.h>
#include "winternl.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "wtsapi32.lib")

#define FILENAME L"scr.jpg"

typedef NTSTATUS(*MYPROC) (HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

void Error(const char* name) {
    printf("%s Error: %d\n", name, GetLastError());
    exit(-1);
}

void ErrorContinue(const char* name) {
    printf("%s Error: %d\n", name, GetLastError());
    return;
}

int main(int argc, char** argv) {
    HWINSTA hWindowStation = OpenWindowStationA("WinSta0", FALSE, WINSTA_ALL_ACCESS);
    if (!hWindowStation) {
        Error("OpenWindowStationA");
    }

    if (!SetProcessWindowStation(hWindowStation)) {
        Error("SetProcessWindowStation");
    }

    HDESK hInputDesktop = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
    if (!hInputDesktop) {
        Error("OpenInputDesktop\n");
    }

    if (!SetThreadDesktop(hInputDesktop)) {
        Error("SetThreadDesktop\n");
    }

    HWND hDesktopWnd = GetDesktopWindow();
    if (!hDesktopWnd) {
        Error("GetDesktopWindow\n");
    }

    HDC hDC = GetDC(hDesktopWnd);
    if (!hDC) {
        Error("GetDC\n");
    }

    HDC hMemDC = CreateCompatibleDC(hDC);
    if (!hMemDC) {
        Error("CreateCompatibleDC\n");
    }

    int sizeX = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int sizeY = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    int posX  = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int posY  = GetSystemMetrics(SM_YVIRTUALSCREEN);

    HBITMAP hBMP = CreateCompatibleBitmap(hDC, sizeX, sizeY);
    if (!SelectObject(hMemDC, hBMP)) {
        Error("SelectObject\n");
    }

    if (!SetProcessDPIAware()) {
        Error("SetProcessDPIAware\n");
    };

    if (!StretchBlt(hMemDC, 0, 0, sizeX, sizeY, hDC, posX, posY, sizeX, sizeY, SRCCOPY)) {
        Error("StretchBlt\n");
    }

    CImage image;
    image.Attach(hBMP);
    image.Save(FILENAME);

    // cleanup
    ReleaseDC(hDesktopWnd, hDC);
    DeleteDC(hMemDC);
    DeleteObject(hBMP);
    CloseWindowStation(hWindowStation);
    CloseDesktop(hInputDesktop);
    return 0;
}
