#ifndef uld_image_h
#define uld_image_h
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
#include "target.h"
#include "image/data.h"
#include "image/segment.h"
#include "image/string_table.h"
#include "image/symbol_table.h"
#include "image/program_table.h"

namespace uld {

/* image
   container for an in-memory executable image
   TODO:
    - recover (rollback latest factory additions in case of a failure)
    - global relocations (factory to also export unsolved relocations)
*/
class image
{
  target*         m_target;

  string_table_t  m_string_table;
  symbol_table_t  m_symbol_table;
  program_table_t m_program;
  unsigned int    m_state;

  private:
          void   uld_set();
          bool   uld_load_elf32(bin_bfd_t&, unsigned int) noexcept;
          bool   uld_load_object(raw_bfd_t&, unsigned int) noexcept;
          bool   uld_load_library(raw_bfd_t&, unsigned int) noexcept;
          bool   uld_error(int, const char*, ...) noexcept;
          void   uld_clear() noexcept;

  public:
          image(target*) noexcept;
          image(const image&) noexcept = delete;
          image(image&&) noexcept = delete;
          ~image();

          bool      load(const char*) noexcept;
          void      reset() noexcept;

          symbol_t* find_symbol(const char*, unsigned int = symbol_t::bind_any) noexcept;
          symbol_t* make_symbol(const char*, unsigned int, unsigned int) noexcept;
          symbol_t* make_symbol(const char*, unsigned int, unsigned int, void*, void* = nullptr) noexcept;

          segment*  get_segment_by_name(const char*) noexcept;
          segment*  get_segment_by_attributes(unsigned int, unsigned int) noexcept;
          int       get_segment_count() const noexcept;

          target*   get_target() noexcept;
          auto      get_string_table() noexcept -> string_table_t*;
          auto      get_symbol_table() noexcept -> symbol_table_t*;
          auto      get_program_table() noexcept -> program_table_t*;

          image& operator=(const image&) noexcept = delete;
          image& operator=(image&&) noexcept = delete;
};
  
/*namespace uld*/ }
#endif
