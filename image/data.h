#ifndef uld_image_data_h
#define uld_image_data_h
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
#include <cstring>
#include <os.h>

namespace uld {

struct symbol_t;
struct section_t;

/* symbol_t
   format-independent representation of a symbol
*/
struct symbol_t
{
  /* type_*
     symbol type flags (mapped directly from ELF STT_*)
  */
  static constexpr unsigned int type_undef = 0u;    // symbol type is unspecified
  static constexpr unsigned int type_object =	1u;   // symbol is a data object
  static constexpr unsigned int type_function = 2u; // symbol is a code object
  static constexpr unsigned int type_section = 3u;  // symbol associated with a section
  static constexpr unsigned int type_file = 4u;     // symbol associated with a source file
  static constexpr unsigned int type_common = 5u;   // symbol is a common data object
  static constexpr unsigned int type_tls = 6u;      // symbol is thread-local data object
  static constexpr unsigned int type_bits = 15u;

  /* bind_*
     flags for symbol binding (mapped directly from ELF STB_*)
     [ -- -- -- -F ]
  */
  static constexpr unsigned int bind_local    = 0u;
  static constexpr unsigned int bind_global   = 0x00000001;
  static constexpr unsigned int bind_weak     = 0x00000002;
  static constexpr unsigned int bind_bits     = 0X0000000f;
  static constexpr unsigned int bind_any      = bind_local | bind_global | bind_weak;

  /* org_*
     origin flag
     [ -F -- -- -- ]
  */
  static constexpr unsigned int org_storage   = 0u;           // symbol is stored in a section
  static constexpr unsigned int org_absolute  = 0x01000000;
  static constexpr unsigned int org_common    = 0x02000000;

  static constexpr unsigned int no_flags      = 0u;
  static constexpr unsigned int any_flags     = 0xffffffff;

  /* bit_*
     [ -- -F -- -- ]
  */
  static constexpr unsigned int bit_none      = 0u;
  static constexpr unsigned int bit_keep      = 0x00010000;
  static constexpr unsigned int bit_export    = bit_keep;
  static constexpr unsigned int bit_define    = 0x00020000;

  const char*    name;
  unsigned int   type;
  unsigned int   flags;
  int            size;
  std::uint8_t*  ea;       // effective address  (where in the memory the symbol data is effectively stored)
  std::uint8_t*  ra;       // runtime address (what "the process" should see as the address of the symbol)

  static constexpr void set_int8(symbol_t* symbol, int value) noexcept
  {
        *symbol->ea = value & 0xff;
  }

  static constexpr void set_int16_le(symbol_t* symbol, int value) noexcept
  {
        if(symbol->type == type_object) {
            if constexpr (os::is_lsb) {
                symbol->ea[0] = value & 0xff;
                symbol->ea[1] =(value & 0xff00) >> 8;
            } else
            if constexpr (os::is_msb) {
                symbol->ea[0] =(value & 0xff00) >> 8;
                symbol->ea[1] = value & 0xff;
            }
        }
  }
  
  static constexpr void set_int16_be(symbol_t* symbol, int value) noexcept
  {
        if(symbol->type == type_object) {
            if constexpr (os::is_lsb) {
                symbol->ea[0] =(value & 0xff00) >> 8;
                symbol->ea[1] = value & 0xff;
            } else
            if constexpr (os::is_msb) {
                symbol->ea[0] = value & 0xff;
                symbol->ea[1] =(value & 0xff00) >> 8;
            }
        }
  }
  
  static constexpr void set_int32_le(symbol_t* symbol, long int value) noexcept
  {
        if(symbol->type == type_object) {
            if constexpr (os::is_lsb) {
                symbol->ea[0] = value & 0xff;
                symbol->ea[1] =(value & 0xff00) >> 8;
                symbol->ea[2] =(value & 0xff0000) >> 16;
                symbol->ea[3] =(value & 0xff000000) >> 24;
            } else
            if constexpr (os::is_msb) {
                symbol->ea[0] =(value & 0xff000000) >> 24;
                symbol->ea[1] =(value & 0xff0000) >> 16;
                symbol->ea[2] =(value & 0xff00) >> 8;
                symbol->ea[3] = value & 0xff;
            }
        }
  }
  
  static constexpr void set_int32_be(symbol_t* symbol, long int value) noexcept
  {
        if(symbol->type == type_object) {
            if constexpr (os::is_lsb) {
                symbol->ea[0] =(value & 0xff000000) >> 24;
                symbol->ea[1] =(value & 0xff0000) >> 16;
                symbol->ea[2] =(value & 0xff00) >> 8;
                symbol->ea[3] = value & 0xff;
            } else
            if constexpr (os::is_msb) {
                symbol->ea[0] = value & 0xff;
                symbol->ea[1] =(value & 0xff00) >> 8;
                symbol->ea[2] =(value & 0xff0000) >> 16;
                symbol->ea[3] =(value & 0xff000000) >> 24;
            }
        }
  }
  
