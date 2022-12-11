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
#include "elf32.h"
#include "bin.h"
#include <log.h>
#include <elf.h>
#include <limits>

namespace uld {

static void dbg_dump_shdr() noexcept;
static void dbg_dump_shdr(Elf32_Shdr& shdr, int = -1, bool = true, bool = true) noexcept;

      elf32_bfd_t::elf32_bfd_t(bin_bfd_t& source) noexcept:
      bin_bfd_t(source),
      m_str_cache(m_file_ptr),
      m_str_section(0),
      m_str_offset(0),
      m_str_size(0),
      m_shdr_cache(m_file_ptr),
      m_data_cache(m_file_ptr)
{
      // read the ELF header
      if(source.is_elf()) {
          Elf32_Ehdr   l_head_info;
          int          l_read_size = 0;
          int          l_read_offset = m_data_cache.seek(m_file_offset + EI_NIDENT);
          if(l_read_offset == m_file_offset + EI_NIDENT) {
              if(m_lsb_bit) {
                  l_read_size = m_data_cache.lsb_get(
                      l_head_info.e_type,
                      l_head_info.e_machine,
                      l_head_info.e_version,
                      l_head_info.e_entry,
                      l_head_info.e_phoff,
                      l_head_info.e_shoff,
                      l_head_info.e_flags,
                      l_head_info.e_ehsize,
                      l_head_info.e_phentsize,
                      l_head_info.e_phnum,
                      l_head_info.e_shentsize,
                      l_head_info.e_shnum,
                      l_head_info.e_shstrndx
                  );
              } else
              if(m_msb_bit) {
                  l_read_size = m_data_cache.msb_get(
                      l_head_info.e_type,
                      l_head_info.e_machine,
                      l_head_info.e_version,
                      l_head_info.e_entry,
                      l_head_info.e_phoff,
                      l_head_info.e_shoff,
                      l_head_info.e_flags,
                      l_head_info.e_ehsize,
                      l_head_info.e_phentsize,
                      l_head_info.e_phnum,
                      l_head_info.e_shentsize,
                      l_head_info.e_shnum,
                      l_head_info.e_shstrndx
                  );
              }
              if(l_read_size == sizeof(Elf32_Ehdr) - EI_NIDENT) {
                  e_type = l_head_info.e_type;
                  e_machine = l_head_info.e_machine;
                  e_version = l_head_info.e_version;
                  e_entry = l_head_info.e_entry;
                  e_phoff = l_head_info.e_phoff;
                  e_shoff = l_head_info.e_shoff;
                  e_flags = l_head_info.e_flags;
                  e_ehsize = l_head_info.e_ehsize;
                  e_phentsize = l_head_info.e_phentsize;
                  e_phnum = l_head_info.e_phnum;
                  e_shentsize = l_head_info.e_shentsize;
                  e_shnum = l_head_info.e_shnum;
                  e_shstrndx = l_head_info.e_shstrndx;
              }
          }
      }
}

