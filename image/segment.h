#ifndef uld_image_segment_h
#define uld_image_segment_h
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
#include "pool.h"

namespace uld {

/* segment
*/
class segment: public pool<std::uint8_t, page_size>
{
  const char*   m_name;
  unsigned int  m_type;
  unsigned int  m_flags;

  public:
        segment(const char*, unsigned int, unsigned int, int = 0) noexcept;
        segment(const segment&) noexcept = delete;
        segment(segment&&) noexcept = delete;
        ~segment();

        const char*   get_name() const noexcept;
        bool  has_name(const char*) const noexcept;
        bool  has_type(unsigned int) const noexcept;
        unsigned int  get_type() const noexcept;
        bool  has_flags(unsigned int) const noexcept;
        unsigned int  get_flags() const noexcept;

        segment& operator=(const segment&) noexcept = delete;
        segment& operator=(segment&&) noexcept = delete;
};

/*namespace uld*/ }
#endif
