#include "pch.h"
#include "Signature Scanner.h"
#include "DMA/Process.h"
#include "Game/EFT.h"

const char* SignatureScanner::hexdigits =
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\001\002\003\004\005\006\007\010\011\000\000\000\000\000\000"
"\000\012\013\014\015\016\017\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\012\013\014\015\016\017\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"

"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000";

uint8_t SignatureScanner::GetByte(const char* hex)
{
    return static_cast<uint8_t>((hexdigits[static_cast<unsigned char>(hex[0])] << 4) |
        hexdigits[static_cast<unsigned char>(hex[1])]);
}

uint64_t SignatureScanner::FindSignature(DMA_Connection* Conn, const char* signature, uint64_t range_start, uint64_t range_end, int PID)
{
    if (!signature || signature[0] == '\0' || range_start >= range_end)
        return 0;

    if (PID == 0)
        PID = EFT::GetProcess().GetPID();

    std::vector<uint8_t> buffer(range_end - range_start);
    if (!VMMDLL_MemReadEx(Conn->GetHandle(), PID, range_start, buffer.data(), buffer.size(), 0, VMMDLL_FLAG_NOCACHE))
        return 0;

    const char* pat = signature;
    uint64_t first_match = 0;

    for (uint64_t i = range_start; i < range_end; i++)
    {
        if (*pat == '?' || buffer[i - range_start] == GetByte(pat))
        {
            if (!first_match)
                first_match = i;

            if (!pat[2]) // end of pattern
                break;

            pat += (*pat == '?') ? 2 : 3;
        }
        else
        {
            pat = signature;
            first_match = 0;
        }
    }

    return first_match;
}