      elf32_bfd_t::~elf32_bfd_t() noexcept
{
}

/* idc_str_load()
   find and load the details of the string section pointed to by the given index
*/
bool  elf32_bfd_t::ids_str_load(int section) noexcept
{
      if((section >= 0) &&
          (section <= std::numeric_limits<short int>::max())) {
          Elf32_Shdr l_shdr_info;
          if(bool
              l_shdr_found = read_section_info(l_shdr_info, section);
              l_shdr_found == true) {
              m_str_section = section;
              m_str_offset = l_shdr_info.sh_offset;
              m_str_size = l_shdr_info.sh_size;
              return true;
          }
      }
      return false;
}

/* idc_str_get
   get a pointer to a string in the string section specified by its index
*/
bool  elf32_bfd_t::ids_str_get(int section, int offset, const char*& str_ptr, int& str_length) noexcept
{
      if(offset >= 0) {
          bool l_load_success = true;
          if((m_str_offset == 0) ||
              (m_str_section == 0) ||
              (m_str_section != section)) {
              l_load_success = ids_str_load(section);
          }
          if(l_load_success) {
              if(offset < m_str_size) {
                  int  l_file_offset = m_file_offset + m_str_offset + offset;
                  int  l_seek_offset = m_str_cache.seek(l_file_offset);
                  if(l_seek_offset == l_file_offset) {
                      int  l_char;
                      int  l_string_offset;
                      int  l_string_length = 0;
                      int  l_string_capacity = std::numeric_limits<short int>::max();
                      bool l_string_valid = false;
                      // have an offset to the string, but don't know how much of the file to read to make it fit into memory;
                      // read it in char by char
                      m_str_cache.acquire(l_string_offset);
                      while(l_string_length < l_string_capacity) {
                          l_char = m_str_cache.raw_get();
                          if(l_char <= ' ') {
                              if(l_char != EOF) {
                                  l_string_valid = true;
                              }
                              break;
                          } else
                              l_string_length++;
                      }
                      m_str_cache.release(l_string_offset);
                      if(l_string_valid) {
                          str_ptr    = m_str_cache.at(l_string_offset);
                          str_length = l_string_length;
                          return true;
                      }
                  }
              }
          }
      }
      return false;
}

bool  elf32_bfd_t::read_section_info(Elf32_Shdr& shdr, int index) noexcept
{
      if((index >= 0) &&
          (index < e_shnum)) {
          int l_ent_size    = e_shentsize;
          int l_file_offset = m_file_offset + e_shoff + index * l_ent_size;
          int l_seek_offset = m_shdr_cache.seek(l_file_offset);
          int l_read_size = 0;
          if(l_seek_offset == l_file_offset) {
              if(m_lsb_bit) {
                  l_read_size = m_shdr_cache.lsb_get(
                      shdr.sh_name,
                      shdr.sh_type,
                      shdr.sh_flags,
                      shdr.sh_addr,
                      shdr.sh_offset,
                      shdr.sh_size,
                      shdr.sh_link,
                      shdr.sh_info,
                      shdr.sh_addralign,
                      shdr.sh_entsize
                  );
              } else
              if(m_msb_bit) {
                  l_read_size = m_shdr_cache.msb_get(
                      shdr.sh_name,
                      shdr.sh_type,
                      shdr.sh_flags,
                      shdr.sh_addr,
                      shdr.sh_offset,
                      shdr.sh_size,
                      shdr.sh_link,
                      shdr.sh_info,
                      shdr.sh_addralign,
                      shdr.sh_entsize
                  );
              }
              return l_read_size >= static_cast<int>(sizeof(Elf32_Shdr));
          }
      }
      return false;
}

bool  elf32_bfd_t::read_section_name(Elf32_Shdr& shdr, const char*& name_ptr, int& name_length) noexcept
{
      name_ptr = nullptr;
      name_length = 0;
      if(shdr.sh_name >= 0) {
          return ids_str_get(e_shstrndx, shdr.sh_name, name_ptr, name_length);
      }
      return false;
}

/* read_section_data()
   load full section data into the local data buffer and return a pointer to it
*/
bool  elf32_bfd_t::read_section_data(Elf32_Shdr& shdr, std::uint8_t*& data, int size) noexcept
{
      return read_section_at(shdr, 0, data, shdr.sh_size);
}

/* read_section_at()
   load section data at the specified offset and with the specified size into the local data buffer and return a pointer to it
*/
bool  elf32_bfd_t::read_section_at(Elf32_Shdr& shdr, int spos, std::uint8_t*& data, int size) noexcept
{
      return false;
}

/* copy_section_data()
   copy full section data into the buffer pointed to by `data`
*/
bool  elf32_bfd_t::copy_section_data(Elf32_Shdr& shdr, std::uint8_t* data, int size) noexcept
{
      return copy_section_at(shdr, 0, data, shdr.sh_size);
}

/* copy_section_at()
   copy section data at the specified offset and with the specified size into the data buffer pointed to by `data`.
*/
bool  elf32_bfd_t::copy_section_at(Elf32_Shdr& shdr, int spos, std::uint8_t* data, int size) noexcept
{
      int l_read_offset = m_file_offset + shdr.sh_offset + spos;
      int l_tail_offset = l_read_offset + size;
      if(l_tail_offset > l_read_offset) {
          if(l_tail_offset <= static_cast<int>(shdr.sh_offset) + static_cast<int>(shdr.sh_size)) {
              unsigned int l_read_size;
              if(FRESULT
                  l_rc = f_lseek(m_file_ptr, l_read_offset);
                  l_rc == FR_OK) {
                  if(FRESULT
                      l_rc = f_read(m_file_ptr, data, size, std::addressof(l_read_size));
                      l_rc == FR_OK) {
                      l_read_offset += static_cast<int>(l_read_size);
                  }
              }
          }
      }
      return l_read_offset >= l_tail_offset;
}

int   elf32_bfd_t::get_section_count() noexcept
{
      return e_shnum;
}

bool  elf32_bfd_t::read_symbol_info(Elf32_Sym& sym, Elf32_Shdr& shdr, int index) noexcept
{
      if(index >= 0) {
          if(shdr.sh_entsize > 0) {
              int l_sym_offset = index * shdr.sh_entsize;
              if(l_sym_offset < static_cast<int>(shdr.sh_size)) {
                  int l_file_offset = m_file_offset + shdr.sh_offset + l_sym_offset;
                  int l_seek_offset = m_data_cache.seek(l_file_offset);
                  int l_read_size = 0;
                  if(l_seek_offset == l_file_offset) {
                      if(m_lsb_bit) {
                          l_read_size = m_data_cache.lsb_get(
                              sym.st_name,
                              sym.st_value,
                              sym.st_size,
                              sym.st_info,
                              sym.st_other,
                              sym.st_shndx
                          );
                      } else
                      if(m_msb_bit) {
                          l_read_size = m_data_cache.msb_get(
                              sym.st_name,
                              sym.st_value,
                              sym.st_size,
                              sym.st_info,
                              sym.st_other,
                              sym.st_shndx
                          );
                      }
                      return l_read_size >= static_cast<int>(sizeof(Elf32_Sym));
                  }
              }
          }
      }
      return false;
}

bool  elf32_bfd_t::read_symbol_name(Elf32_Sym& sym, Elf32_Shdr& shdr, const char*& name_ptr, int& name_length) noexcept
{
      name_ptr = nullptr;
      name_length = 0;
      return ids_str_get(shdr.sh_link, sym.st_name, name_ptr, name_length);
}

int   elf32_bfd_t::get_symbol_count(Elf32_Shdr& shdr) noexcept
{
      if(shdr.sh_type == SHT_SYMTAB) {
          if(shdr.sh_size > 0) {
              if(shdr.sh_entsize > 0) {
                  if(shdr.sh_size % shdr.sh_entsize) {
                      printdbg(
                          "Symbol table consistency issue, symbol count may be incorrect.",
                          __FILE__,
                          __LINE__
                      );
                  }
                  return shdr.sh_size / shdr.sh_entsize;
              }
          }
      }
      return 0;
}

bool  elf32_bfd_t::read_rel_info(Elf32_Rel& rel, Elf32_Shdr& shdr, int index) noexcept
{
      if(index >= 0) {
          if(shdr.sh_entsize > 0) {
              int l_rel_offset = index * shdr.sh_entsize;
              if(l_rel_offset < static_cast<int>(shdr.sh_size)) {
                  int l_file_offset = m_file_offset + shdr.sh_offset + l_rel_offset;
                  int l_seek_offset = m_data_cache.seek(l_file_offset);
                  int l_read_size = 0;
                  if(l_seek_offset == l_file_offset) {
                      if(m_lsb_bit) {
                          l_read_size = m_data_cache.lsb_get(
                              rel.r_offset,
                              rel.r_info
                          );
                      } else
                      if(m_msb_bit) {
                          l_read_size = m_data_cache.msb_get(
                              rel.r_offset,
                              rel.r_info
                          );
                      }
                      return l_read_size >= static_cast<int>(sizeof(Elf32_Rel));
                  }
              }
          }
      }
      return false;
}

int   elf32_bfd_t::get_rel_count(Elf32_Shdr& shdr) noexcept
{
      if(shdr.sh_type == SHT_REL) {
          if(shdr.sh_size > 0) {
              if(shdr.sh_entsize > 0) {
                  if(shdr.sh_size % shdr.sh_entsize) {
                      printdbg(
                          "REL relocation table consistency issue, entry count may be incorrect.",
                          __FILE__,
                          __LINE__
                      );
                  }
                  return shdr.sh_size / shdr.sh_entsize;
              }
          }
      }
      return 0;
}

bool  elf32_bfd_t::read_rela_info(Elf32_Rela& rela, Elf32_Shdr& shdr, int index) noexcept
{
      if(index >= 0) {
          if(shdr.sh_entsize > 0) {
              int l_rela_offset = index * shdr.sh_entsize;
              if(l_rela_offset < static_cast<int>(shdr.sh_size)) {
                  int l_file_offset = m_file_offset + shdr.sh_offset + l_rela_offset;
                  int l_seek_offset = m_data_cache.seek(l_file_offset);
                  int l_read_size = 0;
                  if(l_seek_offset == l_file_offset) {
                      if(m_lsb_bit) {
                          l_read_size = m_data_cache.lsb_get(
                              rela.r_offset,
                              rela.r_info,
                              rela.r_addend
                          );
                      } else
                      if(m_msb_bit) {
                          l_read_size = m_data_cache.msb_get(
                              rela.r_offset,
                              rela.r_info,
                              rela.r_addend
                          );
                      }
                      return l_read_size >= static_cast<int>(sizeof(Elf32_Rela));
                  }
              }
          }
      }
      return false;
}

int   elf32_bfd_t::get_rela_count(Elf32_Shdr& shdr) noexcept
{
      if(shdr.sh_type == SHT_RELA) {
          if(shdr.sh_size > 0) {
              if(shdr.sh_entsize > 0) {
                  if(shdr.sh_size % shdr.sh_entsize) {
                      printdbg(
                          "REL relocation table consistency issue, entry count may be incorrect.",
                          __FILE__,
                          __LINE__
                      );
                  }
                  return shdr.sh_size / shdr.sh_entsize;
              }
          }
      }
      return 0;
}

unsigned int elf32_bfd_t::get_object_type() const noexcept
{
      return e_type;
}

bool  elf32_bfd_t::has_object_type(unsigned int object_type) const noexcept
{
      return e_type == object_type;
}

unsigned int elf32_bfd_t::get_machine_type() const noexcept
{
      return e_machine;
}

bool  elf32_bfd_t::has_machine_type(unsigned int machine_type) const noexcept
{
      return e_machine == machine_type;
}

bool  elf32_bfd_t::is_valid() const noexcept
{
      return m_elf_bit;
}

void  dbg_dump_shdr() noexcept
{
      printdbg("        sh_name\tsh_type\tsh_flgs\tsh_addr\tsh_offs\tsh_size\tsh_link\tsh_info\tsh_algn\tsh_ent \tsh_link");

}

void  dbg_dump_shdr(Elf32_Shdr& shdr, int index, bool show_header, bool show_data) noexcept
{
      char l_index_cache[16];
      if(index >= 0) {
          std::snprintf(l_index_cache, 16, "%5d:", index);
      } else
          l_index_cache[0] = 0;
      if(show_header) {
          dbg_dump_shdr();
      }
      if(show_data) {
          printdbg("%-8s%dh\t%dh\t%dh\t%dh\t%dh\t%dh\t%dh\t%dh\t%d\t%d\t%d",
              l_index_cache,
              shdr.sh_name,
              shdr.sh_type,
              shdr.sh_flags,
              shdr.sh_addr,
              shdr.sh_offset,
              shdr.sh_size,
              shdr.sh_link,
              shdr.sh_info,
              shdr.sh_addralign,
              shdr.sh_entsize,
              shdr.sh_link
          );
      }
}

/*namespace uld*/ }
