#pragma once
#include "Cipher.h"

class GOST :
	public CCipherAlgorithm
{
public:
	static bool Encrypt(uint32_t* lpBuffer, unsigned long long ullBufferSize);
	static bool Decrypt(uint32_t* lpBuffer, unsigned long long ullBufferSize);
};

