#pragma once

#include <atlimage.h>

bool WriteBitmap(const std::string& sFileName, BITMAPINFOHEADER *pBMI, DWORD cbBMI, BYTE *pData, DWORD cbData);
bool WriteBitmap(const std::wstring& sFileName, BITMAPINFOHEADER *pBMI, DWORD cbBMI, BYTE *pData, DWORD cbData);
