#pragma once
#include "DMA/DMA.h"

class SignatureScanner
{
public:
    static uint64_t FindSignature(DMA_Connection* Conn, const char* signature, uint64_t range_start, uint64_t range_end, int PID = 0);

private:
    static const char* hexdigits;
    static uint8_t GetByte(const char* hex);
};