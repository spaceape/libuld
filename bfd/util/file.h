#ifndef uld_bfd_util_file_h
#define uld_bfd_util_file_h
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
#include <ff.h>

namespace uld {
namespace util {

class file_ptr
{
  private:
  class   file: public FIL
  {
    int   m_hooks;

    public:
            file() noexcept;
            file(const file&) noexcept = delete;
            file(file&&) noexcept = delete;
            ~file();
            file* hook() noexcept;
            file* drop() noexcept;
            file& operator=(const file&) noexcept = delete;
            file& operator=(file&&) noexcept = delete;
  };

  private:
  file*     m_ptr;

  public:
          file_ptr() noexcept;
          file_ptr(const file_ptr&) noexcept;
          file_ptr(file_ptr&&) noexcept;
          ~file_ptr();

  static  file_ptr make_file_cb() noexcept;

          void release() noexcept;
          void dispose(bool = true) noexcept;

  inline  operator FIL*() noexcept {
          return m_ptr;
  }

  inline  operator bool() const noexcept {
          return m_ptr != nullptr;
  }

          file_ptr& operator=(const file_ptr&) noexcept;
          file_ptr& operator=(file_ptr&&) noexcept;
};

/*namespace util*/ }
/*namespace uld*/ }
#endif
