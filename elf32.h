#ifndef uld_elf32_util_h
#define uld_elf32_util_h
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
#include "image/data.h"
#include "image/string_table.h"
#include "image/symbol_table.h"
#include <elf.h>
#include <vector>

namespace uld {
namespace elf32 {

class factory
{
  image*  m_image;
  target* m_target;

  string_table_t          m_string_pool;  // local string cache
  symbol_table_t          m_symbol_pool;  // local symbol cache

  std::vector<section_t>  m_shdr_map;
  std::vector<symbol_t*>  m_symbol_map;
  std::vector<binding_t>  m_bind_list;

  int     m_shdr_count;
  bool    m_shdr_have_code;
  bool    m_shdr_have_data;
  bool    m_shdr_have_symtab;
  bool    m_shdr_have_rel;

  private:
          auto   uld_get_local_section(int) noexcept -> section_t*;
          auto   uld_get_local_section(symbol_t*, int* = nullptr) noexcept -> section_t*;
          auto   uld_get_local_symbol(int) noexcept -> symbol_t*;
          auto   uld_get_global_address(symbol_t*) noexcept -> std::uint8_t*;
          auto   uld_get_symbol_address(symbol_t*) noexcept -> std::uint8_t*;
          auto   uld_get_symbol_address(symbol_t*, std::int32_t) noexcept -> std::uint8_t*;
          auto   uld_get_virtual_address(symbol_t*) noexcept -> std::uint8_t*;
          auto   uld_get_virtual_address(symbol_t*, std::int32_t) noexcept -> std::uint8_t*;
          auto   uld_get_base_address(symbol_t*) noexcept -> std::uint8_t*;

          auto   uld_get_section_data(elf32_bfd_t&, Elf32_Shdr&, int, std::int32_t, std::int32_t) noexcept -> std::uint8_t*;
          auto   uld_get_section_data(elf32_bfd_t&, int, std::int32_t, std::int32_t) noexcept -> std::uint8_t*;
          bool   uld_load_section(elf32_bfd_t&, Elf32_Shdr&, int) noexcept;
          bool   uld_load_section(elf32_bfd_t&, Elf32_Shdr&, int, std::int32_t, std::int32_t) noexcept;
          bool   uld_load_symbol(elf32_bfd_t&, Elf32_Shdr&, Elf32_Sym&, int) noexcept;
          bool   uld_resolve_rel(elf32_bfd_t&, Elf32_Shdr&, Elf32_Rel&, std::int32_t = 0) noexcept;
          bool   uld_resolve_rela(elf32_bfd_t&, Elf32_Shdr&, Elf32_Rela&) noexcept;
          bool   uld_import(elf32_bfd_t&) noexcept;
          bool   uld_resolve(elf32_bfd_t&) noexcept;
          bool   uld_export() noexcept;
          bool   uld_revert() noexcept;
          bool   uld_error(int, const char*, const char*, int, ...) noexcept;
          void   uld_clear() noexcept;

  public:
          factory(image*) noexcept;
          factory(const factory&) noexcept = delete;
          factory(factory&&) noexcept = delete;
          ~factory();

          bool     prefetch(elf32_bfd_t&) noexcept;
          bool     collect(elf32_bfd_t&) noexcept;

          factory& operator=(const factory&) noexcept = delete;
          factory& operator=(factory&&) noexcept = delete;
};

/*namespace elf32*/ }
/*namespace uld*/ }
#endif
