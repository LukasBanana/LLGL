/*
 * Float16Compressor.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Float16Compressor.h"


namespace LLGL
{


/*
This class has been adopted from a public-domain code sample.
see http://stackoverflow.com/questions/1659440/32-bit-to-16-bit-floating-point-conversion
*/
class Float16Compressor
{

    public:

        static std::uint16_t Compress(float value)
        {
            Bits v, s;
            v.f = value;
            std::uint32_t sign = v.si & signN;
            v.si ^= sign;
            sign >>= shiftSign; // logical shift
            s.si = mulN;
            s.si = static_cast<std::int32_t>(s.f * v.f); // correct subnormals
            v.si ^= (s.si ^ v.si) & -(minN > v.si);
            v.si ^= (infN ^ v.si) & -((infN > v.si) & (v.si > maxN));
            v.si ^= (nanN ^ v.si) & -((nanN > v.si) & (v.si > infN));
            v.ui >>= shift; // logical shift
            v.si ^= ((v.si - maxD) ^ v.si) & -(v.si > maxC);
            v.si ^= ((v.si - minD) ^ v.si) & -(v.si > subC);
            return (v.ui | sign);
        }

        static float Decompress(std::uint16_t value)
        {
            Bits v;
            v.ui = value;
            int32_t sign = v.si & signC;
            v.si ^= sign;
            sign <<= shiftSign;
            v.si ^= ((v.si + minD) ^ v.si) & -(v.si > subC);
            v.si ^= ((v.si + maxD) ^ v.si) & -(v.si > maxC);
            Bits s;
            s.si = mulC;
            s.f *= v.si;
            int32_t mask = -(norC > v.si);
            v.si <<= shift;
            v.si ^= (s.si ^ v.si) & mask;
            v.si |= sign;
            return v.f;
        }

    private:

        union Bits
        {
            float           f;
            std::int32_t    si;
            std::uint32_t   ui;
        };

        static const std::int32_t shift     = 13;
        static const std::int32_t shiftSign = 16;

        static const std::int32_t infN      = 0x7f800000; // flt32 infinity
        static const std::int32_t maxN      = 0x477fe000; // max flt16 normal as a flt32
        static const std::int32_t minN      = 0x38800000; // min flt16 normal as a flt32
        static const std::int32_t signN     = 0x80000000; // flt32 sign bit

        static const std::int32_t infC      = (infN >> shift);
        static const std::int32_t nanN      = ((infC + 1) << shift); // minimum flt16 nan as a flt32
        static const std::int32_t maxC      = (maxN >> shift);
        static const std::int32_t minC      = (minN >> shift);
        static const std::int32_t signC     = (signN >> shiftSign); // flt16 sign bit

        static const std::int32_t mulN      = 0x52000000; // (1 << 23) / minN
        static const std::int32_t mulC      = 0x33800000; // minN / (1 << (23 - shift))

        static const std::int32_t subC      = 0x003ff; // max flt32 subnormal down shifted
        static const std::int32_t norC      = 0x00400; // min flt32 normal down shifted

        static const std::int32_t maxD      = (infC - maxC - 1);
        static const std::int32_t minD      = (minC - subC - 1);

};


LLGL_EXPORT std::uint16_t CompressFloat16(float value)
{
    return Float16Compressor::Compress(value);
}

LLGL_EXPORT float DecompressFloat16(std::uint16_t value)
{
    return Float16Compressor::Decompress(value);
}


} // /namespace LLGL



// ================================================================================
