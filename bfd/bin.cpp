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
#include "raw.h"
#include <elf.h>

namespace uld {

      bin_bfd_t::bin_bfd_t(raw_bfd_t& source) noexcept:
      raw_bfd_t(source),
      m_class(bin_32),
      m_abi_id(ELFOSABI_NONE),
      m_abi_version(0),
      m_lsb_bit(false),
      m_msb_bit(false),
      m_elf_bit(false)
{
      // load the ident header in order to set up the object's operating parameters
      if(source.has_type(file_type_elf)) {
          std::uint8_t  l_head_data[EI_NIDENT];
          unsigned int  l_elf_class;
          unsigned int  l_elf_fmt;
          unsigned int  l_elf_version;
          unsigned int  l_elf_abi;
          unsigned int  l_abi_version;
          FRESULT       l_rc;
          unsigned int  l_read_size;
          if(l_rc = f_read(m_file_ptr, l_head_data, EI_NIDENT, std::addressof(l_read_size));
              l_rc == FR_OK) {
              l_elf_class   = l_head_data[EI_CLASS];
              l_elf_fmt     = l_head_data[EI_DATA];
              l_elf_version = l_head_data[EI_VERSION];
              l_elf_abi     = l_head_data[EI_OSABI];
              l_abi_version = l_head_data[EI_ABIVERSION];
              // performa a sanity check on the header information
              m_elf_bit = 
                  ((l_elf_class == ELFCLASS32) || (l_elf_class == ELFCLASS64)) &&
                  ((l_elf_fmt == ELFDATA2LSB) || (l_elf_fmt == ELFDATA2MSB)) &&
                  (l_elf_version == EV_CURRENT) &&
                  (l_elf_abi <= static_cast<unsigned int>(ELFOSABI_STANDALONE));

              // set the class members with the relevant header information
              if(m_elf_bit) {
                  if(l_elf_class == ELFCLASS32) {
                      m_class = bin_32;
                  } else
                  if(l_elf_class == ELFCLASS64) {
                      m_class = bin_64;
                  }
                  m_abi_id = l_elf_abi;
                  m_abi_version = l_abi_version;
                  m_lsb_bit = l_elf_fmt == ELFDATA2LSB;
                  m_msb_bit = l_elf_fmt == ELFDATA2MSB;
              }
          }
      }
}

      bin_bfd_t::bin_bfd_t(bin_bfd_t& source) noexcept:
      raw_bfd_t(source),
      m_class(source.m_class),
      m_abi_id(source.m_abi_id),
      m_abi_version(source.m_abi_version),
      m_lsb_bit(source.m_lsb_bit),
      m_msb_bit(source.m_msb_bit),
      m_elf_bit(source.m_elf_bit)
{
}

      bin_bfd_t::~bin_bfd_t() noexcept
{
}

bool  bin_bfd_t::has_class(unsigned int value) const noexcept
{
      return m_class == value;
}

bool  bin_bfd_t::has_abi_id(unsigned int abi_id) const noexcept
{
      return m_abi_id == abi_id;
}

unsigned int bin_bfd_t::get_abi_id() const noexcept
{
      return m_abi_id;
}

bool  bin_bfd_t::has_abi_version(unsigned int abi_version) const noexcept
{
      return m_abi_version == abi_version;
}

unsigned int bin_bfd_t::get_abi_version() const noexcept
{
      return m_abi_version;
}

bool  bin_bfd_t::is_lsb() const noexcept
{
      return m_lsb_bit == true;
}

bool  bin_bfd_t::get_lsb() const noexcept
{
      return m_lsb_bit;
}

bool  bin_bfd_t::is_msb() const noexcept
{
      return m_msb_bit == true;
}

bool  bin_bfd_t::get_msb() const noexcept
{
      return m_msb_bit;
}

bool  bin_bfd_t::is_elf() const noexcept
{
      return m_elf_bit;
}

      bin_bfd_t::operator bool() const noexcept
{
      return m_elf_bit;
}

/*namespace uld*/ }
