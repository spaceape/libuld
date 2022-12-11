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
#include "file.h"
#include <cstring>
#include <limits>

namespace uld {
namespace util {

      file_ptr::file_ptr() noexcept:
      m_ptr(nullptr)
{
}

      file_ptr::file_ptr(const file_ptr& copy) noexcept:
      file_ptr()
{
      if(copy.m_ptr) {
          m_ptr = copy.m_ptr->hook();
      }
}

      file_ptr::file_ptr(file_ptr&& copy) noexcept:
      file_ptr()
{
      if(copy.m_ptr) {
          m_ptr = copy.m_ptr;
      }
      copy.release();
}

      file_ptr::~file_ptr()
{
      dispose();
}

      file_ptr::file::file() noexcept:
      m_hooks(1)
{
      std::memset(static_cast<FIL*>(this), 0, sizeof(FIL));
}

      file_ptr::file::~file()
{
      f_close(this);
}

file_ptr::file* file_ptr::file::hook() noexcept
{
      if(m_hooks < std::numeric_limits<int>::max()) {
          m_hooks++;
          return this;
      }
      return nullptr;
}

file_ptr::file* file_ptr::file::drop() noexcept
{
      if(m_hooks > 0) {
          m_hooks--;
          if(m_hooks == 0) {
              delete this;
          }
      }
      return nullptr;
}

file_ptr file_ptr::make_file_cb() noexcept
{
      file_ptr  l_file_ptr;
      l_file_ptr.m_ptr = new(std::nothrow) file;
      return    l_file_ptr;
}

void  file_ptr::release() noexcept
{
      m_ptr = nullptr;
}

void  file_ptr::dispose(bool clear) noexcept
{
      if(m_ptr) {
          m_ptr = m_ptr->drop();
      }
}

file_ptr& file_ptr::operator=(const file_ptr& rhs) noexcept
{
      if(std::addressof(rhs) != this) {
          dispose();
          if(rhs.m_ptr) {
              m_ptr = rhs.m_ptr->hook();
          }
      }
      return *this;
}

file_ptr& file_ptr::operator=(file_ptr&& rhs) noexcept
{
      if(std::addressof(rhs) != this) {
          dispose();
          if(rhs.m_ptr) {
              m_ptr = rhs.m_ptr;
              rhs.release();
          }
      }
      return *this;
}

/*namespace util*/ }
/*namespace uld*/ }
