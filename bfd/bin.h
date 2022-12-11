#ifndef uld_bfd_bin_h
#define uld_bfd_bin_h
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
#include "raw.h"

namespace uld {

/* bin
   binary file decoder for any supported executable/linkable artifacts
*/
class bin_bfd_t: public raw_bfd_t
{
  protected:
  unsigned int  m_class;
  unsigned int  m_abi_id;
  unsigned int  m_abi_version;
  bool          m_lsb_bit;
  bool          m_msb_bit;
  bool          m_elf_bit;      // valid ELF object

  protected:

  public:
          bin_bfd_t(raw_bfd_t&) noexcept;
          bin_bfd_t(bin_bfd_t&) noexcept;
          bin_bfd_t(const bin_bfd_t&) noexcept = delete;
          bin_bfd_t(bin_bfd_t&&) noexcept = delete;
          ~bin_bfd_t() noexcept;

          bool  has_class(unsigned int) const noexcept;
          bool  has_abi_id(unsigned int) const noexcept;
          unsigned int get_abi_id() const noexcept;
          bool  has_abi_version(unsigned int) const noexcept;
          unsigned int get_abi_version() const noexcept;
          bool  is_lsb() const noexcept;
          bool  get_lsb() const noexcept;
          bool  is_msb() const noexcept;
          bool  get_msb() const noexcept;
          bool  is_elf() const noexcept;
          
                      operator bool() const noexcept;

          bin_bfd_t&  operator=(const bin_bfd_t&) = delete;
          bin_bfd_t&  operator=(bin_bfd_t&&) = delete;
};

/*namespace uld*/ }
#endif
