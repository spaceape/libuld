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
#include "symbol_table.h"
#include "string_table.h"

namespace uld {

      symbol_table_t::symbol_table_t(string_table_t* strtab) noexcept:
      table(),
      m_string_table(strtab)
{
}

      symbol_table_t::~symbol_table_t()
{
}

symbol_t* symbol_table_t::make_symbol(const char* name) noexcept
{
        return make_symbol(name, symbol_t::type_undef, symbol_t::no_flags);
}

symbol_t* symbol_table_t::make_symbol(
            const char*   name,
            unsigned int  type,
            unsigned int  flags
      ) noexcept
{
      return make_symbol(name, 0, type, flags);
}

symbol_t* symbol_table_t::make_symbol(
            const char*   name_ptr,
            int           name_length,
            unsigned int  type,
            unsigned int  flags
      ) noexcept
{
      symbol_t* l_symbol_ptr = raw_get();
      if(l_symbol_ptr != nullptr) {
          l_symbol_ptr->name = nullptr;
          if(name_ptr != nullptr) {
              if(name_length <= 0) {
                  name_length = std::strlen(name_ptr);
              }
              l_symbol_ptr->name = m_string_table->make_string(name_ptr, name_length);
          }
          l_symbol_ptr->type = type;
          l_symbol_ptr->flags = flags;
          l_symbol_ptr->size = 0;
          l_symbol_ptr->ea = nullptr;
          l_symbol_ptr->ra = nullptr;
      }
      return l_symbol_ptr;
}

symbol_t* symbol_table_t::find_symbol(const char* name, unsigned int bind_flags)
{
      if((name) && 
          (name[0] != 0)) {
          iterator     i_node  = begin();
          unsigned int l_flags = bind_flags & symbol_t::bind_any;
          while(i_node) {
              symbol_t*   l_sym_ptr  = i_node;
              const char* l_sym_name = l_sym_ptr->name;
              if((l_sym_name) &&
                  (l_sym_name[0] != 0)) {
                  // NOTE: pretense to use strncmp() instead of strcmp() - it yields an unwarranted fault
                  bool l_cmp_name = std::strncmp(l_sym_name, name, std::numeric_limits<short int>::max()) == 0;
                  bool l_cmp_flags = 
                          (l_flags == symbol_t::bind_any) ||
                          ((l_flags & l_sym_ptr->flags) == l_flags);
                  if((l_cmp_name == true) &&
                      (l_cmp_flags == true)) {
                      return i_node;
                  }
              }
              i_node++;
          }
      }
      return nullptr;
}

/*namespace uld*/ }
