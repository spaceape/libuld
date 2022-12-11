#ifndef uld_bfd_elf32_h
#define uld_bfd_elf32_h
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
#include "bin.h"
#include <elf.h>

namespace uld {

/* elf32_bfd_t
*/
class elf32_bfd_t: public bin_bfd_t
{
  data_cache_t  m_str_cache;            // cache for the string table(s)
  int           m_str_section;
  int           m_str_offset;           // offset of the string table within the file
  int           m_str_size;             // size of the string table
  data_cache_t  m_shdr_cache;           // cache for the section header table
  data_cache_t  m_data_cache;           // cache for general purpose section data

  public:
  unsigned short int e_type;            // object file type
  unsigned short int e_machine;         // architecture
  unsigned int       e_version;         // object file version

  int           e_entry;                // entry point virtual address
  int           e_phoff;                // program header table file offset
  int           e_shoff;                // section header table file offset
  unsigned int  e_flags;                // processor-specific flags
  int           e_ehsize;               // ELF header size in bytes
  int           e_phentsize;            // program header table entry size
  int           e_phnum;                // program header table entry count
  int           e_shentsize;            // section header table entry size
  int           e_shnum;                // section header table entry count
  int           e_shstrndx;             // section header string table index

  private:
          bool  ids_str_load(int) noexcept;
          bool  ids_str_get(int, int, const char*&, int&) noexcept;

  public:
          elf32_bfd_t(bin_bfd_t&) noexcept;
          elf32_bfd_t(const elf32_bfd_t&) noexcept = delete;
          elf32_bfd_t(elf32_bfd_t&&) noexcept = delete;
          ~elf32_bfd_t() noexcept;

          unsigned int get_object_type() const noexcept;
          bool    has_object_type(unsigned int) const noexcept;
          unsigned int get_machine_type() const noexcept;
          bool    has_machine_type(unsigned int) const noexcept;

          bool    read_section_info(Elf32_Shdr&, int) noexcept;
          bool    read_section_name(Elf32_Shdr&, const char*&, int&) noexcept;
          bool    read_section_data(Elf32_Shdr&, std::uint8_t*&, int) noexcept;
          bool    read_section_at(Elf32_Shdr&, int, std::uint8_t*&, int) noexcept;
          bool    copy_section_data(Elf32_Shdr&, std::uint8_t*, int) noexcept;
          bool    copy_section_at(Elf32_Shdr&, int, std::uint8_t*, int) noexcept;
          int     get_section_count() noexcept;
          bool    read_symbol_info(Elf32_Sym&, Elf32_Shdr&, int) noexcept;
          bool    read_symbol_name(Elf32_Sym&, Elf32_Shdr&, const char*&, int&) noexcept;
          int     get_symbol_count(Elf32_Shdr&) noexcept;
          bool    read_rel_info(Elf32_Rel&, Elf32_Shdr&, int) noexcept;
          int     get_rel_count(Elf32_Shdr&) noexcept;
          bool    read_rela_info(Elf32_Rela&, Elf32_Shdr&, int) noexcept;
          int     get_rela_count(Elf32_Shdr&) noexcept;

          bool    is_valid() const noexcept;
    
                  operator bool() const noexcept;
          elf32_bfd_t&  operator=(const elf32_bfd_t&) = delete;
          elf32_bfd_t&  operator=(elf32_bfd_t&&) = delete;
};

/*namespace uld*/ }
#endif
