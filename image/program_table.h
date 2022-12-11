#ifndef uld_image_program_table_h
#define uld_image_program_table_h
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
#include "segment.h"
#include "table.h"
#include <memory>

namespace uld {

class string_table_t;
class symbol_table_t;
  
class program_table_t
{
  target*                   m_target;
  string_table_t*           m_string_table;
  symbol_table_t*           m_symbol_table;

  private:
  static constexpr int      segment_count_min = 8;
  static constexpr int      segment_count_max = 16;

  private:
  std::unique_ptr<segment>  m_segment_list[segment_count_max];
  int                       m_segment_count;

  private:
          int         get_default_segment_mapping(unsigned int, unsigned int) const noexcept;
          int         get_default_segment_alignment(unsigned int, unsigned int) const noexcept;

  public:
          program_table_t(target*, string_table_t*, symbol_table_t*) noexcept;
          program_table_t(const program_table_t&) noexcept = delete;
          program_table_t(program_table_t&&) noexcept = delete;
          ~program_table_t();

          segment*    make_segment(int, unsigned int, unsigned int, int = 0) noexcept;
          segment*    make_segment(const char*, unsigned int, unsigned int, int = 0) noexcept;
          segment*    get_segment_by_index(int) noexcept;
          segment*    get_segment_by_name(const char*) noexcept;
          segment*    get_segment_by_attributes(unsigned int, unsigned int) noexcept;
          int         get_segment_count() const noexcept;
          void        free_segment(segment*) noexcept;

          program_table_t& operator=(const program_table_t&) noexcept = delete;
          symbol_table_t& operator=(program_table_t&&) noexcept = delete;
};

/*namespace uld*/ }
#endif
