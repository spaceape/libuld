#ifndef uld_bfd_util_cache_h
#define uld_bfd_util_cache_h
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
#include <os.h>
#include "file.h"

namespace uld {
namespace util {

/* data_cache_t
   utility to manage a small memory cache of the file data at a certain offset, in order to optimize performance
*/
class data_cache_t
{
  file_ptr      m_file_ptr;
  std::uint8_t* m_data_ptr;
  int           m_read_offset;      // file offset
  int           m_data_index;       // buffer offset
  int           m_read_size;
  int           m_data_size;
  int           m_lock_count:8;

  private:
          int  ids_seek() noexcept;
          int  ids_fetch(std::size_t) noexcept;


  public:
        data_cache_t(file_ptr, int = 0) noexcept;
        ~data_cache_t();

  inline  int  lsb_get() noexcept {
          return 0;
  }

  inline  int  msb_get() noexcept {
          return 0;
  }

          char*        at(int) noexcept;

          std::uint8_t raw_get() noexcept;

          int          raw_get(std::uint8_t*, std::size_t) noexcept;

  template<typename Xt>
  inline  int  raw_get(Xt* object) noexcept {
          auto        l_object_ptr  = reinterpret_cast<std::uint8_t*>(object);
          std::size_t l_object_size = sizeof(Xt);
          std::size_t l_read_size   = raw_get(l_object_ptr, l_object_size);
          return l_read_size;
  }

  template<typename... Args>
  inline  int  lsb_get(char& value, Args&&... next) noexcept {
          return  raw_get(std::addressof(value)) + lsb_get(std::forward<Args>(next)...);
  }

  template<typename... Args>
  inline  int  msb_get(char& value, Args&&... next) noexcept {
          return  raw_get(std::addressof(value)) + msb_get(std::forward<Args>(next)...);
  }

  template<typename... Args>
  inline  int  lsb_get(unsigned char& value, Args&&... next) noexcept {
          return  raw_get(std::addressof(value)) + lsb_get(std::forward<Args>(next)...);
  }

  template<typename... Args>
  inline  int  msb_get(unsigned char& value, Args&&... next) noexcept {
          return  raw_get(std::addressof(value)) + msb_get(std::forward<Args>(next)...);
  }

  /* lsb_get<type>
     get an object of given <type> in little endian; this is meant for arithmetic types, use read() and alike for structs.
  */
  template<typename Xt, typename... Args>
  inline  int  lsb_get(Xt* value, Args&&... next) noexcept {    
          int  l_size = sizeof(Xt);
          auto l_data = reinterpret_cast<std::uint8_t*>(value);
          if constexpr (os::is_lsb) {
              for(int l_index = 0; l_index < l_size; l_index++) {
                  l_data[l_index] = raw_get();
              }
          } else
          if constexpr (os::is_msb) {
              for(int l_index = l_size - 1; l_index >= 0; l_index--) {
                  l_data[l_index] = raw_get();
              }
          }
          return lsb_get(std::forward<Args>(next)...) + l_size;
  }

  /* msb_get<type>
     get an object of given <type> in big endian; this is meant for arithmetic types, use read() and alike for structs.
  */
  template<typename Xt, typename... Args>
  inline  int  msb_get(Xt* value, Args&&... next) noexcept {    
          int  l_size = sizeof(Xt);
          auto l_data = reinterpret_cast<std::uint8_t*>(value);
          if constexpr (os::is_lsb) {
              for(int l_index = l_size - 1; l_index >= 0; l_index--) {
                  l_data[l_index] = raw_get();
              }
          } else
          if constexpr (os::is_msb) {
              for(int l_index = 0; l_index < l_size; l_index++) {
                  l_data[l_index] = raw_get();
              }
          }
          return msb_get(std::forward<Args>(next)...) + l_size;
  }

  template<typename Xt, typename... Args>
  inline  int  lsb_get(Xt& value, Args&&... next) noexcept {
          return lsb_get(std::addressof(value), std::forward<Args>(next)...);
  }

  template<typename Xt, typename... Args>
  inline  int  msb_get(Xt& value, Args&&... next) noexcept {
          return msb_get(std::addressof(value), std::forward<Args>(next)...);
  }

        int    seek(int) noexcept;

        bool   acquire(int&) noexcept;
        char*  release(int&) noexcept;
        bool   release(int&, char*) noexcept;
        bool   release(int&, std::uint8_t*) noexcept;


        bool   reserve(int) noexcept;
};

/*namespace util*/ }
/*namespace uld*/ }
#endif
