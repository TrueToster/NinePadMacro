#include "hidapi.h"
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>

#define _CRT_SECURE_NO_WARNINGS

// Идентификаторы для меню
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
#define ID_STYLE_BLACK 1002
#define ID_STYLE_TRANS 1003

NOTIFYICONDATA nid = {};
HWND hwnd = NULL;
bool g_Running = true;
bool g_useTransparent = false;
int g_currentLayer = 0;      

void SaveSettings() {
    HKEY hKey;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\NinePad", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        DWORD val = g_useTransparent ? 1 : 0;
        RegSetValueExA(hKey, "TransparentStyle", 0, REG_DWORD, (BYTE*)&val, sizeof(val));
        RegCloseKey(hKey);
    }
}

void LoadSettings() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\NinePad", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD val = 0;
        DWORD size = sizeof(val);
        if (RegQueryValueExA(hKey, "TransparentStyle", NULL, NULL, (BYTE*)&val, &size) == ERROR_SUCCESS) {
            g_useTransparent = (val == 1);
        }
        RegCloseKey(hKey);
    }
}

// black background
HICON CreateNumberIconBlack(int number) {
    HDC hdc = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hbm = CreateCompatibleBitmap(hdc, 16, 16);
    SelectObject(hdcMem, hbm);
    RECT rect = {0, 0, 16, 16};
    HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdcMem, &rect, hBrush);
    DeleteObject(hBrush);
    SetBkMode(hdcMem, TRANSPARENT);
    SetTextColor(hdcMem, RGB(255, 255, 255));
    char text[2] = {(char)('0' + number), 0};
    DrawTextA(hdcMem, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    ICONINFO ii = {0};
    ii.fIcon = TRUE;
    ii.hbmColor = hbm;
    ii.hbmMask = hbm;
    HICON hIcon = CreateIconIndirect(&ii);
    DeleteDC(hdcMem);
    DeleteObject(hbm);
    ReleaseDC(NULL, hdc);
    return hIcon;
}

// transparent background
HICON CreateNumberIconTrans(int number) {
    HDC hdc = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdc);

    BITMAPV5HEADER bi = {0};
    bi.bV5Size = sizeof(BITMAPV5HEADER);
    bi.bV5Width = 16;
    bi.bV5Height = -16;  
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_RGB;
    bi.bV5AlphaMask = 0xFF000000; 
    
    void* bits = NULL;
    HBITMAP hbm = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &bits, NULL, 0);
    if (!hbm) { ReleaseDC(NULL, hdc); return NULL; }
    
    SelectObject(hdcMem, hbm);
    DWORD* pixel = (DWORD*)bits;
    for (int i = 0; i < 16 * 16; i++) pixel[i] = 0x00000000; 
    
    SetBkMode(hdcMem, TRANSPARENT);
    SetTextColor(hdcMem, RGB(0, 0, 0)); 
    char text[2] = {(char)('0' + number), 0};
    
    HDC hdcTemp = CreateCompatibleDC(hdc);
    HBITMAP hbmTemp = CreateCompatibleBitmap(hdc, 16, 16);
    SelectObject(hdcTemp, hbmTemp);
    
    RECT rect = {0, 0, 16, 16};
    HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdcTemp, &rect, hBrush);
    DeleteObject(hBrush);
    
    SetBkMode(hdcTemp, TRANSPARENT);
    SetTextColor(hdcTemp, RGB(0, 0, 0));
    DrawTextA(hdcTemp, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            COLORREF color = GetPixel(hdcTemp, x, y);
            if (color != RGB(255, 255, 255)) {  
                pixel[y * 16 + x] = (0xFF << 24) | 0x000000; 
            }
        }
    }
    
    DeleteDC(hdcTemp);
    DeleteObject(hbmTemp);
    ICONINFO ii = {0};
    ii.fIcon = TRUE;
    ii.hbmColor = hbm;
    HBITMAP hbmMask = CreateBitmap(16, 16, 1, 1, NULL);
    ii.hbmMask = hbmMask;
    HICON hIcon = CreateIconIndirect(&ii);
    DeleteDC(hdcMem);
    DeleteObject(hbm);
    DeleteObject(hbmMask);
    ReleaseDC(NULL, hdc);
    return hIcon;
}

HICON CreateNumberIcon(int number) {
    return g_useTransparent ? CreateNumberIconTrans(number) : CreateNumberIconBlack(number);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_TRAYICON) {
        if (lParam == WM_RBUTTONUP) {
            POINT cp; GetCursorPos(&cp);
            HMENU hMenu = CreatePopupMenu();
            
            AppendMenuW(hMenu, MF_STRING | (g_useTransparent ? MF_UNCHECKED : MF_CHECKED), ID_STYLE_BLACK, L"Темная тема");
            AppendMenuW(hMenu, MF_STRING | (g_useTransparent ? MF_CHECKED : MF_UNCHECKED), ID_STYLE_TRANS, L"Прозрачная тема");
            AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Выход");
            
            SetForegroundWindow(hWnd);
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, cp.x, cp.y, 0, hWnd, NULL);
            DestroyMenu(hMenu);
        }
    }
    if (message == WM_COMMAND) {
        int id = LOWORD(wParam);
        if (id == ID_TRAY_EXIT) g_Running = false;
        if (id == ID_STYLE_BLACK || id == ID_STYLE_TRANS) {
            g_useTransparent = (id == ID_STYLE_TRANS);
            SaveSettings();
            HICON hNew = CreateNumberIcon(g_currentLayer);
            if (hNew) {
                nid.hIcon = hNew;
                Shell_NotifyIcon(NIM_MODIFY, &nid);
                DestroyIcon(hNew);
            }
        }
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void AddToStartup() {
    HKEY hKey;
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "NinePad", 0, REG_SZ, (BYTE*)exePath, strlen(exePath) + 1);
        RegCloseKey(hKey);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    LoadSettings(); 
    AddToStartup();

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "NinePadClass";
    RegisterClassA(&wc);

    hwnd = CreateWindowA("NinePadClass", "LayerTray", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = CreateNumberIcon(0);
    strcpy(nid.szTip, "NinePad");

    Shell_NotifyIcon(NIM_ADD, &nid);
    
    if (hid_init()) {
        Shell_NotifyIcon(NIM_DELETE, &nid);
        return 1;
    }
    
    while(g_Running) {
        hid_device* handle = hid_open(0xfeed, 0x0000, NULL);
        if (handle) {
            unsigned char buf[32];
            if (hid_read_timeout(handle, buf, sizeof(buf), 100) > 0) {
                int layer = buf[1];
                if (layer >= 0 && layer <= 9) {
                    g_currentLayer = layer;
                    HICON hNewIcon = CreateNumberIcon(layer);
                    if (hNewIcon) {
                        nid.hIcon = hNewIcon;
                        sprintf(nid.szTip, "NinePad Layer %d", layer);
                        Shell_NotifyIcon(NIM_MODIFY, &nid);
                        DestroyIcon(hNewIcon);
                    }
                }
            }
            hid_close(handle);
        }
        
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(10);
    }

    hid_exit();
    Shell_NotifyIcon(NIM_DELETE, &nid);
    return 0;
}
