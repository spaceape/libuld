#ifndef uld_image_symbol_table_h
#define uld_image_symbol_table_h
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
#include "data.h"
#include "table.h"

namespace uld {

class string_table_t;
  
class symbol_table_t: public table<symbol_t, page_size>
{
  string_table_t* m_string_table;

  public:
          symbol_table_t(string_table_t*) noexcept;
          symbol_table_t(const symbol_table_t&) noexcept = delete;
          symbol_table_t(symbol_table_t&&) noexcept = delete;
          ~symbol_table_t();

          symbol_t* make_symbol(const char*) noexcept;
          symbol_t* make_symbol(const char*, unsigned int, unsigned int) noexcept;
          symbol_t* make_symbol(const char*, int, unsigned int, unsigned int) noexcept;
          symbol_t* find_symbol(const char*, unsigned int = symbol_t::bind_any);

          symbol_table_t& operator=(const symbol_table_t&) noexcept = delete;
          symbol_table_t& operator=(symbol_table_t&&) noexcept = delete;
};

/*namespace uld*/ }
#endif
