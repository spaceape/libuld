#ifndef uld_target_h
#define uld_target_h
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
#include "uld.h"
#include <string>

namespace uld {

/* target
   runtime configuration object for runtime images
*/
class target
{
  private:
  /* m_machine
     ELF EM_* identifier
  */
  unsigned int  m_machine;

  /* m_abi_id
     ELF ELFOSABI_* identifier
  */
  unsigned int  m_abi_id;

  /* m_abi_version
  */
  unsigned int  m_abi_version;

  /* m_class
     ELF ELFCLASS* identifier
  */
  unsigned int  m_class;

  /* m_vle_bit
     vle instrunction encoding
  */
  bool  m_vle_bit;

  /* m_?sb_bit
     endianess of the target
  */
  bool  m_lsb_bit;
  bool  m_msb_bit;

  public:
  /* smt_*
     section meta-types
  */
  enum smt {
      null,
      text,             // code
      data,             // generic data
      idata = data,     // small, initialised data
      rodata,           // small, read-only data
      bss,              // small data initialised with zeroes
      zdata = bss,      
      udata,            // small, uninitialised data
      got,
      raw,              // large, opaque data (custom)
      rel,
      rela,
      comment,          // typically love letters from the ELF manufacturer
      symtab,           // symbol table
      strtab,           // generic string table
      shstrtab          // section header string table
  };

  public:
          target(unsigned int, unsigned int, bool = false, bool = false) noexcept;
          target(const target&) noexcept = delete;
          target(target&&) noexcept = delete;
          ~target();

          unsigned int  get_machine_type() const noexcept;
          bool          has_machine_type(unsigned int) const noexcept;
          unsigned int  get_abi_id() const noexcept;
          unsigned int  get_abi_version() const noexcept;
          unsigned int  get_class() const noexcept;
          bool          has_class(unsigned int) const noexcept;
          int           get_machine_word_bits() const noexcept;
          int           get_machine_word_size() const noexcept;

          const char*   get_ep_name() const noexcept;
          const char*   get_got_name() const noexcept;
          const char*   get_segment_name(int) const noexcept;
          int           get_address_align() const noexcept;
          long int      get_address_mask() const noexcept;
          std::uint8_t* get_address_base() const noexcept;
          int           get_code_align() const noexcept;
          int           get_data_align() const noexcept;
          int           get_default_align() const noexcept;
          long int      get_vle_bit() const noexcept;
          long int      get_vle_mask() const noexcept;

          bool          is_vle() const noexcept;

          bool          is_lsb() const noexcept;
          bool          is_msb() const noexcept;

          target& operator=(const target&) noexcept;
          target& operator=(target&&) noexcept;
};
  
/*namespace uld*/ }
#endif
