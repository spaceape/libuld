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
#include "string_table.h"
#include <cstring>

namespace uld {

      string_table_t::string_table_t() noexcept:
      pool()
{
      // reserve a first empty string entry
      make_string(nullptr, 1);
}

      string_table_t::~string_table_t()
{
}

char* string_table_t::make_string(const char* string_ptr, int string_length) noexcept
{
      char* l_string_ptr = nullptr;
      int   l_string_size;
      if(string_length <= 0) {
          if(string_ptr != nullptr) {
              l_string_size = std::strlen(string_ptr) + 1;
          } else
              return nullptr;
      } else
          l_string_size = string_length + 1;
      l_string_ptr = raw_get(l_string_size);
      if(l_string_ptr != nullptr) {
          if(string_ptr != nullptr) {
              std::memcpy(l_string_ptr, string_ptr, l_string_size);
          } else
              std::memset(l_string_ptr, 0, l_string_size);
      }
      return l_string_ptr;
}

char* string_table_t::get_string(int offset) noexcept
{
      return get_offset_ptr(offset);
}

/*namespace uld*/ }
