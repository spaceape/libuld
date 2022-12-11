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
#include "target.h"
#include <elf.h>
#include "hardware/regs/addressmap.h"
#include <limits>

namespace uld {

/* s_section_name
   section names for the meta-types 
*/
static const char* s_section_name[] =
{
    "",           // null
    ".text",      // text
    ".data",      // data
    ".rodata",    // rodata
    ".bss",       // zdata
    ".udata",     // udata
    ".got",       // got
    ".raw",       // raw
    ".rel",       // rel
    ".rela",      // rela
    ".comment",   // comment
    ".symtab",    // symtab
    ".strtab",    // strtab
    ".shstrtab"   // shstrtab

};

      target::target(
          unsigned int machine_type,
          unsigned int machine_class,
          bool         machine_lsb,
          bool         machine_vle
      ) noexcept:
      m_machine(machine_type),
      m_abi_id(ELFOSABI_NONE),
      m_abi_version(0),
      m_class(machine_class),
      m_vle_bit(machine_vle == true),
      m_lsb_bit(machine_lsb == true),
      m_msb_bit(machine_lsb == false)
{
      // set machine-specific defaults
      if(m_machine == EM_ARM) {
          m_abi_id = ELFOSABI_ARM;
          m_abi_version = 0u;
          m_class = bin_32;
      }
}

      target::~target()
{
}

unsigned int target::get_machine_type() const noexcept
{
      return m_machine;
}

bool  target::has_machine_type(unsigned int machine) const noexcept
{
      return m_machine == machine;
}

unsigned int  target::get_abi_id() const noexcept
{
      return m_abi_id;
}

unsigned int  target::get_abi_version() const noexcept
{
      return m_abi_version;
}

unsigned int  target::get_class() const noexcept
{
      return m_class;
}

bool  target::has_class(unsigned int value) const noexcept
{
      return m_class == value;
}

int   target::get_machine_word_bits() const noexcept
{
      if(m_class == bin_32) {
          return 32;
      } else
      if(m_class == bin_64) {
          return 64;
      }
      return 0;
}

int   target::get_machine_word_size() const noexcept
{
      if(m_class == bin_32) {
          return 4;
      } else
      if(m_class == bin_64) {
          return 8;
      }
      return 0;
}

/* get_ep_name()
   get entry point name - quite overwhelmingly so - "main()"
*/
const char* target::get_ep_name() const noexcept
{
      return "main";
}

/* get_got_name()
   get global offset table symbol name
*/
const char* target::get_got_name() const noexcept
{
      return "_GLOBAL_OFFSET_TABLE_";
}

/* get_segment_name()
   get default section name for the specified meta-type
*/
const char* target::get_segment_name(int meta_type_id) const noexcept
{
      return s_section_name[meta_type_id];
}

int   target::get_address_align() const noexcept
{
      if(m_class == bin_64) {
          return 3;
      } else
      if(m_class == bin_32) {
          return 2;
      } else
          return 1;
}

long int target::get_address_mask() const noexcept
{
      return std::numeric_limits<long int>::max() ^ ((1 << get_address_align()) - 1);
}

/* get_address_base()
   get the base of the address space, as seen by programs on the target
*/
std::uint8_t* target::get_address_base() const noexcept
{
      return reinterpret_cast<std::uint8_t*>(SRAM_BASE);
}

int   target::get_code_align() const noexcept
{
      return get_address_align();
}

int   target::get_data_align() const noexcept
{
      return get_address_align();
}

int   target::get_default_align() const noexcept
{
      return 0;
}

long int target::get_vle_bit() const noexcept
{
      return 1;
}

long int target::get_vle_mask() const noexcept
{
      return std::numeric_limits<long int>::max() ^ get_vle_bit();
}

bool  target::is_vle() const noexcept
{
      return m_vle_bit;
}

bool  target::is_lsb() const noexcept
{
      return m_lsb_bit;
}

bool  target::is_msb() const noexcept
{
      return m_msb_bit;
}

/*namespace uld*/ }
