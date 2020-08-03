#pragma once

#include "config.h"

#include <cstdint>
#include <sstream>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

void GanymedeOutputDebugString(const char* str);

// http://madebyevan.com/obscure-cpp-features/?viksra
namespace __hidden__
{
    struct GanymedePrint {
        bool space;
        std::stringstream ss;
        GanymedePrint() : space(false) { ss.clear(); }
        ~GanymedePrint() { ss << std::endl;  GanymedeOutputDebugString(ss.str().c_str()); }

        template <typename T>
        GanymedePrint& operator , (const T& t) {
            if (space) ss << ' ';
            else space = true;
            ss << t;
            return *this;
        }
    };
}

#define GanymedePrint __hidden__::GanymedePrint(),
#define GanymedeDelete(x) if (x) delete x