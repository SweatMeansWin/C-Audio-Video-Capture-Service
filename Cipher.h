#pragma once

class CCipherAlgorithm
{
public:
	virtual bool Encrypt(uint32_t* lpBuffer, unsigned long long ullBufferSize) = 0;
	virtual bool Decrypt(uint32_t* lpBuffer, unsigned long long ullBufferSize) = 0;
};

