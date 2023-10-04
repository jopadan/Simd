/*
* Simd Library (http://ermig1979.github.io/Simd).
*
* Copyright (c) 2011-2019 Yermalayeu Ihar.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#include "Simd/SimdMemory.h"
#include "Simd/SimdStore.h"
#include "Simd/SimdExtract.h"
#include "Simd/SimdDescrInt.h"
#include "Simd/SimdDescrIntCommon.h"
#include "Simd/SimdCpu.h"

namespace Simd
{
#ifdef SIMD_NEON_ENABLE    
    namespace Neon
    {
        template<int bits> int32_t Correlation(const uint8_t* a, const uint8_t* b, size_t size);

        template<> int32_t Correlation<7>(const uint8_t* a, const uint8_t* b, size_t size)
        {
            assert(size % 8 == 0);
            uint32x4_t _ab = K32_00000000;
            for (size_t i = 0; i < size; i += 8)
            {
                uint32x4_t _a0 = vandq_u32(vshlq_u32(vdupq_n_u32(*(uint32_t*)(a + 0)), C7_SHL0), C7_AND);
                uint32x4_t _b0 = vandq_u32(vshlq_u32(vdupq_n_u32(*(uint32_t*)(b + 0)), C7_SHL0), C7_AND);
                _ab = vmlaq_u32(_ab, _a0, _b0);
                uint32x4_t _a1 = vandq_u32(vshlq_u32(vdupq_n_u32(*(uint32_t*)(a + 3)), C7_SHL1), C7_AND);
                uint32x4_t _b1 = vandq_u32(vshlq_u32(vdupq_n_u32(*(uint32_t*)(b + 3)), C7_SHL1), C7_AND);
                _ab = vmlaq_u32(_ab, _a1, _b1);
                a += 7;
                b += 7;
            }
            return ExtractSum32u(_ab);
        }

        template<bool align> SIMD_INLINE void Correlation8(const uint8_t* a, const uint8_t* b, uint32x4_t & ab)
        {
            uint8x16_t _a = Load<align>(a);
            uint8x16_t _b = Load<align>(b);
            ab = vpadalq_u16(ab, vmull_u8(Half<0>(_a), Half<0>(_b)));
            ab = vpadalq_u16(ab, vmull_u8(Half<1>(_a), Half<1>(_b)));
        }

        template<> int32_t Correlation<8>(const uint8_t* a, const uint8_t* b, size_t size)
        {
            assert(size % 8 == 0);
            uint32x4_t _ab = K32_00000000;
            size_t i = 0, size16 = AlignLo(size, 16);
            if (Aligned(a) && Aligned(b))
            {
                for (; i < size16; i += 16)
                    Correlation8<true>(a + i, b + i, _ab);
            }
            else
            {
                for (; i < size16; i += 16)
                    Correlation8<false>(a + i, b + i, _ab);
            }
            for (; i < size; i += 8)
            {
                uint8x8_t _a = LoadHalf<false>(a + i);
                uint8x8_t _b = LoadHalf<false>(b + i);
                _ab = vpadalq_u16(_ab, vmull_u8(_a, _b));
            }
            return ExtractSum32u(_ab);
        }

        template<int bits> void CosineDistance(const uint8_t* a, const uint8_t* b, size_t size, float* distance)
        {
            float abSum = (float)Correlation<bits>(a + 16, b + 16, size);
            Base::DecodeCosineDistance(a, b, abSum, distance);
        }

        //-------------------------------------------------------------------------------------------------

        Base::DescrInt::CosineDistancePtr GetCosineDistance(size_t depth)
        {
            switch (depth)
            {
            //case 4: return CosineDistance<4>;
            //case 5: return CosineDistance<5>;
            //case 6: return CosineDistance<6>;
            //case 7: return CosineDistance<7>;
            case 8: return CosineDistance<8>;
            default: Base::GetCosineDistance(depth);
            }
        }
    }
#endif// SIMD_NEON_ENABLE
}
