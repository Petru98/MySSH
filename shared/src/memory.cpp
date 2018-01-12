#include <memory.hpp>
#include <cstdint>



char* toHex(const void* bin, char* str)
{
    constexpr char digits[] = "0123456789abcdef";
    const uint8_t* bit = reinterpret_cast<const uint8_t*>(bin);
    char* sit = str;

    while((*bit) != '\0')
    {
        (*sit) = digits[((*bit) >> 4) & 0x0f];
        ++sit;
        (*sit) = digits[(*bit) & 0x0f];
        ++sit; ++bit;
    }

    (*sit) = '\0';
    return str;
}

void* fromHex(const char* str, void* bin)
{
    const char* sit = str;
    uint8_t* bit = reinterpret_cast<uint8_t*>(bin);

    while((*sit) != '\0')
    {
        if((*sit) <= '9')
            (*bit) = ((*sit) - '0') << 4;
        else if((*sit) <= 'A')
            (*bit) = ((*sit) - 'A') << 4;
        else if((*sit) <= 'a')
            (*bit) = ((*sit) - 'a') << 4;
        ++sit;

        if((*sit) <= '9')
            (*bit) |= (*sit) - '0';
        else if((*sit) <= 'A')
            (*bit) |= (*sit) - 'A';
        else if((*sit) <= 'a')
            (*bit) |= (*sit) - 'a';
        ++sit; ++bit;
    }

    return bit;
}
