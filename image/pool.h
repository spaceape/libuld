#ifndef uld_image_pool_h
#define uld_image_pool_h
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
#include "page.h"
#include <util.h>

namespace uld {

template<typename Xt, int PageSize = page_size>
class pool
{
  public:
  using  page_type = typename page<Xt, PageSize>::base_type;
  using  node_type = typename page<Xt, PageSize>::node_type;

  protected:
  page_type*  m_page_head;
  page_type*  m_page_tail;
  page_type*  m_page_current;
  int         m_page_count;

  public:
  inline  pool() noexcept:
          m_page_head(nullptr),
          m_page_tail(nullptr),
          m_page_current(nullptr),
          m_page_count(0) {
  }

          pool(const pool&) noexcept = delete;
          pool(pool&&) noexcept = delete;

  inline  ~pool() {
          page_type* l_page_prev;
          page_type* l_page_iter = m_page_tail;
          while(l_page_iter != nullptr) {
              l_page_prev = l_page_iter->m_page_prev;
              page<node_type, PageSize>::free_page(l_page_iter);
              l_page_iter = l_page_prev;
          }
  }

  /* raw_get()
     reserve an element from the pool
  */
          node_type*  raw_get() noexcept {
          node_type*  l_node;
          if(m_page_current != nullptr) {
              l_node = m_page_current->raw_get();
              if(l_node != nullptr) {
                  return l_node;
              }
              m_page_current = m_page_current->m_page_next;
          }
          if(m_page_current == nullptr) {
              if(bool
                  l_alloc_success = page<Xt, PageSize>::make_page(m_page_current, m_page_tail, nullptr, 1);
                  l_alloc_success == true) {
                  m_page_tail = m_page_current;
                  if(m_page_head == nullptr) {
                      m_page_head = m_page_current;
                  }
                  m_page_count++;
              }
          }
          return raw_get();
  }

          pool& operator=(const pool&) noexcept = delete;
          pool& operator=(pool&&) noexcept = delete;
};

template<int PageSize>
class pool<char, PageSize>
{
  public:
  using  page_type = typename page<char, PageSize>::base_type;
  using  data_type = typename page<char, PageSize>::node_type;

  protected:
  /* chr_reserve_min
     strings from the pool here will be allocated a size that is a multiple of this value
  */
  static constexpr int  chr_reserve_min = 8;

  protected:
  page_type*  m_page_head;
  page_type*  m_page_tail;
  page_type*  m_page_current;
  int         m_page_count;

  public:
  inline  pool() noexcept:
          m_page_head(nullptr),
          m_page_tail(nullptr),
          m_page_current(nullptr),
          m_page_count(0) {
  }

          pool(const pool&) noexcept = delete;
          pool(pool&&) noexcept = delete;

  inline  ~pool() {
          page_type* l_page_prev;
          page_type* l_page_iter = m_page_tail;
          while(l_page_iter != nullptr) {
              l_page_prev = l_page_iter->m_page_prev;
              page<data_type, PageSize>::free_page(l_page_iter);
              l_page_iter = l_page_prev;
          }
  }

  /* raw_get()
     reserve `size` contiguous characters from the pool
  */
          data_type*  raw_get(int count) noexcept {
          do {
              if(__builtin_expect(m_page_current == nullptr, false)) {
                  if(bool
                      l_alloc_success = page<data_type, PageSize>::make_page(m_page_current, m_page_tail, nullptr, count);
                      l_alloc_success == true) {
                      m_page_tail = m_page_current;
                      if(m_page_head == nullptr) {
                          m_page_head = m_page_current;
                      }
                      m_page_count++;
                  }
              }
              if(__builtin_expect(m_page_current != nullptr, true)) {
                  int        l_char_count = get_round_value(count, chr_reserve_min);
                  data_type* l_char_base  = m_page_current->raw_get(l_char_count);
                  if(l_char_base != nullptr) {
                      return  l_char_base;
                  }
                  m_page_current = m_page_current->m_page_next;
              }
          }
          while(true);
          return nullptr;
  }

  /**/    data_type* get_offset_ptr(int offset) noexcept {
          if(offset >= 0) {
              page_type*  i_page_ptr    = m_page_head;
              int         l_page_offset = offset;
              while(i_page_ptr) {
                  if(l_page_offset < i_page_ptr->m_gto_next) {
                      return i_page_ptr->get_node_ptr(l_page_offset);
                  }
                  l_page_offset -= i_page_ptr->m_gto_next;
                  i_page_ptr     = i_page_ptr->m_page_next;
              }
          }
          return nullptr;
  }

