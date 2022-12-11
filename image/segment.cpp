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
#include "segment.h"
#include <cstring>

namespace uld {

      segment::segment(const char* name, unsigned int type, unsigned int flags, int align) noexcept:
      pool(align),
      m_name(name),
      m_type(type),
      m_flags(flags)
{
}

      segment::~segment()
{
}

const char* segment::get_name() const noexcept
{
      if((m_name != nullptr) && (m_name[0] != 0)) {
          return m_name;
      }
      return nullptr;
}

bool  segment::has_name(const char* name) const noexcept
{
      const char* l_name = get_name();
      if(name != nullptr) {
          return std::strcmp(l_name, name) == 0;
      } else
          return l_name == nullptr;
}

bool  segment::has_type(unsigned int value) const noexcept
{
      return m_type == value;
}

unsigned int  segment::get_type() const noexcept
{
      return m_type;
}

bool  segment::has_flags(unsigned int value) const noexcept
{
      return (m_flags & value) == value;
}

unsigned int  segment::get_flags() const noexcept
{
      return m_flags;
}

/*namespace uld*/ }
