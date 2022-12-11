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
#include "target.h"
#include "image.h"
#include "error.h"
#include "bfd/elf32.h"
#include <log.h>
#include "bits/arm.h"
#include <cstring>
#include <cstdarg>
#include <elf.h>
#include <dbg.h>

static constexpr int bind_reserve_min = 32;       // how many items to initially reserve into the the binding table

namespace uld {
namespace elf32 {

      factory::factory(image* image_ptr) noexcept:
      m_image(image_ptr),
      m_target(image_ptr->get_target()),
      m_string_pool(),
      m_symbol_pool(std::addressof(m_string_pool)),
      m_shdr_count(0),
      m_shdr_have_code(false),
      m_shdr_have_data(false),
      m_shdr_have_symtab(false),
      m_shdr_have_rel(false)
{
}

      factory::~factory()
{
}

auto  factory::uld_get_local_section(int index) noexcept -> section_t*
{
      if(index > 0) {
          if(index < static_cast<int>(m_shdr_map.size())) {
              return std::addressof(m_shdr_map[index]);
          }
      }
      return nullptr;
}

auto  factory::uld_get_local_section(symbol_t* symbol_ptr, int* index) noexcept -> section_t*
{
      if(symbol_ptr != nullptr) {
          int  l_shdr_count = m_shdr_map.size();
          if(l_shdr_count > 0) {
              section_t*  l_shdr_base = std::addressof(m_shdr_map[0]);
              section_t*  l_shdr_last = l_shdr_base + l_shdr_count - 1;
              if((symbol_ptr >= l_shdr_base) &&
                  (symbol_ptr <= l_shdr_last)) {
                  section_t*  l_shdr_ptr = static_cast<section_t*>(symbol_ptr);
                  if(index != nullptr) {
                      *index = l_shdr_ptr - l_shdr_base;
                  }
                  return l_shdr_ptr;
              }
          }
      }
      if(index != nullptr) {
          *index = 0;
      }
      return nullptr;
}

auto  factory::uld_get_local_symbol(int index) noexcept -> symbol_t*
{
      if(index > 0) {
          if(index < static_cast<int>(m_symbol_map.size())) {
              return m_symbol_map[index];
          }
      }
      return nullptr;
}

auto  factory::uld_get_global_address(symbol_t* symbol_ptr) noexcept -> std::uint8_t*
{
      if(symbol_ptr != nullptr) {
          return reinterpret_cast<std::uint8_t*>(symbol_ptr) + offsetof(struct symbol_t, ra);
      }
      return m_target->get_address_base();
}

auto  factory::uld_get_symbol_address(symbol_t* symbol_ptr) noexcept -> std::uint8_t*
{
      if(symbol_ptr != nullptr) {
          // if symbol is a section we can't necessarily trust its 'ra' member: it's just the rollback address
          if((symbol_ptr->type & symbol_t::type_section) == symbol_t::type_section) {
              if(section_t*
                  l_section_ptr = uld_get_local_section(symbol_ptr);
                  l_section_ptr != nullptr) {
                  return l_section_ptr->support->get_table_ptr(l_section_ptr->offset_base);
              }
              return nullptr;
          }
          return symbol_ptr->ea;
      }
      return nullptr;
}

auto  factory::uld_get_symbol_address(symbol_t* symbol_ptr, std::int32_t offset) noexcept -> std::uint8_t*
{
      std::uint8_t* l_ea = uld_get_symbol_address(symbol_ptr);
      if(l_ea != nullptr) {
          return l_ea + offset;
      }
      return l_ea;
}

auto  factory::uld_get_virtual_address(symbol_t* symbol_ptr) noexcept -> std::uint8_t*
{
      if(symbol_ptr != nullptr) {
          // if symbol is a section we can't necessarily trust its 'ra' member: it's just the rollback address
          if((symbol_ptr->type & symbol_t::type_section) == symbol_t::type_section) {
              if(section_t*
                  l_section_ptr = uld_get_local_section(symbol_ptr);
                  l_section_ptr != nullptr) {
                  if(l_section_ptr->offset_last == l_section_ptr->offset_base) {
                      // this section has not been loaded yet
                      uld_error(
                          e_nosym,
                          "Data for section `%s` is not available at runtime.",
                          __FILE__,
                          __LINE__,
                          symbol_ptr->name
                      );
                      return nullptr;
                  }
                  return l_section_ptr->support->get_table_ptr(l_section_ptr->offset_base);
              }
              return nullptr;
          }
          return symbol_ptr->ra;
      }
      return nullptr;
}

auto  factory::uld_get_virtual_address(symbol_t* symbol_ptr, std::int32_t offset) noexcept -> std::uint8_t*
{
      std::uint8_t* l_ra = uld_get_virtual_address(symbol_ptr);
      if(l_ra != nullptr) {
          return l_ra + offset;
      }
      return l_ra;
}

auto  factory::uld_get_base_address(symbol_t*) noexcept -> std::uint8_t*
{
      return m_target->get_address_base();
}

auto  factory::uld_get_section_data(elf32_bfd_t& bi, Elf32_Shdr& shdr_info, int shdr_index, std::int32_t data_offset, std::int32_t data_size) noexcept -> std::uint8_t*
{
      section_t* l_section_ptr = uld_get_local_section(shdr_index);
      if(l_section_ptr != nullptr) {
          segment*  l_segment_ptr = l_section_ptr->support;
          if(l_segment_ptr == nullptr) {
              uld_error(
                  e_fault,
                  "Unable to load data from section `%s`: no adequate segment found.",
                  __FILE__,
                  __LINE__,
                  l_section_ptr->name
              );
              return nullptr;
          }
          std::int32_t l_offset_base = l_section_ptr->offset_base;
          std::int32_t l_offset_last = l_section_ptr->offset_last;
          std::int32_t l_extend_last = l_offset_base + data_offset + data_size;
          if(l_extend_last > l_offset_last) {
              std::int32_t  l_copy_pos;
              std::int32_t  l_copy_size;
              std::uint8_t* l_copy_ptr;
              if(l_offset_base + data_offset < l_offset_last) {
                  // we don't want this to happen because the segment allocation may cross pages and yield a non-contiguous
                  // region
                  uld_error(
                      e_access,
                      "Unable to load data from section `%s`: overlapping an existing data chunk [%.4x..%.4x].",
                      __FILE__,
                      __LINE__,
                      l_section_ptr->name,
                      data_offset,
                      l_extend_last
                  );
                  return nullptr;
              }
              l_copy_pos  = l_offset_last - l_offset_base;
              l_copy_size = l_extend_last - l_offset_last;
              l_copy_ptr  = l_segment_ptr->raw_get(l_copy_size);
              if(l_copy_ptr == nullptr) {
                  uld_error(
                      e_memory,
                      "Unable to load data from section `%s`: memory allocation error.",
                      __FILE__,
                      __LINE__,
                      l_section_ptr->name
                  );
                  return nullptr;
              }
              if(shdr_info.sh_type == SHT_NOBITS) {
                  std::memset(l_copy_ptr, 0, l_copy_size);
              } else
              if(shdr_info.sh_type != SHT_NULL) {
                  if(bool
                      l_fetch_data_success = bi.copy_section_at(shdr_info, l_copy_pos, l_copy_ptr, l_copy_size);
                      l_fetch_data_success == false) {
                      uld_error(
                          e_access,
                          "Unable to load data from section `%s`: data acquisition error.",
                          __FILE__,
                          __LINE__,
                          l_section_ptr->name
                      );
                      return nullptr;
                  }
              }
              l_section_ptr->offset_last = l_section_ptr->support->get_table_offset();
          }
          return l_section_ptr->support->get_table_ptr(l_offset_base + data_offset);
      }
      return nullptr;
}

auto  factory::uld_get_section_data(elf32_bfd_t& bi, int shdr_index, std::int32_t data_offset, std::int32_t data_size) noexcept -> std::uint8_t*
{
      int  l_section_index = shdr_index;
      auto l_section_ptr = uld_get_local_section(l_section_index);
      if(l_section_ptr != nullptr) {
          Elf32_Shdr  l_section_info;
          if(bool
              l_fetch_shdr_success = bi.read_section_info(l_section_info, l_section_index);
              l_fetch_shdr_success == true) {
              return uld_get_section_data(bi, l_section_info, l_section_index, data_offset, data_size);
          }
      }
      return nullptr;
}

bool  factory::uld_load_section(elf32_bfd_t& bi, Elf32_Shdr& shdr_info, int shdr_index) noexcept
{
      return uld_load_section(bi, shdr_info, shdr_index, 0, shdr_info.sh_size);
}

bool  factory::uld_load_section(elf32_bfd_t& bi, Elf32_Shdr& shdr_info, int shdr_index, std::int32_t data_offset, std::int32_t data_size) noexcept
{
      auto l_section_data = uld_get_section_data(bi, shdr_info, shdr_index, data_offset, data_size);
      if(l_section_data != nullptr) {
          return true;
      }
      return false;
}

bool  factory::uld_load_symbol(elf32_bfd_t& bi, Elf32_Shdr& shdr_info, Elf32_Sym& sym_info, int sym_index) noexcept
{
      const char*  l_sym_name;
      int          l_sym_name_length;
      unsigned int l_sym_type = ELF32_ST_TYPE(sym_info.st_info);
      unsigned int l_sym_bind = ELF32_ST_BIND(sym_info.st_info);
      if(bool
          l_fetch_name_success = bi.read_symbol_name(sym_info, shdr_info, l_sym_name, l_sym_name_length);
          l_fetch_name_success == false) {
          uld_error(
              e_access,
              "Read error: Failed to fetch symbol name.",
              __FILE__,
              __LINE__
          );
          return false;
      }
      m_symbol_map[sym_index] = nullptr;
      if(l_sym_type == STT_NOTYPE) {
          if(sym_info.st_shndx == SHN_UNDEF) {
              // found an undefined symbol: bind if found defined within the image, save if new, drop otherwise
              if(sym_info.st_name != 0) {
                  symbol_t* l_sym_ptr = m_image->find_symbol(l_sym_name, symbol_t::bind_any);
                  if(l_sym_ptr == nullptr) {
                      l_sym_ptr = m_symbol_pool.make_symbol(
                          l_sym_name,
                          l_sym_name_length,
                          l_sym_type,
                          l_sym_bind | symbol_t::bit_export
                      );
                  }
                  m_symbol_map[sym_index] = l_sym_ptr;
              }
          } else
          if(sym_info.st_shndx < m_shdr_count) {
              // found a local data chunk (i.e. a function-static buffer): store into the section it indicates
              std::int32_t  l_sym_size = sym_info.st_size;
              if(l_sym_size) {
                  symbol_t* l_sym_ptr = m_symbol_pool.make_symbol(
                      l_sym_name,
                      l_sym_name_length,
                      l_sym_type,
                      l_sym_bind
                  );
                  if(l_sym_ptr == nullptr) {
                      uld_error(
                          e_nodef,
                          "Failed to define symbol `%s`: out of memory.",
                          __FILE__,
                          __LINE__,
                          l_sym_name
                      );
                      return false;
                  }
                  int           l_sym_shndx = sym_info.st_shndx;
                  std::int32_t  l_sym_offset = sym_info.st_value;
                  std::uint8_t* l_sym_data  = uld_get_section_data(bi, l_sym_shndx, l_sym_offset, l_sym_size);
                  if(l_sym_data == nullptr) {
                      uld_error(
                          e_nodef,
                          "Failed to define symbol `%s`.",
                          __FILE__,
                          __LINE__,
                          l_sym_name
                      );
                      return false;
                  }
                  l_sym_ptr->ea = l_sym_data;
                  l_sym_ptr->ra = l_sym_data;
                  l_sym_ptr->size = l_sym_size;
                  if constexpr (is_debug) {
                      printf(
                          "(i) Stored symbol `%s` at effective address %p\n",
                          l_sym_ptr->name,
                          l_sym_ptr->ea
                      );
                      dbg_dump_hex(l_sym_ptr->ea, l_sym_ptr->size, true);
                  }
                  m_symbol_map[sym_index] = l_sym_ptr;
              }
          } else
              uld_error(
                  e_fault,
                  "Unable to map symbol `%s` to section index %.4x: invalid segment.",
                  __FILE__,
                  __LINE__,
                  l_sym_name,
                  sym_info.st_shndx
              );
      } else
      if(l_sym_type == STT_SECTION) {
          // have a section symbol: bind to one of ours or drop
          // NOTE: relevant sections are loaded during the 'prefetch()' phase; makes no functional difference, but the section
          // headers are directly available there, while here we'd have to search for them and reseek the file.
          if(sym_info.st_shndx > SHN_UNDEF) {
              if(sym_info.st_shndx < m_shdr_count) {
                  m_symbol_map[sym_index] = uld_get_local_section(sym_info.st_shndx);
              }
          }
      } else
      if((l_sym_type == STT_FUNC) ||
          (l_sym_type == STT_OBJECT)) {
          symbol_t* l_sym_ptr = m_image->find_symbol(l_sym_name, symbol_t::bind_any);
          if(l_sym_ptr == nullptr) {
              l_sym_ptr = m_symbol_pool.make_symbol(
                  l_sym_name,
                  l_sym_name_length,
                  l_sym_type,
                  l_sym_bind
              );
              if(l_sym_ptr == nullptr) {
                  uld_error(
                      e_nodef,
                      "Failed to define symbol `%s`: out of memory.",
                      __FILE__,
                      __LINE__,
                      l_sym_name
                  );
                  return false;
              }
              if((l_sym_bind == STB_WEAK) ||
                  (l_sym_bind == STB_GLOBAL)) {
                  l_sym_ptr->flags |= symbol_t::bit_export;
              }
          } else
          if(l_sym_ptr != nullptr) {
              // if a symbol that goes by the same name exists in the table, it should be a weak or an undefined one
              if(l_sym_bind != STB_WEAK) {
                  if(l_sym_ptr->ra != nullptr) {
                      uld_error(
                          e_redef,
                          "Redefinition of symbol `%s`.",
                          __FILE__,
                          __LINE__,
                          l_sym_name
                      );
                      return false;
                  }
              }
              l_sym_ptr->flags |= symbol_t::bit_define;
          }
          // load symbol data
          if(sym_info.st_shndx == SHN_ABS) {
              uld_error(
                  e_fault,
                  "Unable to map symbol `%s` to an ABS segment.",
                  __FILE__,
                  __LINE__,
                  l_sym_name
              );
              return false;
          } else
          if(sym_info.st_shndx == SHN_COMMON) {
              uld_error(
                  e_fault,
                  "Unable to map symbol `%s` to a COMMON segment.",
                  __FILE__,
                  __LINE__,
                  l_sym_name
              );
              return false;
          } else
          if(sym_info.st_shndx < m_shdr_count) {
              int           l_sym_shndx        = sym_info.st_shndx;
              std::int32_t  l_sym_value        = sym_info.st_value;
              auto          l_sym_offset_mask  = m_target->get_vle_mask();
              std::int32_t  l_sym_offset       = l_sym_value & l_sym_offset_mask;
              auto          l_vle_bit          = m_target->get_vle_bit();
              std::int32_t  l_sym_size         = sym_info.st_size;
              std::uint8_t* l_sym_data         = uld_get_section_data(bi, l_sym_shndx, l_sym_offset, l_sym_size);
              if(l_sym_data == nullptr) {
                  uld_error(
                      e_nodef,
                      "Failed to define symbol `%s`.",
                      __FILE__,
                      __LINE__,
                      l_sym_name
                  );
                  return false;
              }
              l_sym_ptr->ea = l_sym_data;
              l_sym_ptr->ra = l_sym_data;
              l_sym_ptr->size = l_sym_size;
              // VLEs (like thumb) add a +1 to the effective function address; set the virtual address to that
              // for function symbols
              if(l_sym_type == STT_FUNC) {
                  l_sym_ptr->ra += l_vle_bit;
              }
              if constexpr (is_debug) {
                  printf(
                      "(i) Stored symbol `%s` at effective address %p\n",
                      l_sym_ptr->name,
                      l_sym_ptr->ea
                  );
                  dbg_dump_hex(l_sym_ptr->ea, l_sym_ptr->size, true);
              }
              // generate a binding entry, in order for the factory to be able to detect and apply
              // relocations against the contents of this symbol
              binding_t& l_bind_info = m_bind_list.emplace_back();
              l_bind_info.symbol_index = sym_index;
              l_bind_info.source_index = sym_info.st_shndx;
              l_bind_info.source_offset_base = l_sym_offset;
              l_bind_info.source_offset_last = l_sym_offset + l_sym_size;
              m_symbol_map[sym_index] = l_sym_ptr;
          } else
              uld_error(
                  e_fault,
                  "Unable to map symbol `%s` to section index %.4x: invalid segment.",
                  __FILE__,
                  __LINE__,
                  l_sym_name,
                  sym_info.st_shndx
              );
      }
      return true;
}

bool  factory::uld_resolve_rel(elf32_bfd_t& bi, Elf32_Shdr& shdr_info, Elf32_Rel& rel_info, std::int32_t rel_addend) noexcept
{
      int  l_rel_sym = ELF32_R_SYM(rel_info.r_info);
      int  l_rel_type = ELF32_R_TYPE(rel_info.r_info);
      // run through the bind list, check the current relocation entry applies to it and resolve it if so
      for(binding_t& l_bind_info : m_bind_list) {
          if((l_bind_info.source_index == static_cast<int>(shdr_info.sh_info)) &&
              (l_bind_info.source_offset_base <= static_cast<std::int32_t>(rel_info.r_offset)) &&
              (l_bind_info.source_offset_last > static_cast<std::int32_t>(rel_info.r_offset))) {
              // load and check the symbol over which the relocation applies
              symbol_t*  l_dst_sym = uld_get_local_symbol(l_bind_info.symbol_index);
              if((l_dst_sym == nullptr) ||
                  (l_dst_sym->ea == nullptr)) {
                  uld_error(
                      e_nosym,
                      "Unable to perform relocation: destination symbol index `%d` unavailable.",
                      __FILE__,
                      __LINE__,
                      l_bind_info.symbol_index
                  );
                  return false;
              }
              // load and check the symbol being invoked in the relocation;
              symbol_t*  l_src_sym = uld_get_local_symbol(l_rel_sym);
              if(l_rel_sym > 0) {
                  if(l_src_sym == nullptr) {
                      uld_error(
                          e_nosym,
                          "Unable to perform relocation: source symbol index `%d` unavailable.",
                          __FILE__,
                          __LINE__,
                          l_rel_sym
                      );
                      return false;
                  }
              }
              // relocation variables, as named on the "ELF for the Arm Architecture" ABI doc, for ease of implementation
              std::uint8_t*  s = 0; // symbol address
              std::uint8_t*  b_s;   // base address of the segment defining the symbol 's' (fixed to m_target->get_address_base())
              std::uint8_t*  got_s; // address of the GOT entry pertaining to the symbol 's'
              std::int32_t   a;     // addend
              // address where the relocation applies
              std::uint8_t*  p = uld_get_symbol_address(l_dst_sym, rel_info.r_offset - l_bind_info.source_offset_base);
              // return pointer, 16 bit value(s)
              std::uint16_t* r = reinterpret_cast<std::uint16_t*>(p);

              if constexpr (os::is_lsb) {
                  switch(l_rel_type) {
                    case R_ARM_NONE:
                        break;
                    case R_ARM_ABS32:
                        b_arm_get32(p, a);
                        s = uld_get_virtual_address(l_src_sym);
                        b_arm_set32(p, reinterpret_cast<std::int32_t>(s + a));
                        break;
                    case R_ARM_ABS32_NOI:
                        b_arm_get32(p, a);
                        s = uld_get_virtual_address(l_src_sym);
                        b_arm_set32(p, reinterpret_cast<std::int32_t>(s + a));
                        break;
                    case R_ARM_REL32:
                        b_arm_get32(p, a);
                        s = uld_get_virtual_address(l_src_sym);
                        b_arm_set32(p, reinterpret_cast<std::int32_t>(s + a) - reinterpret_cast<std::int32_t>(p));
                        break;
                    case R_ARM_REL32_NOI:
                        b_arm_get32(p, a);
                        s = uld_get_virtual_address(l_src_sym);
                        b_arm_set32(p, reinterpret_cast<std::int32_t>(s + a) - reinterpret_cast<std::int32_t>(p));
                        break;
                    // case R_ARM_PC13:        //a.k.a. R_ARM_LDR_PC_G0
                    //     break;
                    case R_ARM_SBREL32:
                        b_arm_get32(p, a);
                        s = uld_get_virtual_address(l_src_sym);
                        b_s = uld_get_base_address(l_src_sym);
                        b_arm_set32(p, reinterpret_cast<std::int32_t>(s + a) - reinterpret_cast<std::int32_t>(b_s));
                        break;
                    case R_ARM_PREL31:
                        b_arm_get30(p, a);
                        s = uld_get_virtual_address(l_src_sym);
                        b_arm_set30(p, reinterpret_cast<std::int32_t>(s + a) - reinterpret_cast<std::int32_t>(p));
                        break;
                    case R_ARM_ABS16:
                        b_arm_get16(r, a);
                        s = uld_get_virtual_address(l_src_sym);
                        if(b_can_reach(p, s + a, 16) == false) {
                            uld_error(
                                e_noreach,
                                "Address %p for the relocation R_ARM_ABS16:%p is not reachable.",
                                __FILE__,
                                __LINE__,
                                s + a,
                                p
                            );
                            return false;
                        }
                        b_arm_set16(r, reinterpret_cast<std::int32_t>(s + a));
                        break;
                    case R_ARM_ABS12:
                        b_arm_get12(r, a);
                        s = uld_get_virtual_address(l_src_sym);
                        if(b_can_reach(p, s + a, 12) == false) {
                            uld_error(
                                e_noreach,
                                "Address %p for the relocation R_ARM_ABS12:%p is not reachable.",
                                __FILE__,
                                __LINE__,
                                s + a,
                                p
                            );
                            return false;
                        }
                        b_arm_set12(r, reinterpret_cast<std::int32_t>(s + a));
                        break;
                    case R_ARM_ABS8:
                        b_arm_get8(r, a);
                        s = uld_get_virtual_address(l_src_sym);
                        if(b_can_reach(p, s + a, 8) == false) {
                            uld_error(
                                e_noreach,
                                "Address %p for the relocation R_ARM_ABS8:%p is not reachable.",
                                __FILE__,
                                __LINE__,
                                s + a,
                                p
                            );
                            return false;
                        }
                        b_arm_set8(r, reinterpret_cast<std::int32_t>(s + a));
                        break;

                    case R_ARM_CALL:
                        b_arm_getbl26(r, a);
                        s = uld_get_virtual_address(l_src_sym);
                        if(b_can_reach(p, s + a, 26) == false) {
                            uld_error(
                                e_noreach,
                                "Address %p for the relocation R_ARM_CALL:%p is not reachable.",
                                __FILE__,
                                __LINE__,
                                s + a,
                                p
                            );
                            return false;
                        }
                        b_arm_setbl26(r, reinterpret_cast<std::int32_t>(s + a) - reinterpret_cast<std::int32_t>(p));
                        break;
                    case R_ARM_JUMP24:
                        b_arm_getbl26(r, a);
                        s = uld_get_virtual_address(l_src_sym);
                        if(b_can_reach(p, s + a, 26) == false) {
                            uld_error(
                                e_noreach,
                                "Address %p for the relocation R_ARM_CALL:%p is not reachable.",
                                __FILE__,
                                __LINE__,
                                s + a,
                                p
                            );
                            return false;
                        }
                        b_arm_setbl26(r, s + a - p);
                        break;
                    case R_ARM_MOVW_ABS_NC:
                        b_arm_get16(r, a);
                        s = uld_get_virtual_address(l_src_sym);
                        b_arm_set16(r, reinterpret_cast<std::int32_t>(s + a));
                        break;
                    case R_ARM_MOVT_ABS:
                        b_arm_get16(r, a);
                        s = uld_get_virtual_address(l_src_sym);
                        b_arm_set16(r, reinterpret_cast<std::int32_t>(s + a) >> 16);
                        break;
                    case R_ARM_MOVW_PREL_NC:
                        b_arm_get16(r, a);
                        s = uld_get_virtual_address(l_src_sym);
                        b_arm_set16(r, reinterpret_cast<std::int32_t>(s + a) - reinterpret_cast<std::int32_t>(p));
                        break;
                    case R_ARM_MOVT_PREL:
                        b_arm_get16(r, a);
                        s = uld_get_virtual_address(l_src_sym);
                        b_arm_set16(r, (reinterpret_cast<std::int32_t>(s + a) - reinterpret_cast<std::int32_t>(p)) >> 16);
                        break;
                    case R_ARM_ALU_PC_G0_NC:
                        // abs(x) & G0
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_ALU_PC_G0:
                        // abs(x) & G0
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_ALU_PC_G1_NC:
                        // abs(x) & G1
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_ALU_PC_G1:
                        // abs(x) & G1
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_ALU_PC_G2:
                        // abs(x) & G2
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_LDR_PC_G1:
                        // abs(x) & G1(LDR)
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_LDR_PC_G2:
                        // abs(x) & G2(LDR)
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_LDRS_PC_G0:
                        // abs(x) & G0(LDRS)
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_LDRS_PC_G1:
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_LDRS_PC_G2:
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_LDC_PC_G0:
                        // abs(x) & G0(LDC)
                        // ldc stc
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_LDC_PC_G1:
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_LDC_PC_G2:
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_ALU_SB_G0_NC:
                        // add sub
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_ALU_SB_G0:
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_ALU_SB_G1_NC:
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_ALU_SB_G1:
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_ALU_SB_G2:
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_LDR_SB_G0:
                        // ldr str ldrb strb
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_LDR_SB_G1:
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_LDR_SB_G2:
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_LDRS_SB_G0:
                        // ldrd strd ldrh strh ldrsh ldrsb
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_LDRS_SB_G1:
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_LDRS_SB_G2:
                        uld_error(
                            1,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_LDC_SB_G0:
                        // ldc, stc
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;   
                    case R_ARM_LDC_SB_G1:
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_LDC_SB_G2:
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_MOVW_BREL_NC:
                        b_arm_get16(r, a);
                        s = uld_get_virtual_address(l_src_sym);
                        b_s = uld_get_base_address(l_src_sym);
                        b_arm_set16(r, reinterpret_cast<std::int32_t>(s + a) - reinterpret_cast<std::int32_t>(b_s));
                        break;
                    case R_ARM_MOVT_BREL:
                        b_arm_get16(r, a);
                        s = uld_get_virtual_address(l_src_sym);
                        b_s = uld_get_base_address(l_src_sym);
                        b_arm_set16(r, (reinterpret_cast<std::int32_t>(s + a) - reinterpret_cast<std::int32_t>(b_s)) >> 16);
                        break;
                    case R_ARM_MOVW_BREL:
                        b_arm_get16(r, a);
                        s = uld_get_virtual_address(l_src_sym);
                        b_s = uld_get_base_address(l_src_sym);
                        b_arm_set16(r, reinterpret_cast<std::int32_t>(s + a) - reinterpret_cast<std::int32_t>(b_s));
                        break;
                    case R_ARM_GOTOFF12:
                        // abs(x) & 0x0fff
                        break;
                    // case R_ARM_TLS_LDO12:
                    //     // abs(x) & 0x0fff
                    //     break;
                    // case R_ARM_TLS_LE12:
                    //     // abs(x) & 0x0fff
                    //     break;
                    // case R_ARM_TLS_IE12GP:
                    //     // abs(x) & 0x0fff
                    //     break;

                    case R_ARM_THM_ABS5:
                        // x & 0x7c ldr(1) ldr (imm/thumb)
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_THM_PC8:
                        // x & 0x3fc ldr(2) ldr (literal) add(5)/adr 
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_THM_JUMP6:
                        // x & 0x7e cbz cbnz
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_THM_PC11:  // a.k.a. R_ARM_THM_JUMP11
                        // x & 0xffe b(2) b
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_THM_PC9:   // a.k.a. R_ARM_THM_JUMP8
                        // x & 0x1fe b(1) b<cond>
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    // case R_ARM_THM_ALU_ABS_G0_NC:
                    //     break;
                    // case R_ARM_THM_ALU_ABS_G1_NC;
                    //     break;
                    // case R_ARM_THM_ALU_ABS_G2_NC:
                    //     break;
                    // case R_ARM_THM_ALU_ABS_G3:
                    //     break

                    case R_ARM_THM_PC22:    //a.k.a. R_ARM_THM_CALL
                        b_armt_getbl22(r, a);
                        s = uld_get_virtual_address(l_src_sym);
                        if(b_can_reach(p, s + a, 22) == false) {
                            uld_error(
                                e_noreach,
                                "Address %p for the relocation R_ARM_THM_PC22:%p is not reachable.",
                                __FILE__,
                                __LINE__,
                                s + a,
                                p
                            );
                            return false;
                        }
                        b_armt_setbl22(r, reinterpret_cast<std::int32_t>(s + a) - reinterpret_cast<std::int32_t>(p));
                        break;
                    case R_ARM_THM_JUMP24:
                        // 0x01fffffe
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_THM_MOVW_ABS_NC:
                        // 0xffff
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_THM_MOVT_ABS:
                        // 0xffff0000
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_THM_MOVW_PREL_NC:
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_THM_MOVT_PREL:
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_THM_JUMP19:
                        // 0x001ffffe
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_THM_ALU_PREL_11_0:
                        // 0x00000fff
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_THM_PC12:
                        // 0x00000fff
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_THM_MOVW_BREL_NC:
                        // 0x0000ffff
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_THM_MOVT_BREL:
                        // 0xffff0000
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_THM_MOVW_BREL:
                        // 0x0000ffff
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;

                    case R_ARM_GOTPC:   // a.k.a. R_ARM_BASE_PREL == B(S) + A - P
                        b_arm_get32(p, a);
                        b_s = uld_get_base_address(l_src_sym);
                        b_arm_set32(p, reinterpret_cast<std::int32_t>(b_s + a) - reinterpret_cast<std::int32_t>(p));
                        break;
                    case R_ARM_GOT32:   // a.k.a. R_ARM_GOT_BREL == GOT(S) + A - GOT_ORG
                        // we don't have an actual GOT, but even better - a runtime symbol table - so this relocation will
                        // return GOT(S) as a pointer to symbol's effective address member
                        b_arm_get32(p, a);
                        b_s = uld_get_base_address(l_src_sym);
                        got_s = uld_get_global_address(l_src_sym);
                        b_arm_set32(p, reinterpret_cast<std::int32_t>(got_s + a) - reinterpret_cast<std::int32_t>(b_s));
                        break;
                    case R_ARM_GOT_ABS: // absolute address of the GOT entry
                        b_arm_get32(p, a);
                        got_s = uld_get_global_address(l_src_sym);
                        b_arm_set32(p, reinterpret_cast<std::int32_t>(got_s + a));
                        break;
                    case R_ARM_GOT_PREL:  // offset of the GOT entry relative to the PC
                        b_arm_get32(p, a);
                        got_s = uld_get_global_address(l_src_sym);
                        b_arm_set32(p, reinterpret_cast<std::int32_t>(got_s + a) - reinterpret_cast<std::int32_t>(p));
                        break;
                    case R_ARM_GOT_BREL12:
                        // b_arm_get12(r, a);
                        // b_arm_set12(r, reinterpret_cast<std::int32_t>(got_s + a) - reinterpret_cast<std::int32_t>(m_target->get_got_base()));
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_THM_GOT_BREL12:
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;


                    // case R_ARM_TLS_DTPMOD32:
                    // case R_ARM_TLS_DTPOFF32:
                    // case R_ARM_TLS_TPOFF32:
                    //     break;
                    case R_ARM_COPY:
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_GLOB_DAT:
                        // s + a | t
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_JUMP_SLOT:
                        // s + a | t
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                    case R_ARM_RELATIVE:
                        // b(s) + a
                        uld_error(
                            e_norel,
                            "Relocation %d against symbol `%s` not implemented.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;

                    // case R_ARM_TLS_GD32:
                    // case R_ARM_TLS_LDM32:
                    // case R_ARM_TLS_LDO32:
                    // case R_ARM_TLS_IE32:
                    // case R_ARM_TLS_LE32:
                    // case R_ARM_TLS_LDO12:
                    // case R_ARM_TLS_LE12:
                    // case R_ARM_TLS_IE12GP:
                    default:
                        uld_error(
                            e_norel,
                            "Unknown relocation type `%d` against symbol `%s`.",
                            __FILE__,
                            __LINE__,
                            l_rel_type,
                            l_src_sym->name
                        );
                        return false;
                  };
              } else
              if constexpr (os::is_msb) {
                  uld_error(
                      e_invalid_host,
                      "Unable to perform relocations on a BIG ENDIAN host: not implemented.",
                      __FILE__,
                      __LINE__
                  );
                  return false;
              }
          }
      }
      return true;
}

bool  factory::uld_resolve_rela(elf32_bfd_t& bi, Elf32_Shdr& shdr_info, Elf32_Rela& rela_info) noexcept
{
      Elf32_Rel    l_rel_info;
      std::int32_t l_rel_addend;
      // repact the infrmation into an Elf32_Rel, in order to be able to forward it to _resolve_rel()
      l_rel_info.r_offset = rela_info.r_offset;
      l_rel_info.r_info   = rela_info.r_info;
      l_rel_addend        = rela_info.r_addend;
      // forward...
      return uld_resolve_rel(bi, shdr_info, l_rel_info, l_rel_addend);
}

bool  factory::uld_error(int error, const char* message, const char* file, int line, ...) noexcept
{
#ifdef NDEBUG
      printf("-!- Error %d: ", error);
#else
      printf("-!- Error %d %s:%d: ", error, file, line);
#endif
      va_list va;
      va_start(va, line);
      vprintf(message, va);
      va_end(va);
      puts("\n");
      return error == 0;
}

void  factory::uld_clear() noexcept
{
}

/* prefetch()
   gather information about the curren object file, set up internal section map
*/
bool  factory::prefetch(elf32_bfd_t& bi) noexcept
{
      bool l_have_code = false;
      bool l_have_data = false;
      bool l_have_symtab = false;
      bool l_have_rel = false;
      int  l_shdr_count = bi.get_section_count();
      if(l_shdr_count > 0) {
          // reserve enough entries in the section map
          m_shdr_map.resize(l_shdr_count);
      }
      // run a scan of the section table and map to the existing image sections
      for(int l_shdr_index = 0; l_shdr_index < l_shdr_count; l_shdr_index++) {
          Elf32_Shdr  l_shdr_info;
          const char* l_shdr_name;
          int         l_shdr_name_length;
          if(bool
              l_fetch_shdr_success = bi.read_section_info(l_shdr_info, l_shdr_index);
              l_fetch_shdr_success == true) {
              m_shdr_map[l_shdr_index].name = nullptr;
              m_shdr_map[l_shdr_index].type = section_t::type_bits_from_shdr(l_shdr_info.sh_type) | symbol_t::type_section;
              m_shdr_map[l_shdr_index].flags = section_t::data_bits_from_shdr(l_shdr_info.sh_type);
              m_shdr_map[l_shdr_index].ea = nullptr;
              m_shdr_map[l_shdr_index].ra = nullptr;
              m_shdr_map[l_shdr_index].offset_base = 0;
              m_shdr_map[l_shdr_index].offset_last = 0;
              m_shdr_map[l_shdr_index].support = nullptr;
              if(bool
                  l_fetch_name_success = bi.read_section_name(l_shdr_info, l_shdr_name, l_shdr_name_length);
                  l_fetch_name_success == true) {
                  segment*  l_segment_ptr;
                  switch(l_shdr_info.sh_type) {
                      case SHT_NULL:
                          // only the first section should be a NULL
                          if(l_shdr_index > 0) {
                              printdbg(
                                  "Found NULL section at index %d.",
                                  __FILE__,
                                  __LINE__,
                                  l_shdr_index
                              );
                          }
                          break;
                      case SHT_NOBITS:
                          if(l_shdr_info.sh_flags & SHF_ALLOC) {
                              l_have_data = l_shdr_info.sh_size > 0;
                              l_segment_ptr = m_image->get_segment_by_attributes(
                                  section_t::type_nobits,
                                  section_t::data_bits_from_shdr(l_shdr_info.sh_flags)
                              );
                              m_shdr_map[l_shdr_index].name = l_segment_ptr->get_name();
                              // m_shdr_map[l_shdr_index].ea = l_segment_ptr->get_next_ptr();
                              // m_shdr_map[l_shdr_index].ra = l_segment_ptr->get_next_ptr();
                              // store the base offsets of the segments, in order to be able to roll back upon failure
                              m_shdr_map[l_shdr_index].offset_base = l_segment_ptr->get_table_offset();
                              m_shdr_map[l_shdr_index].offset_last = m_shdr_map[l_shdr_index].offset_base;
                              // remember the segment this section is supposed to be allocated to
                              m_shdr_map[l_shdr_index].support = l_segment_ptr;
                          }
                          break;
                      case SHT_PROGBITS:
                          if(l_shdr_info.sh_flags & SHF_ALLOC) {
                              if(l_shdr_info.sh_flags & SHF_EXECINSTR) {
                                  l_have_code = l_shdr_info.sh_size > 0;
                              } else
                                  l_have_data = l_shdr_info.sh_size > 0;
                              l_segment_ptr = m_image->get_segment_by_attributes(
                                  section_t::type_progbits,
                                  section_t::data_bits_from_shdr(l_shdr_info.sh_flags)
                              );
                              m_shdr_map[l_shdr_index].name = l_segment_ptr->get_name();
                              // m_shdr_map[l_shdr_index].ea = l_segment_ptr->get_next_ptr();
                              // m_shdr_map[l_shdr_index].ra = l_segment_ptr->get_next_ptr();
                              // store the base offsets of the segments, in order to be able to roll beck upon failure
                              m_shdr_map[l_shdr_index].offset_base = l_segment_ptr->get_table_offset();
                              m_shdr_map[l_shdr_index].offset_last = m_shdr_map[l_shdr_index].offset_base;
                              // remember the segment this section is supposed to be allocated to, if any
                              m_shdr_map[l_shdr_index].support = l_segment_ptr;
                              // load PROGBITS/ALLOC non-executable sections ahead of time, as they are very likely
                              // referenced in relocations
                              if(l_shdr_info.sh_size) {
                                  if((l_shdr_info.sh_flags & SHF_EXECINSTR) == 0) {
                                      std::int32_t  l_shdr_size = l_shdr_info.sh_size;
                                      std::uint8_t* l_shdr_data = uld_get_section_data(bi, l_shdr_info, l_shdr_index, 0, l_shdr_size);
                                      if(l_shdr_data == nullptr) {
                                          uld_error(
                                              e_fault,
                                              "Failed to load section `%s`.",
                                              __FILE__,
                                              __LINE__,
                                              l_shdr_name
                                          );
                                          return false;
                                      }
                                      if constexpr (is_debug) {
                                          printf(
                                              "(i) Stored section `%s` at effective address %p.\n",
                                              l_shdr_name,
                                              l_shdr_data
                                          );
                                          dbg_dump_hex(l_shdr_data, l_shdr_size, true);
                                      }
                                  }
                              }
                          }
                          break;
                      case SHT_SYMTAB:
                          // check if the symbol table is valid
                          if(l_shdr_info.sh_size > 0) {
                              if(l_shdr_info.sh_entsize > 0) {
                                  l_have_symtab = true;
                              }
                          }
                          break;
                      case SHT_REL:
                      case SHT_RELA:
                          // check if the symbol table is valid
                          if(l_shdr_info.sh_size > 0) {
                              if(l_shdr_info.sh_entsize > 0) {
                                  l_have_rel = true;
                              }
                          }
                          break;
                      default:
                          break;
                  }
              }
          }
      }
      m_shdr_count = l_shdr_count;
      m_shdr_have_code = l_have_code;
      m_shdr_have_data = l_have_data;
      m_shdr_have_symtab = l_have_symtab;
      m_shdr_have_rel = l_have_rel;
      return true;
}

/* uld_import()
   import symbols and sections from the object file
*/
bool  factory::uld_import(elf32_bfd_t& bi) noexcept
{
      // no symbol table - nothing to collect
      if(m_shdr_have_symtab == false) {
          return true;
      }
      // reserve an arbitrary number of entries into the bind table, if we have (at least) a relocation section
      if(m_shdr_have_rel) {
          m_bind_list.reserve(bind_reserve_min);
      }
      // collect all symbols in the symbol tables, if any were detected at the 'prefetch' step
      int  l_shdr_count = bi.get_section_count();
      int  l_shdr_success = 0;
      for(int l_shdr_index = 0; l_shdr_index < l_shdr_count; l_shdr_index++) {
          Elf32_Shdr  l_shdr_info;
          if(bool
              l_fetch_shdr_success = bi.read_section_info(l_shdr_info, l_shdr_index);
              l_fetch_shdr_success == true) {
              if(l_shdr_info.sh_type == SHT_SYMTAB) {
                  int l_sym_base = m_symbol_map.size();
                  int l_sym_count = bi.get_symbol_count(l_shdr_info);
                  int l_sym_success = 0;
                  // reserve space for every symbol in the symbol index array (relocations need it)
                  if(l_sym_count > 0) {
                      m_symbol_map.resize(l_sym_base + l_sym_count);
                  }
                  // run through the symbol table and collect the relevant ones
                  for(int l_sym_index = 0; l_sym_index < l_sym_count; l_sym_index++) {
                      Elf32_Sym  l_sym_info;
                      bool       l_load_sym_success;
                      if(bool
                          l_fetch_sym_success = bi.read_symbol_info(l_sym_info, l_shdr_info, l_sym_index);
                          l_fetch_sym_success == true) {
                          l_load_sym_success = uld_load_symbol(bi, l_shdr_info, l_sym_info, l_sym_base + l_sym_index);
                          if(l_load_sym_success == false) {
                              break;
                          };
                          ++l_sym_success;
                      }
                  }
                  if(l_sym_success != l_sym_count) {
                      break;
                  }
              }
              ++l_shdr_success;
          }
      }
      return l_shdr_success == l_shdr_count;
}

/* uld_resolve()
   resolve undefined symbols (perform partial relocation)
*/
bool  factory::uld_resolve(elf32_bfd_t& bi) noexcept
{
      // bind list empty - no symbols to bind
      if(m_bind_list.size() == 0u) {
          return true;
      }
      int l_bind_count = m_bind_list.size();
      int l_bind_error = 0;
      if(l_bind_count > 0) {
          int  l_shdr_count = bi.get_section_count();
          int  l_shdr_success = 0;
          // search for relocs against this symbol in any relocation table
          for(int l_shdr_index = 0; l_shdr_index < l_shdr_count; l_shdr_index++) {
              Elf32_Shdr  l_shdr_info;
              if(bool
                  l_fetch_shdr_success = bi.read_section_info(l_shdr_info, l_shdr_index);
                  l_fetch_shdr_success == true) {
                  if(l_shdr_info.sh_type == SHT_REL) {
                      int l_rel_count = bi.get_rel_count(l_shdr_info);
                      int l_rel_success = 0;
                      // run through the relocation list and esolve each one in turn, if possible
                      for(int l_rel_index = 0; l_rel_index < l_rel_count; l_rel_index++) {
                          Elf32_Rel  l_rel_info;
                          bool       l_resolve_rel_success;
                          if(bool
                              l_fetch_rel_success = bi.read_rel_info(l_rel_info, l_shdr_info, l_rel_index);
                              l_fetch_rel_success == true) {
                              l_resolve_rel_success = uld_resolve_rel(bi, l_shdr_info, l_rel_info);
                              if(l_resolve_rel_success == false) {
                                  l_bind_error++;
                                  break;
                              };
                              ++l_rel_success;
                          }
                      }
                      if(l_rel_success != l_rel_count) {
                          break;
                      }
                  } else
                  if(l_shdr_info.sh_type == SHT_RELA) {
                      int l_rela_count = bi.get_rel_count(l_shdr_info);
                      int l_rela_success = 0;
                      // run through the symbol table and collect the relevant ones
                      for(int l_rela_index = 0; l_rela_index < l_rela_count; l_rela_index++) {
                          Elf32_Rela l_rela_info;
                          bool       l_resolve_rela_success;
                          if(bool
                              l_fetch_rela_success = bi.read_rela_info(l_rela_info, l_shdr_info, l_rela_index);
                              l_fetch_rela_success == true) {
                              l_resolve_rela_success = uld_resolve_rela(bi, l_shdr_info, l_rela_info);
                              if(l_resolve_rela_success == false) {
                                  l_bind_error++;
                                  break;
                              };
                              ++l_rela_success;
                          }
                      }
                      if(l_rela_success != l_rela_count) {
                          break;
                      }
                  }
                  ++l_shdr_success;
              }
          }
      }
      return l_bind_error == 0;
}

/* uld_export()
   link loaded globals into the image
*/
bool  factory::uld_export() noexcept
{
      int l_export_count = 0;
      int l_export_success = 0;
      for(symbol_t* l_map_ptr : m_symbol_map) {
          if(l_map_ptr) {
              if(l_map_ptr->flags & symbol_t::bind_global) {
                  if(l_map_ptr->flags & symbol_t::bit_export) {
                      if(symbol_t*
                          l_sym_ptr = m_image->make_symbol(l_map_ptr->name, l_map_ptr->type, l_map_ptr->flags);
                          l_sym_ptr != nullptr) {
                          l_sym_ptr->ea = l_map_ptr->ea;
                          l_sym_ptr->ra = l_map_ptr->ra;
                          l_sym_ptr->size = l_map_ptr->size;
                          l_sym_ptr->flags ^= symbol_t::bit_export;
                          l_export_success++;
                      }
                      l_export_count++;
                  } else
                  if(l_map_ptr->flags & symbol_t::bit_define) {
                      if(symbol_t*
                          l_sym_ptr = l_map_ptr;
                          l_sym_ptr != nullptr) {
                          l_sym_ptr->flags ^= symbol_t::bit_define;
                      }
                  }
              }
          }
      }
      return l_export_success == l_export_count;
}

/* uld_revert()
*/
bool  factory::uld_revert() noexcept
{
      return true;
}

bool  factory::collect(elf32_bfd_t& bi) noexcept
{
      return
          uld_import(bi) &&
          uld_resolve(bi) &&
          uld_export();
}

/*namespace elf32*/ }
/*namespace uld*/ }
