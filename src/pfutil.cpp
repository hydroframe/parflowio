#include <climits>
#include <cstdint>

#include "pfutil.hpp"

//Check for endianess at compile time, if possible. Otherwise fallback to runtime check
//At least gcc, clang, and icc define this macro. 
#ifdef __BYTE_ORDER__
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define PARFLOWIO_LITTLE_ENDIAN (true)
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define PARFLOWIO_LITTLE_ENDIAN (false)
    #else
        #error "Architecture appears to not be big or little endian. Only big and little endian architectures are supported."
    #endif
//Test for MSVC
#elif defined(_MSC_VER)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #if REG_DWORD == REG_DWORD_LITTLE_ENDIAN
        #define PARFLOWIO_LITTLE_ENDIAN (true)
    #elif REG_DWORD == REG_DWORD_BIG_ENDIAN
        #define PARFLOWIO_LITTLE_ENDIAN (false)
    #else
        #error "Architecture appears to not be big or little endian. Only big and little endian architectures are supported."
    #endif
#else
    //Fallback to runtime check
    const uint32_t runtimeFallbackTestInt = 1;
    const bool runtimeFallbackIsLittleEndian = (*reinterpret_cast<const unsigned char*>(&runtimeFallbackTestInt) == 1);
    #define PARFLOWIO_LITTLE_ENDIAN (runtimeFallbackIsLittleEndian)
#endif


bool isLittleEndian(){
    return PARFLOWIO_LITTLE_ENDIAN;
}

bool isBigEndian(){
    return !(PARFLOWIO_LITTLE_ENDIAN);
}

//Note: on x86_64 compiles down to bswap
uint32_t bswap32(uint32_t data){
    if(!(PARFLOWIO_LITTLE_ENDIAN)) return data;

    static_assert(CHAR_BIT == 8, "Byte conversion requires that char be 8 bits.");
    const unsigned char* alias = reinterpret_cast<unsigned char*>(&data);
    return 
        (static_cast<uint32_t>( alias[0] ) << 24) | 
        (static_cast<uint32_t>( alias[1] ) << 16) | 
        (static_cast<uint32_t>( alias[2] ) <<  8) | 
        (static_cast<uint32_t>( alias[3] ) <<  0);
}

//Note: on x86_64 compiles down to bswap
uint64_t bswap64(uint64_t data){
    if(!(PARFLOWIO_LITTLE_ENDIAN)) return data;

    static_assert(CHAR_BIT == 8, "Byte conversion requires that char be 8 bits.");
    const unsigned char* alias = reinterpret_cast<unsigned char*>(&data);
    return 
        (static_cast<uint64_t>( alias[0] ) << 56) | 
        (static_cast<uint64_t>( alias[1] ) << 48) | 
        (static_cast<uint64_t>( alias[2] ) << 40) | 
        (static_cast<uint64_t>( alias[3] ) << 32) | 
        (static_cast<uint64_t>( alias[4] ) << 24) | 
        (static_cast<uint64_t>( alias[5] ) << 16) | 
        (static_cast<uint64_t>( alias[6] ) <<  8) | 
        (static_cast<uint64_t>( alias[7] ) <<  0);
}
