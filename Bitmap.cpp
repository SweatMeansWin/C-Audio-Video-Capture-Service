#include "PreCompiled.h"
#include "Bitmap.h"

#include "Logging.h"

#include <fstream>

bool WriteBitmap(const std::wstring& sFileName, BITMAPINFOHEADER *pBMI, DWORD cbBMI, BYTE *pData, DWORD cbData) {
	return WriteBitmap(std::string(sFileName.cbegin(), sFileName.cend()), pBMI, cbBMI, pData, cbData);
}

bool WriteBitmap(const std::string& sFileName, BITMAPINFOHEADER *pBMI, DWORD cbBMI, BYTE *pData, DWORD cbData) {
	std::ofstream f(sFileName, std::ios::binary);
	if (f.fail()) {
		std::stringstream ss;
		ss << "WriteBitmap::Failed open [" << sFileName << L"]";
		LogError(ss.str(), ::GetLastError());

		return false;
	}
	
	BITMAPFILEHEADER bmf = {};
	bmf.bfType = 'MB';
	bmf.bfSize = cbBMI + cbData + (DWORD)sizeof(bmf);
	bmf.bfOffBits = (DWORD)sizeof(bmf) + cbBMI;
	
	try {
		f.write((LPCSTR)&bmf, sizeof(bmf));
		f.write((LPCSTR)pBMI, cbBMI);
		f.write((LPCSTR)pData, cbData);
	}
	catch (const std::ofstream::failure& e) {
		std::stringstream ss;
		ss << "WriteBitmap::Failed write [" << sFileName << L"]. " << e.what();
		LogError(ss.str(), e.code().value());

		f.close();
		std::remove(sFileName.c_str());
		return false;
	}

	return true;
}
