#ifndef uld_bits_arm_h
#define uld_bits_arm_h
/** 
    Copyright (c) 2022, wicked systems
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following
    conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following
      disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
      disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of wicked systems nor the names of its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
    EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/
#include "common.h"

namespace uld {

/* b_se12()
   sign-extend a 12 bit signed- to a 32 bit signed integer
*/
constexpr int   b_se12(std::int32_t value) noexcept
{
      return b_se(value, 12);
}

/* b_se23()
   sign-extend a 23 bit signed- to a 32 bit signed integer
*/
constexpr int   b_se23(std::int32_t value) noexcept
{
      return b_se(value, 23);
}

/* b_se26()
   sign-extend a 26 bit signed- to a 32 bit signed integer
*/
constexpr int   b_se26(std::int32_t value) noexcept
{
      return b_se(value, 26);
}

/* b_se30()
   sign-extend a 30 bit signed- to a 32 bit signed integer
*/
constexpr int   b_se30(std::int32_t value) noexcept
{
      return b_se(value, 30);
}

/* b_arm_get8()
*/
constexpr void  b_arm_get8(std::uint16_t* r, std::int32_t& value) noexcept
{
    value = r[0] & 0xff;
}

/* b_arm_set8()
*/
constexpr void  b_arm_set8(std::uint16_t* r, std::int32_t value) noexcept
{
      r[0] |= value & 0xff;
}

/* b_arm_get12()
*/
constexpr void  b_arm_get12(std::uint16_t* r, std::int32_t& value) noexcept
{
    value = r[0] & 0x0fff;
}

/* b_arm_set12()
*/
constexpr void  b_arm_set12(std::uint16_t* r, std::int32_t value) noexcept
{
      r[0] &= ~0x0fff;
      r[0] |= value & 0x0fff;
}

/* b_arm_get16()
*/
constexpr void  b_arm_get16(std::uint16_t* r, std::int32_t& value) noexcept
{
    value = r[0];
}

/* b_arm_set16()
*/
constexpr void  b_arm_set16(std::uint16_t* r, std::int32_t value) noexcept
{
      r[0] = value & 0xffff;
}

/* b_arm_get30()
*/
constexpr void  b_arm_get30(std::uint8_t* p, std::int32_t& value) noexcept
{
      value = b_se30(p[0] | (p[1] << 8) | (p[2] << 16) | ((p[3] & 0x3f) << 24));
}

/* b_arm_set30()
*/
constexpr void  b_arm_set30(std::uint8_t* p, std::int32_t value) noexcept
{
      p[0] = value & 0x000000ff;
      p[1] =(value & 0x0000ff00) >> 8;
      p[2] =(value & 0x00ff0000) >> 16;
      p[3] =(value & 0x3f000000) >> 24;
}

/* b_arm_get32()
*/
constexpr void  b_arm_get32(std::uint8_t* p, std::int32_t& value) noexcept
{
      value = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

/* b_arm_set32()
*/
constexpr void  b_arm_set32(std::uint8_t* p, std::int32_t value) noexcept
{
      p[0] = value & 0x000000ff;
      p[1] =(value & 0x0000ff00) >> 8;
      p[2] =(value & 0x00ff0000) >> 16;
      p[3] =(value & 0xff000000) >> 24;
}

/* b_arm_getld12()
*/
// constexpr void  b_arm_getld12(std::uint16_t* r, std::int32_t& value) noexcept
// {
//     value = b_se12(r[0] & 0x0fff);
// }

/* b_armt_getbl26()
   get sign-extended 22 bit address encoded into a thumb instruction stream pointed to by `r`
*/
constexpr void  b_arm_getbl26(std::uint16_t* r, std::int32_t& value) noexcept
{
      value = b_se26((r[0] | ((r[1] & 0x00ff) << 16)) << 2);
}

/* b_armt_setbl26()
   set 22 bit address into the thumb instruction stream pointed to by `i`
*/
constexpr void  b_arm_setbl26(std::uint16_t* r, std::int32_t value) noexcept
{
      r[0] = (value & 0x0003ffff) >> 2;
      r[1] = (value & 0x03fc0000) >> 18;
}

/* b_armt_getbl22()
   get sign-extended 22 bit address encoded into a thumb instruction stream pointed to by `r`
*/
constexpr void  b_armt_getbl22(std::uint16_t* r, std::int32_t& value) noexcept
{
      value = b_se23(((r[1] & 0x07ff) << 1) | ((r[0] & 0x07ff) << 12));
}

/* b_armt_setbl22()
   set 22 bit address into the thumb instruction stream pointed to by `r`
*/
constexpr void  b_armt_setbl22(std::uint16_t* r, std::int32_t value) noexcept
{
      r[1] &= ~(0x07ff);
      r[1] |= (value & 0x0000'0fff) >> 1;
      r[0] &= ~(0x07ff);
      r[0] |= (value & 0x007f'f000) >> 12;
}

/*namespace uld*/ }
#endif
