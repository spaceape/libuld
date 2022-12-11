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
#include "raw.h"
#include <elf.h>
#include <ar.h>
#include <log.h>
#include <f_util.h>
#include <cstring>

namespace uld {

      raw_bfd_t::raw_bfd_t(const char* file_name, int file_offset, int file_open_mode) noexcept:
      m_type(file_type_none),
      m_file_ptr(),
      m_file_offset(0)
{
      file_ptr l_file_ptr = file_ptr::make_file_cb();
      FRESULT  l_rc       = f_open(l_file_ptr, file_name, file_open_mode);
      if(l_rc == FR_OK) {
          char         l_magic[SARMAG];
          unsigned int l_read_size;
          // seek to the specified file position, if we are in an archive
          if(m_file_offset != 0) {
              l_rc = f_lseek(l_file_ptr, m_file_offset);
          }
          // read the file magic
          if(l_rc = f_read(l_file_ptr, l_magic, SARMAG, std::addressof(l_read_size));
              l_rc == FR_OK) {
              if(l_magic[0] == ARMAG[0]) {
                  if(std::strncmp(l_magic, ARMAG, SARMAG) == 0) {
                      m_type = file_type_archive;
                  }
              } else
              if(l_magic[0] == ELFMAG0) {
                  if((l_magic[1] == ELFMAG1) &&
                      (l_magic[2] == ELFMAG2) &&
                      (l_magic[3] == ELFMAG3)) {
                      m_type = file_type_elf;
                  }
              }
              m_file_ptr = l_file_ptr;
          } else
              printdbg(
                  "Error while attempting to read '%s': %s.",
                  __FILE__,
                  __LINE__,
                  file_name,
                  FRESULT_str(l_rc)
              );
      } else
          printdbg(
              "Error while attempting to open '%s': %s.",
              __FILE__,
              __LINE__,
              file_name,
              FRESULT_str(l_rc)
          );
}

      raw_bfd_t::raw_bfd_t(raw_bfd_t& source) noexcept:
      m_type(source.m_type),
      m_file_ptr(source.m_file_ptr),
      m_file_offset(source.m_file_offset)
{
      FRESULT l_rc = f_lseek(m_file_ptr, m_file_offset);
      if(l_rc != FR_OK) {
          printdbg(
              "File error: %s",
              __FILE__,
              __LINE__,
              FRESULT_str(l_rc)
          );
      }
}

      raw_bfd_t::~raw_bfd_t() noexcept
{
}

unsigned int raw_bfd_t::get_type() const noexcept
{
      return m_type;
}

bool  raw_bfd_t::has_type(unsigned int type) const noexcept
{
      return m_type == type;
}

util::file_ptr& raw_bfd_t::get_file_ptr() noexcept
{
      return m_file_ptr;
}

int   raw_bfd_t::get_file_offset() const noexcept
{
      return m_file_offset;
}

      raw_bfd_t::operator bool() const noexcept
{
      return m_file_ptr;
}

/*namespace uld*/ }
