#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <string>
#include <iostream>

BOOL WINAPI TakeDesktopScreenshot(WCHAR* wPath);

int main(int argc, wchar_t* argv[]) {

    wchar_t path[1024];
    std::wcsncpy(path, L".\\screenshot.bmp", 1024);
    TakeDesktopScreenshot(path);
    std::cout << "[+] Saved to .\\screenshot.bmp" << std::endl;

    return 0;
}

BOOL WINAPI TakeDesktopScreenshot(WCHAR* wPath)
{
    // Create variables for bitmap requirements
    BITMAPFILEHEADER bmpFileHeader;
    BITMAPINFOHEADER bmpInfoHeader;
    BITMAPINFO bmpInfo;
    HGDIOBJ hTempBitmap;
    HBITMAP hBitmap;
    BITMAP bmpAllDesktops;
    HDC hDC, hMemDC;
    LONG lWidth, lHeight;
    BYTE* bBits = NULL;
    HANDLE hHeap = GetProcessHeap();
    DWORD cbBits, dwWritten = 0;
    HANDLE hFile;
    INT x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    INT y = GetSystemMetrics(SM_YVIRTUALSCREEN);

    // Set memory requirements and initialize
    ZeroMemory(&bmpFileHeader, sizeof(BITMAPFILEHEADER));
    ZeroMemory(&bmpInfoHeader, sizeof(BITMAPINFOHEADER));
    ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
    ZeroMemory(&bmpAllDesktops, sizeof(BITMAP));

    // Get device context
    hDC = GetDC(NULL);
    hTempBitmap = GetCurrentObject(hDC, OBJ_BITMAP);
    GetObjectW(hTempBitmap, sizeof(BITMAP), &bmpAllDesktops);

    // Collect dimensions
    lWidth = bmpAllDesktops.bmWidth;
    lHeight = bmpAllDesktops.bmHeight;

    DeleteObject(hTempBitmap);

    bmpFileHeader.bfType = (WORD)('B' | ('M' << 8));
    bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfoHeader.biBitCount = 24; //bitmap has max 2^24 colors
    bmpInfoHeader.biCompression = BI_RGB;
    bmpInfoHeader.biPlanes = 1;
    bmpInfoHeader.biWidth = lWidth;
    bmpInfoHeader.biHeight = lHeight;

    bmpInfo.bmiHeader = bmpInfoHeader;

    cbBits = (((24 * lWidth + 31) & ~31) / 8) * lHeight;

    hMemDC = CreateCompatibleDC(hDC);
    hBitmap = CreateDIBSection(hDC, &bmpInfo, DIB_RGB_COLORS, (VOID**)&bBits, NULL, 0);
    SelectObject(hMemDC, hBitmap);
    BitBlt(hMemDC, 0, 0, lWidth, lHeight, hDC, x, y, SRCCOPY);

    // Write file information
    hFile = CreateFileW(wPath, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        DeleteDC(hMemDC);
        ReleaseDC(NULL, hDC);
        DeleteObject(hBitmap);

        return FALSE;
    }
    WriteFile(hFile, &bmpFileHeader, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
    WriteFile(hFile, &bmpInfoHeader, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
    WriteFile(hFile, bBits, cbBits, &dwWritten, NULL);
    FlushFileBuffers(hFile);
    CloseHandle(hFile);

    DeleteDC(hMemDC);
    ReleaseDC(NULL, hDC);
    DeleteObject(hBitmap);

    return TRUE;
}