  static constexpr void set_int64_le(symbol_t* symbol, long long int value) noexcept
  {
        if(symbol->type == type_object) {
            if constexpr (os::is_lsb) {
                symbol->ea[0] = value & 0xff;
                symbol->ea[1] =(value & 0xff00) >> 8;
                symbol->ea[2] =(value & 0xff0000) >> 16;
                symbol->ea[3] =(value & 0xff000000) >> 24;
                symbol->ea[4] =(value & 0xff00000000) >> 32;
                symbol->ea[5] =(value & 0xff0000000000) >> 40;
                symbol->ea[6] =(value & 0xff000000000000) >> 48;
                symbol->ea[7] =(value & 0xff00000000000000) >> 52;
            } else
            if constexpr (os::is_msb) {
                symbol->ea[0] =(value & 0xff00000000000000) >> 52;
                symbol->ea[1] =(value & 0xff000000000000) >> 48;
                symbol->ea[2] =(value & 0xff0000000000) >> 40;
                symbol->ea[3] =(value & 0xff00000000) >> 32;
                symbol->ea[4] =(value & 0xff000000) >> 24;
                symbol->ea[5] =(value & 0xff0000) >> 16;
                symbol->ea[6] =(value & 0xff00) >> 8;
                symbol->ea[7] = value & 0xff;
            }
        }
  }

  static constexpr void set_int64_be(symbol_t* symbol, long long int value) noexcept
  {
        if(symbol->type == type_object) {
            if constexpr (os::is_lsb) {
                symbol->ea[0] =(value & 0xff00000000000000) >> 52;
                symbol->ea[1] =(value & 0xff000000000000) >> 48;
                symbol->ea[2] =(value & 0xff0000000000) >> 40;
                symbol->ea[3] =(value & 0xff00000000) >> 32;
                symbol->ea[4] =(value & 0xff000000) >> 24;
                symbol->ea[5] =(value & 0xff0000) >> 16;
                symbol->ea[6] =(value & 0xff00) >> 8;
                symbol->ea[7] = value & 0xff;
            } else
            if constexpr (os::is_msb) {
                symbol->ea[0] = value & 0xff;
                symbol->ea[1] =(value & 0xff00) >> 8;
                symbol->ea[2] =(value & 0xff0000) >> 16;
                symbol->ea[3] =(value & 0xff000000) >> 24;
                symbol->ea[4] =(value & 0xff00000000) >> 32;
                symbol->ea[5] =(value & 0xff0000000000) >> 40;
                symbol->ea[6] =(value & 0xff000000000000) >> 48;
                symbol->ea[7] =(value & 0xff00000000000000) >> 52;
            }
        }
  }

  static constexpr void clear(symbol_t* symbol) noexcept
  {
        if((symbol->type == type_function) ||
            (symbol->type == type_object)) {
            std::memset(symbol->ea, 0, symbol->size);
        }
  }
};

/* section_t
   format-independent representation of a section
*/
struct section_t: public symbol_t
{
  /* type_*
     section types (complementary to symbol types, since a section_t is also a symbol_t)
  */
  static constexpr unsigned int type_progbits = 0x00000010 | symbol_t::type_section;
  static constexpr unsigned int type_symtab   = 0x00000020 | symbol_t::type_section;
  static constexpr unsigned int type_strtab   = 0x00000030 | symbol_t::type_section;
  static constexpr unsigned int type_note     = 0x00000070 | symbol_t::type_section;
  static constexpr unsigned int type_nobits   = 0x00000080 | symbol_t::type_section;
  static constexpr unsigned int type_rel      = 0x00000090 | symbol_t::type_section;
  static constexpr unsigned int type_rela     = 0x00000040 | symbol_t::type_section;
  static constexpr unsigned int type_bits     = 0x000000f0 | symbol_t::type_section;

  /* data_*
     [ -- -- FF -- ]
  */
  static constexpr unsigned int data_write    = 0x00000100;  // section contains writeable data
  static constexpr unsigned int data_alloc    = 0x00000200;  // section allocatable (occupies memory at runtime)
  static constexpr unsigned int data_execute  = 0x00000400;  // section contains executable data
  static constexpr unsigned int data_string   = 0x00002000;  // section contains a null-terminated string table
  static constexpr unsigned int data_bits     = 0x0000ff00;

  int           offset_base;    // offset of the section within the supporting segment
  int           offset_last;
  segment*      support;        // supporting segment for the section

  static constexpr unsigned int type_bits_from_shdr(unsigned int type) noexcept
  {
        return  (type & 0x0f) << 4;
  }

  static constexpr unsigned int type_bits_to_shdr(unsigned int type) noexcept
  {
        return  (type & type_bits) >> 4;
  }

  static constexpr unsigned int data_bits_from_shdr(unsigned int bits) noexcept
  {
        return  (bits & 255) << 8;
  }

  static constexpr unsigned int data_bits_to_shdr(unsigned int bits) noexcept
  {
        return  (bits & data_bits) >> 8;
  }
};

/* binding_t
   binding entry - keeps track of what symbols are bound to the image
*/
struct binding_t
{
  short int     symbol_index;           // index of the symbol in the source symbol table
  short int     source_index;           // index of the section in the source section table where the symbol resides
  int           source_offset_base;     // offset within the source section where the symbol data starts
  int           source_offset_last;     // offset within the source section where the symbol data ends
};

/*namespace uld*/ }
#endif
