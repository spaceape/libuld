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
#include "cache.h"
#include <util.h>
#include <log.h>
#include <cstring>

constexpr int ids_reserve_min = 128;  // how much memory to reserve initially
constexpr int ids_reserve_max = 1024; // how much memory to reserve at most [[not implemented]]

constexpr int ids_lock_max = 127;

namespace uld {
namespace util {

      data_cache_t::data_cache_t(file_ptr file, int reserve_size) noexcept:
      m_file_ptr(file),
      m_data_ptr(nullptr),
      m_read_offset(0),
      m_data_index(0),
      m_read_size(0),
      m_data_size(0),
      m_lock_count(0)
{
      reserve(reserve_size);
}

      data_cache_t::~data_cache_t()
{
      if(m_data_ptr != nullptr) {
          free(m_data_ptr);
      }
}

/* ids_fetch()
   fetch at least `count` bytes from file into the internal data store
*/
int   data_cache_t::ids_fetch(std::size_t count) noexcept
{
      if(int
          l_load_size = static_cast<int>(count);
          l_load_size > 0) {
          // compute where in file we would be after current operation completes
          int  l_bump_offset = m_data_index + l_load_size;
          // compute how much a full read of `count` bytes would overflow our current buffer
          int  l_over_size = l_bump_offset - m_read_size;
          // fetch the data, if we don't have all of it already
          if(l_over_size > 0) {
              int  l_keep_size = m_read_size - m_data_index;
              // optimize seeks and loads;
              // when in lock mode, we are allowed to resize the buffer but not change any file- or internal offsets
              // NOTE: could be smarter about this, but for now, when we overflow let's just keep only as much data as to
              // fetch the new data in without having to resize
              if(l_keep_size > 0) {
                  if(m_lock_count == 0) {
                      m_read_offset += l_over_size;
                      m_data_index  -= l_over_size;
                      m_read_size   -= l_over_size;
                      l_bump_offset -= l_over_size;
                      l_keep_size   -= l_over_size;
                      l_over_size    = 0;
                  }
              }
              // reserve memory for the incoming data
              if(bool
                  l_reserve_success = reserve(m_read_size + l_over_size);
                  l_reserve_success == false) {
                  printdbg(
                      "[%p] data cache failed to resize to %d bytes.",
                      __FILE__,
                      __LINE__,
                      this,
                      m_read_size + l_over_size
                  );
                  return -1;
              }
              // go to the appropriate position within the file
              unsigned int l_read_size;
              if(FRESULT
                  l_rc = f_lseek(m_file_ptr, m_read_offset + l_keep_size);
                  l_rc != FR_OK) {
                  return -1;
              }
              // adjust the read size such that we fill as much data as possible into the the local buffer
              if(l_bump_offset < m_data_size) {
                  l_over_size = m_data_size - l_keep_size;
              }
              // ...and finally, read in the new data
              if(FRESULT
                  l_rc = f_read(m_file_ptr, m_data_ptr + l_keep_size, l_over_size, std::addressof(l_read_size));
                  l_rc != FR_OK) {
                  return -1;
              }
              m_read_size = l_keep_size + l_read_size;
              return l_read_size;
          }
      }
      return count;
}

char* data_cache_t::at(int offset) noexcept
{
      return reinterpret_cast<char*>(m_data_ptr + offset);
}

std::uint8_t data_cache_t::raw_get() noexcept
{
      int l_fetch_size = ids_fetch(1);
      if(l_fetch_size >= 1) {
          return m_data_ptr[m_data_index++];
      }
      return EOF;
}

int  data_cache_t::raw_get(std::uint8_t* data, std::size_t size) noexcept
{
      int l_fetch_size = ids_fetch(size);
      if(l_fetch_size > 0) {
          std::memcpy(data, m_data_ptr + m_data_index, size);
          m_data_index += size;
          return size;
      }
      return 0;
}

/* ids_seek()
   seek internal file pointer at the absolute position in file given by `position`
*/
int   data_cache_t::seek(int position) noexcept
{
      int l_file_offset = position;
      int l_diff_offset = l_file_offset - m_read_offset;
      // plainly refuse to seek if locked
      if(m_lock_count == 0) {
          // new file offset points inside our buffer: just advance the internal offset
          if(l_diff_offset > 0) {
              if(l_diff_offset < m_read_size) {
                  m_data_index = l_diff_offset;
                  return m_read_offset + m_data_index;
              }
          }
          // new file offset points outside the boundaries of our internally read data, reset the buffer
          FRESULT  l_rc = f_lseek(m_file_ptr, position);
          if(l_rc == FR_OK) {
              m_read_offset = position;
              m_data_index  = 0;
              m_read_size   = 0;
              return m_read_offset;
          }
      } else
      if((m_data_size == 0) ||
          (l_diff_offset == 0)) {
          return position;
      }
      return -1;
}

bool  data_cache_t::acquire(int& save_offset) noexcept
{
      if(__builtin_expect(m_lock_count < ids_lock_max, true)) {
          save_offset = m_data_index;
          ++m_lock_count;
          return true;
      }
      printdbg(
          "[%p] data cache out of sync, `m_lock_count` should be greater than zero!",
          __FILE__,
          __LINE__,
          this
      );
      return false;
}

char* data_cache_t::release(int& save_offset) noexcept
{
      if(__builtin_expect(m_lock_count == 0, false)) {
          printdbg(
              "[%p] data cache out of sync, `m_lock_count` should be greater than zero!",
              __FILE__,
              __LINE__,
              this
          );
          return nullptr;
      }
      --m_lock_count;
      return reinterpret_cast<char*>(m_data_ptr + save_offset);
}

bool  data_cache_t::release(int& save_offset, char* return_ptr) noexcept
{
      if(__builtin_expect(m_lock_count == 0, false)) {
          printdbg(
              "[%p] data cache out of sync, `m_lock_count` should be greater than zero!",
              __FILE__,
              __LINE__,
              this
          );
          return_ptr = nullptr;
          return false;
      }
      return_ptr = reinterpret_cast<char*>(m_data_ptr + save_offset);
      --m_lock_count;
      return true;
}

bool  data_cache_t::release(int& save_offset, std::uint8_t* return_ptr) noexcept
{
      if(__builtin_expect(m_lock_count == 0, false)) {
          printdbg(
              "[%p] data cache out of sync, `m_lock_count` should be greater than zero!",
              __FILE__,
              __LINE__,
              this
          );
          return_ptr = nullptr;
          return false;
      }
      return_ptr = m_data_ptr + save_offset;
      --m_lock_count;
      return true;
}

bool  data_cache_t::reserve(int size) noexcept
{
      if(size > 0) {
          if(size > m_data_size) {
              int   l_data_size = get_round_value(size, ids_reserve_min);
              void* l_data_ptr  = realloc(m_data_ptr, l_data_size);
              if(l_data_ptr != nullptr) {
                  m_data_ptr = reinterpret_cast<std::uint8_t*>(l_data_ptr);
                  m_data_size = l_data_size;
              }
          }
      }
      return m_data_size >= size;
}

/*namespace util*/ }
/*namespace uld*/ }