          pool& operator=(const pool&) noexcept = delete;
          pool& operator=(pool&&) noexcept = delete;
};

template<int PageSize>
class pool<std::uint8_t, PageSize>
{
  public:
  using  page_type = typename page<std::uint8_t, PageSize>::base_type;
  using  data_type = typename page<std::uint8_t, PageSize>::node_type;

  protected:
  page_type*  m_page_head;
  page_type*  m_page_tail;
  page_type*  m_page_current;
  int         m_page_count;
  int         m_align;

  public:
  inline  pool(int align = 0) noexcept:
          m_page_head(nullptr),
          m_page_tail(nullptr),
          m_page_current(nullptr),
          m_page_count(0),
          m_align(align) {
  }

          pool(const pool&) noexcept = delete;
          pool(pool&&) noexcept = delete;

  inline  ~pool() {
          page_type* l_page_prev;
          page_type* l_page_iter = m_page_tail;
          while(l_page_iter != nullptr) {
              l_page_prev = l_page_iter->m_page_prev;
              page<data_type, PageSize>::free_page(l_page_iter);
              l_page_iter = l_page_prev;
          }
  }

  /* raw_get()
     get `size` bytes from the pool, at the set alignment
  */
          data_type*  raw_get(int size) noexcept {
          int         l_data_offset = 0;
          int         l_data_count = get_round_value(size, 1 << m_align);
          do {
              if(__builtin_expect(m_page_current == nullptr, false)) {
                  if(bool
                      l_alloc_success = page<data_type, PageSize>::make_page(m_page_current, m_page_tail, nullptr, size);
                      l_alloc_success == true) {
                      m_page_tail = m_page_current;
                      if(m_page_head == nullptr) {
                          m_page_head = m_page_current;
                      }
                      m_page_count++;
                  }
              }
              if(__builtin_expect(m_page_current != nullptr, true)) {
                  if((m_align >= 1) &&
                      (m_align <= 8)) {
                      int         l_align_bits = 1 << m_align;
                      int         l_align_mask = l_align_bits - 1;
                      data_type*  l_data_ptr   = m_page_current->get_next_ptr();
                      std::size_t l_data_addr  = reinterpret_cast<std::size_t>(l_data_ptr);
                      std::size_t l_addr_mask  = l_data_addr & l_align_mask;
                      if(l_addr_mask) {
                          l_data_offset  = l_align_bits - l_addr_mask;
                          l_data_count  += l_data_offset;
                      }
                  }
                  data_type* l_data_base = m_page_current->raw_get(l_data_count);
                  if(l_data_base != nullptr) {
                      data_type* l_data_ptr = l_data_base + l_data_offset;
                      return     l_data_ptr;
                  }
                  m_page_current = m_page_current->m_page_next;
              }
          }
          while(true);
          return nullptr;
  }

  inline  int  get_table_offset() const noexcept {
          if(m_page_current != nullptr) {
              return m_page_current->m_gto_next;
          }
          return 0;
  }

  inline  std::uint8_t* get_table_ptr(int offset) noexcept {
          if(offset >= 0) {
              page_type*  i_page_ptr    = m_page_head;
              int         l_page_offset = offset;
              while(i_page_ptr) {
                  if((l_page_offset >= i_page_ptr->m_gto_base) &&
                      (l_page_offset < i_page_ptr->m_gto_next)) {
                      return i_page_ptr->get_node_ptr(l_page_offset - i_page_ptr->m_gto_base);
                  }
                  i_page_ptr     = i_page_ptr->m_page_next;
              }
          }
          return nullptr;
  }

  inline  std::uint8_t* get_base_ptr() noexcept {
          if(m_page_current != nullptr) {
              return m_page_current->get_base_ptr();
          }
          return nullptr;
  }

  inline  std::uint8_t* get_next_ptr() noexcept {
          if(m_page_current != nullptr) {
              return m_page_current->get_next_ptr();
          }
          return nullptr;
  }

          pool& operator=(const pool&) noexcept = delete;
          pool& operator=(pool&&) noexcept = delete;
};

/*namespace uld*/ }
#endif
