#pragma once

#include "config.h"

#include <cstdint>
#include <sstream>

#include <boost/preprocessor.hpp>

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

#define GANYMEDE_DEFINE_ENUM_VAL_STR(r, name, val) BOOST_PP_STRINGIZE(val)
#define GANYMEDE_ENUM(name, val_seq)                                                           \
    enum class name : uint32 {                                                                 \
        BOOST_PP_SEQ_ENUM(val_seq)                                                             \
    };                                                                                         \
    static const char* BOOST_PP_CAT(name, Strings[] = ) {                                      \
        BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(GANYMEDE_DEFINE_ENUM_VAL_STR, name, val_seq)) \
    };                                                                                         \
    inline const char* name##ToString(name value) { return name##Strings[uint32(value)]; }

#define GANYMEDE_ENUM_NUMBERED_SEQ_X(s, data, elem) BOOST_PP_TUPLE_ELEM(0, elem) = BOOST_PP_TUPLE_ELEM(1, elem)
#define GANYMEDE_ENUM_NUMBERED_SEQ_ELEM(s, data, elem) BOOST_PP_TUPLE_ELEM(0, elem)
#define GANYMEDE_ENUM_NUMBERED_PRINT_NAME(r, name, elem) if (value == name::elem) return BOOST_PP_STRINGIZE(elem);
#define GANYMEDE_ENUM_NUMBERED(name, val_seq)                                                  \
    enum class name : uint32 {                                                                 \
        BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(GANYMEDE_ENUM_NUMBERED_SEQ_X, ~, val_seq))    \
    };                                                                                         \
    static const char* BOOST_PP_CAT(name, Strings[] = ) {                                      \
        BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(GANYMEDE_DEFINE_ENUM_VAL_STR, name, val_seq)) \
    };                                                                                         \
    static const char* name##ToString(name value) {                                            \
        BOOST_PP_SEQ_FOR_EACH(GANYMEDE_ENUM_NUMBERED_PRINT_NAME, name, BOOST_PP_SEQ_TRANSFORM(GANYMEDE_ENUM_NUMBERED_SEQ_ELEM, ~, val_seq)) \
        return "IllegalValue";                                                                 \
    };

#define GANYMEDE_ENUM_FLAGS_SEQ_X(s, data, elem) BOOST_PP_CAT(data, BOOST_PP_TUPLE_ELEM(0, elem)) = BOOST_PP_TUPLE_ELEM(1, elem)
#define GANYMEDE_ENUM_FLAGS_SEQ_ELEM(s, data, elem) BOOST_PP_CAT(data, BOOST_PP_TUPLE_ELEM(0, elem))
#define GANYMEDE_ENUM_FLAGS_SEQ_ELEM_NAME(s, data, elem) BOOST_PP_TUPLE_ELEM(0, elem)
#define GANYMEDE_ENUM_FLAGS_PRINT_NAME(r, name, elem) if (+(value & BOOST_PP_CAT(name, elem))) ss << BOOST_PP_STRINGIZE(elem) << " ";
#define GANYMEDE_ENUM_FLAGS(name, val_seq)                                                     \
    enum name : uint32 {                                                                       \
        BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(GANYMEDE_ENUM_FLAGS_SEQ_X, name, val_seq))    \
    };                                                                                         \
    static const char* BOOST_PP_CAT(name, Strings[] = ) {                                      \
        BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(GANYMEDE_DEFINE_ENUM_VAL_STR, name, val_seq)) \
    };                                                                                         \
    static std::string name##ToString(name value) {                                            \
        std::stringstream ss;                                                                  \
        BOOST_PP_SEQ_FOR_EACH(GANYMEDE_ENUM_FLAGS_PRINT_NAME, name, BOOST_PP_SEQ_TRANSFORM(GANYMEDE_ENUM_FLAGS_SEQ_ELEM_NAME, ~, val_seq)) \
        return ss.str();                                                                       \
    };