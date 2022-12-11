#ifndef uld_bits_common_h
#define uld_bits_common_h
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
#include <uld.h>
#include <cstdlib>

namespace uld {

/* b_se()
   sign-extend a quantity of a specified number of bits to a native type
*/
template<typename Xt>
constexpr Xt    b_se(Xt value, int bits) noexcept {
      Xt b_mask = 1 << (bits - 1);
      Xt b_ret  = (value ^ b_mask) - b_mask;
      return b_ret;
}

/* b_can_reach()
   check if a certain address is reachable relative to a base adress, within the specified number of bits
*/
constexpr bool  b_can_reach(std::uint8_t* base, std::uint8_t* address, int bits) noexcept {
      int b_platform_bits = sizeof(std::size_t) * 8;
      if(bits < b_platform_bits) {
          ptrdiff_t  b_dp = address - base;
          ssize_t    b_lb = 0 - (1 << (bits - 1));
          ssize_t    b_ub = 0 + (1 << (bits - 1)) - 1;
          return    (b_dp >= b_lb) && (b_dp <= b_ub);
      }
      return false;
}

/* b_and()
   apply a bit mask to a native address or a value
*/
template<typename Xt>
constexpr long int b_and(Xt value, long int mask) noexcept {
      auto   b_lhs = reinterpret_cast<long int>(value);
      auto   b_ret = b_lhs & mask;
      return b_ret;
}

/* b_xor()
   xor between addresses or values
*/
template<typename Xt>
constexpr long int b_xor(Xt lhs, Xt rhs) noexcept {
      auto   b_lhs = reinterpret_cast<long int>(lhs);
      auto   b_rhs = reinterpret_cast<long int>(rhs);
      auto   b_ret = b_lhs & b_rhs;
      return b_ret;
}

/*namespace uld*/ }
#endif
