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
#include "image.h"
#include "target.h"
#include "elf32.h"
#include "elf64.h"
#include "bfd/raw.h"
#include "bfd/bin.h"
#include "bfd/elf32.h"
#include "bfd/elf64.h"
#include <error.h>
#include <cstdarg>

      constexpr unsigned int s_state_clean = 0u;
      constexpr unsigned int s_state_set   = 1u;
      constexpr unsigned int s_state_error = 1u;

      constexpr unsigned int nop = 0;
      constexpr unsigned int op_collect = 1;

namespace uld {

      image::image(target* target) noexcept:
      m_target(target),
      m_string_table(),
      m_symbol_table(std::addressof(m_string_table)),
      m_program(m_target, std::addressof(m_string_table), std::addressof(m_symbol_table)),
      m_state(s_state_clean)
{
      uld_set();
}

      image::~image()
{
}

void  image::uld_set()
{
      if((m_state & s_state_set) == 0) {
          // create the essential sections and their associated symbols
          m_program.make_segment(target::smt::null, section_t::type_undef, section_t::no_flags);
          m_program.make_segment(target::smt::text, section_t::type_progbits, section_t::data_alloc | section_t::data_execute);
          m_program.make_segment(target::smt::data, section_t::type_progbits, section_t::data_alloc | section_t::data_write);
          if(segment*
              l_rodata_segment = m_program.make_segment(target::smt::rodata, section_t::type_progbits, section_t::data_alloc);
              l_rodata_segment != nullptr) {
              // we don't need a GOT but must have a `_GLOBAL_OFFSET_TABLE_` symbol defined somewhere: the top of the `.rodata`
              // segment seems appropriate;
              auto l_got_entry_name = m_target->get_got_name();
              int  l_got_entry_size = m_target->get_machine_word_size();
              int  l_got_table_size = l_got_entry_size * 2;
              bool l_lsb_bit = m_target->is_lsb();
              if(symbol_t*
                  l_got_symbol = m_symbol_table.make_symbol(l_got_entry_name, symbol_t::type_object, symbol_t::bind_weak);
                  l_got_symbol != nullptr) {
                  std::uint8_t*  l_got_address = m_target->get_address_base();
                  std::uint8_t*  l_got_ptr     = l_rodata_segment->raw_get(l_got_table_size);
                  if(l_got_ptr != nullptr) {
                      l_got_symbol->ea = l_got_ptr; 
                      l_got_symbol->ra = l_got_ptr; 
                      if(l_got_entry_size == 4) {
                          if(l_lsb_bit) {
                              symbol_t::set_int32_le(l_got_symbol, reinterpret_cast<long int>(l_got_address));
                          } else
                              symbol_t::set_int32_be(l_got_symbol, reinterpret_cast<long int>(l_got_address));
                      } else
                      if(l_got_entry_size == 8) {
                          if(l_lsb_bit) {
                              symbol_t::set_int64_le(l_got_symbol, reinterpret_cast<long long int>(l_got_address));
                          } else
                              symbol_t::set_int64_be(l_got_symbol, reinterpret_cast<long long int>(l_got_address));
                      }
                      std::memset(l_got_ptr + l_got_entry_size, 0, l_got_entry_size);
                  }
              }
          }
          m_program.make_segment(target::smt::bss, section_t::type_nobits, section_t::data_alloc | section_t::data_write);
          m_state |= s_state_set;
      }
}

bool  image::uld_load_elf32(bin_bfd_t& source, unsigned int operation) noexcept
{
      if(operation != nop) {
          elf32_bfd_t     l_elf32_file(source);
          elf32::factory  l_elf32_factory(this);
          unsigned int    l_target_machine_type = m_target->get_machine_type();
          unsigned int    l_object_machine_type = l_elf32_file.get_machine_type();
          if(l_target_machine_type == l_object_machine_type) {
              l_elf32_factory.prefetch(l_elf32_file);
              while(operation) {
                  if(operation & op_collect) {
                      if(unsigned int
                          l_object_binary_type = l_elf32_file.get_object_type();
                          l_object_binary_type == ET_REL) {
                          if(l_elf32_factory.collect(l_elf32_file)) {
                              operation ^= op_collect;
                          } else
                              break;
                      } else
                          return uld_error(1, "Refusing to load a non-relocatable ELF object.");
                  } else
                      return uld_error(1, "Invalid target operation '%d' for architecture '%d'.", operation, l_object_machine_type);
              }
          } else
              return uld_error(1, "Invalid target: '%d'.", l_object_machine_type);
      }
      return operation == nop;
}

bool  image::uld_load_object(raw_bfd_t& source, unsigned int operation) noexcept
{
      if(operation != nop) {
          bin_bfd_t l_bin_file(source);
          if(l_bin_file.has_class(bin_32) &&
              m_target->has_class(bin_32)) {
              return uld_load_elf32(l_bin_file, operation);
          } else
              return uld_error(1, "Invalid or unsupported object.");
      }
      return true;
}

bool  image::uld_load_library(raw_bfd_t& source, unsigned int operation) noexcept
{
      if(operation != nop) {
          if(operation == op_collect) {
              return uld_error(1, "Invalid or unsupported object");
          } else
              return false;
      }
      return true;
}

bool  image::uld_error(int error, const char* message, ...) noexcept
{
      printf("-!- Error %d: ", error);
      va_list va;
      va_start(va, message);
      vprintf(message, va);
      va_end(va);
      puts("\n");
      return error == 0;
}

bool  image::load(const char* file_name) noexcept
{
      raw_bfd_t l_raw_file(file_name);
      if(l_raw_file) {
          if(l_raw_file.has_type(file_type_archive)) {
              return uld_load_library(l_raw_file, op_collect);
          } else
          if(l_raw_file.has_type(file_type_elf)) {
              return uld_load_object(l_raw_file, op_collect);
          } else
              return uld_error(1, "File `%s` does not have a valid format.", file_name);
      } else
          return uld_error(1, "File `%s` cannot be accessed.", file_name);
}

symbol_t* image::find_symbol(const char* name, unsigned int bind_flags) noexcept
{
      return m_symbol_table.find_symbol(name, bind_flags);
}

symbol_t* image::make_symbol(const char* name, unsigned int type, unsigned int flags) noexcept
{
      return m_symbol_table.make_symbol(name, type, flags);
}

symbol_t* image::make_symbol(const char* name, unsigned int type, unsigned int flags, void* bind_address, void* virtual_address) noexcept
{
      symbol_t* l_symbol = m_symbol_table.make_symbol(name, type, flags);
      if(l_symbol != nullptr) {
          if(bind_address != nullptr) {
              auto    l_am = m_target->get_address_mask();
              ssize_t l_ea = reinterpret_cast<ssize_t>(bind_address);
              ssize_t l_ra = l_ea;
              if(type == symbol_t::type_function) {
                  ssize_t l_aa =(1 << m_target->get_address_align()) - 1;
                  if(virtual_address != nullptr) {
                      l_ra = reinterpret_cast<ssize_t>(virtual_address);
                  }
                  if(l_ea & l_aa) {
                      l_ea = l_ea & l_am;
                  }
              }
              l_symbol->ea = reinterpret_cast<std::uint8_t*>(l_ea);
              l_symbol->ra = reinterpret_cast<std::uint8_t*>(l_ra);
          }
      }
      return l_symbol;
}

segment*  image::get_segment_by_name(const char* name) noexcept
{
      return m_program.get_segment_by_name(name);
}

segment*  image::get_segment_by_attributes(unsigned int type, unsigned int flags) noexcept
{
      return m_program.get_segment_by_attributes(type, flags);
}

int   image::get_segment_count() const noexcept
{
      return m_program.get_segment_count();
}

target* image::get_target() noexcept
{
      return m_target;
}

auto  image::get_string_table() noexcept -> string_table_t*
{
      return std::addressof(m_string_table);
}

auto  image::get_symbol_table() noexcept -> symbol_table_t*
{
      return std::addressof(m_symbol_table);
}

auto  image::get_program_table() noexcept -> program_table_t*
{
      return std::addressof(m_program);
}

/*namespace uld*/ }
