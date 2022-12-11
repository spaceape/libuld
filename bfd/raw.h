#ifndef uld_bfd_raw_h
#define uld_bfd_raw_h
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
#include "util/cache.h"
#include "util/file.h"

namespace uld {

/* raw
   file decoder for artifacts of any supported type
*/
class raw_bfd_t
{
  unsigned int  m_type;

  protected:
  using file_ptr     = util::file_ptr;
  using data_cache_t = util::data_cache_t;

  protected:
  file_ptr      m_file_ptr;
  int           m_file_offset;

  public:
          raw_bfd_t(const char*, int = 0, int = FA_OPEN_EXISTING | FA_READ) noexcept;
          raw_bfd_t(raw_bfd_t&) noexcept;
          raw_bfd_t(const raw_bfd_t&) noexcept = delete;
          raw_bfd_t(raw_bfd_t&&) noexcept = delete;
          ~raw_bfd_t() noexcept;

          unsigned int    get_type() const noexcept;
          bool            has_type(unsigned int) const noexcept;

          util::file_ptr& get_file_ptr() noexcept;
          int             get_file_offset() const noexcept;

                     operator bool() const noexcept;

          raw_bfd_t& operator=(const raw_bfd_t&) = delete;
          raw_bfd_t& operator=(raw_bfd_t&&) = delete;
};
  
/*namespace uld*/ }
#endif
