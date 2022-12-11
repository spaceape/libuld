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
#include "program_table.h"
#include "target.h"
#include "string_table.h"
#include "symbol_table.h"

namespace uld {

      program_table_t::program_table_t(target* target, string_table_t* strtab, symbol_table_t* symtab) noexcept:
      m_target(target),
      m_string_table(strtab),
      m_symbol_table(symtab),
      m_segment_count(segment_count_min)
{
      make_segment(nullptr, section_t::type_undef, section_t::no_flags);
}

      program_table_t::~program_table_t()
{
}

int   program_table_t::get_default_segment_mapping(unsigned int type, unsigned int flags) const noexcept
{
      if(type == section_t::type_undef) {
          return 0;
      } else
      if((type & section_t::type_progbits) == section_t::type_progbits) {
          if(flags & section_t::data_alloc) {
              if(flags & section_t::data_execute) {
                  return 1;
              } else
              if(flags & section_t::data_write) {
                  return 2;
              } else
                  return 3;
          }
      } else
      if((type & section_t::type_nobits) == section_t::type_nobits) {
          if(flags & section_t::data_alloc) {
              return 4;
          }
      }
      return -1;
}

int   program_table_t::get_default_segment_alignment(unsigned int type, unsigned int flags) const noexcept
{
      if(type == section_t::type_undef) {
          return 0;
      } else
      if((type & section_t::type_progbits) == section_t::type_progbits) {
          if(flags & section_t::data_alloc) {
              if(flags & section_t::data_execute) {
                  return m_target->get_code_align();
              } else
                  return m_target->get_data_align();
          }
      } else
      if((type & section_t::type_nobits) == section_t::type_nobits) {
          if(flags & section_t::data_alloc) {
              return m_target->get_data_align();
          }
      }
      return m_target->get_default_align();
}

segment* program_table_t::make_segment(int meta, unsigned int type, unsigned int flags, int align) noexcept
{
        return make_segment(m_target->get_segment_name(meta), type, flags, align);
}

segment* program_table_t::make_segment(const char* name, unsigned int type, unsigned int flags, int align) noexcept
{
      int  i_segment = get_default_segment_mapping(type, flags);
      if(i_segment > 0) {
          // if the default mapping returns an existing segment but names don't match, then this has to be a new segment
          // clear segment index if so and let the routine create a new segment
          if((name != nullptr) &&
              (m_segment_list[i_segment] != nullptr)) {
              if(m_segment_list[i_segment]->has_name(name) == false) {
                  i_segment = -1;
              }
          }
      }
      // look for a free slot into the local segment array
      if(i_segment < 0) {
          for(int i_segment = m_segment_count; i_segment < segment_count_max; i_segment++) {
              if(m_segment_list[i_segment] == nullptr) {
                  i_segment = i_segment;
                  break;
              }
          }
      }
      // create the the segment and associate a 'section' symbol with it
      if(i_segment >= 0) {
          if(i_segment > 0) {
              if(m_segment_list[i_segment] == nullptr) {
                  symbol_t* l_symbol_ptr = m_symbol_table->make_symbol(name, type, flags);
                  if(l_symbol_ptr == nullptr) {
                      return nullptr;
                  }
                  if(align <= 0) {
                      align = get_default_segment_alignment(type, flags);
                  }
                  auto l_segment_ptr = std::make_unique<segment>(l_symbol_ptr->name, type, flags, align);
                  if(l_segment_ptr == nullptr) {
                      return nullptr;
                  }
                  l_symbol_ptr->ea = nullptr;
                  l_symbol_ptr->ra = nullptr;
                  m_segment_list[i_segment] = std::move(l_segment_ptr);
              }
              if(i_segment >= segment_count_min) {
                  m_segment_count++;
              }
          }
          return m_segment_list[i_segment].get();
      }
      return nullptr;
}

segment* program_table_t::get_segment_by_index(int index) noexcept
{
      if((index >= 0) &&
          (index < m_segment_count)) {
          return m_segment_list[index].get();
      }
      return nullptr;
}

segment* program_table_t::get_segment_by_name(const char*) noexcept
{
      return nullptr;
}

segment* program_table_t::get_segment_by_attributes(unsigned int type, unsigned int flags) noexcept
{
      int  i_segment = get_default_segment_mapping(type, flags);
      if(i_segment > 0) {
          return get_segment_by_index(i_segment);
      } else
      if(i_segment < 0) {
          for(i_segment = 0; i_segment < m_segment_count; i_segment++) {
              if(m_segment_list[i_segment]->has_type(type)) {
                  // match only on type and disregard attribute flags or match the exact specified flags
                  if((flags & section_t::type_bits) == section_t::type_bits) {
                      return m_segment_list[i_segment].get();
                  } else
                  if(m_segment_list[i_segment]->has_flags(flags)) {
                      return m_segment_list[i_segment].get();
                  }
              }
          }
      }
      return nullptr;
}

int   program_table_t::get_segment_count() const noexcept
{
      return m_segment_count;
}

void  program_table_t::free_segment(segment*) noexcept
{
}

/*namespace uld*/ }
