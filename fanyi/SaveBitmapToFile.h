#pragma once
#include <Windows.h>

BOOL SaveBitmapToFile(LPCTSTR lpszFilePath, HBITMAP hBm)
{
    //  ����λͼ�ļ���ͷ
    BITMAPFILEHEADER bmfh;
    //  ����λͼ��Ϣ��ͷ
    BITMAPINFOHEADER bmih;

    //  ��ɫ�峤��
    int nColorLen = 0;
    //  ��ɫ���С
    DWORD dwRgbQuadSize = 0;
    //  λͼ��С
    DWORD dwBmSize = 0;
    //  �����ڴ��ָ��
    HGLOBAL    hMem = NULL;

    LPBITMAPINFOHEADER     lpbi;

    BITMAP bm;
    HDC hDC;

    HANDLE hFile = NULL;

    DWORD dwWritten;

    GetObject(hBm, sizeof(BITMAP), &bm);

    bmih.biSize = sizeof(BITMAPINFOHEADER);    // ���ṹ��ռ���ֽ�
    bmih.biWidth = bm.bmWidth;            // λͼ��
    bmih.biHeight = bm.bmHeight;            // λͼ��
    bmih.biPlanes = 1;
    bmih.biBitCount = bm.bmBitsPixel;    // ÿһͼ�ص�λ��
    bmih.biCompression = BI_RGB;            // ��ѹ��
    bmih.biSizeImage = 0;  //  λͼ��С
    bmih.biXPelsPerMeter = 0;
    bmih.biYPelsPerMeter = 0;
    bmih.biClrUsed = 0;
    bmih.biClrImportant = 0;

    //  ����λͼͼ����������С 
    dwBmSize = 4 * ((bm.bmWidth * bmih.biBitCount + 31) / 32) * bm.bmHeight;

    //  ���ͼ��λ <= 8bit�����е�ɫ��
    if (bmih.biBitCount <= 8)
    {
        nColorLen = (1 << bm.bmBitsPixel);
    }

    //  �����ɫ���С
    dwRgbQuadSize = nColorLen * sizeof(RGBQUAD);

    //  �����ڴ�
    hMem = GlobalAlloc(GHND, dwBmSize + dwRgbQuadSize + sizeof(BITMAPINFOHEADER));

    if (NULL == hMem)
    {
        return FALSE;
    }

    //  �����ڴ�
    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hMem);

    //  ��bmih�е�����д�������ڴ���
    *lpbi = bmih;
    hDC = GetDC(NULL);
    //  ��λͼ�е�������bits����ʽ����lpData�С�
    GetDIBits(hDC, hBm, 0, (DWORD)bmih.biHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwRgbQuadSize, (BITMAPINFO*)lpbi, (DWORD)DIB_RGB_COLORS);

    bmfh.bfType = 0x4D42;  // λͼ�ļ����ͣ�BM
    bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwRgbQuadSize + dwBmSize;  // λͼ��С
    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;
    bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwRgbQuadSize;  // λͼ�������ļ�ͷ����ƫ����

    //  �����������д���ļ���

    hFile = CreateFile(lpszFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        return FALSE;
    }

    //  д��λͼ�ļ�ͷ
    WriteFile(hFile, (LPSTR)&bmfh, sizeof(BITMAPFILEHEADER), (DWORD*)&dwWritten, NULL);
    //  д��λͼ����
    WriteFile(hFile, (LPBITMAPINFOHEADER)lpbi, bmfh.bfSize - sizeof(BITMAPFILEHEADER), (DWORD*)&dwWritten, NULL);

    GlobalFree(hMem);
    CloseHandle(hFile);

    return TRUE;
}