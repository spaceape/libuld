#ifndef uld_image_page_h
#define uld_image_page_h
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
#include <config.h>
#include <type_traits>
#include <limits>

namespace uld {

template<typename Xt>
struct page_base_t
{
  using node_type = std::remove_cv_t<Xt>;

  public:
  int                 m_used;         // count of occupied elements (and also index of the first free element)
  int                 m_gto_base;     // global table offset at the beginning of the page
  int                 m_gto_next;     // global table offset at the end of the allocated space
  int                 m_size;         // capacity
  page_base_t*        m_page_prev;
  page_base_t*        m_page_next;
  node_type           m_node_list[0];

  public:

  /* raw_get()
     reserve `count` contiguous elements from the list
  */
          node_type*  raw_get(int count = 1) noexcept {
          if(count > 0) {
              node_type* l_result;
              int        l_node_base = m_used;
              int        l_node_last = m_used + count;
              if(l_node_last <= m_size) {
                  l_result = std::addressof(m_node_list[l_node_base]);
                  m_gto_next = m_gto_next + count * sizeof(node_type);
                  m_used     = l_node_last;
                  return l_result;
              }
          }
          return nullptr;
  }

  inline  node_type*  get_base_ptr() noexcept {
          return std::addressof(m_node_list[0]);
  }

  inline  node_type*  get_node_ptr(int index) noexcept {
          return get_base_ptr() + index;
  }

  inline  node_type*  get_next_ptr() noexcept {
          return get_node_ptr(m_used);
  }

  inline  int    get_used_count() const noexcept {
          return m_used;
  }

  inline  int    get_free_count() const noexcept {
          return m_size - m_used;
  }

  inline  int    get_capacity() const noexcept {
          return m_size;
  }
};

template<typename Xt, int PageSize = page_size>
struct page: public page_base_t<Xt>
{
  using base_type = page_base_t<Xt>;
  using node_type = std::remove_cv_t<Xt>;

  /* get_header_size()
     compute header size such that the first element meets its native alignment
  */
  static constexpr int get_header_size() noexcept {
         int l_head_size = sizeof(base_type);
         int l_align_size = alignof(node_type);
         int l_align_mask = l_align_size - 1;
         if(l_align_mask > 0) {
            if(l_head_size & l_align_mask) {
                l_head_size &= ~(l_align_mask);
                l_head_size += l_align_size;
            }
         }
         return l_head_size;
  }

  static constexpr int head_size = get_header_size();
  static constexpr int node_size = sizeof(node_type);

  static constexpr int get_capacity(int size) noexcept {
         return (size - head_size) / node_size;
  }

  static constexpr int size     = PageSize;

  static constexpr int capacity = get_capacity(size);

  static constexpr int capacity_min = 4;

  // sanity check: size should at least be the size the header
  static_assert(size >= sizeof(page_base_t<Xt>), "PageSize should be at least the size of `page_base_t`");

  // sanity check: make sure that PageSize allows for at least 4 elements
  static_assert(capacity > capacity_min, "PageSize should allow for at least `capacity_min` elements");

  static  bool  make_page(base_type*& page, base_type* prev, base_type* next, int count) noexcept {
          base_type*  l_page_ptr = nullptr;
          int         l_page_count = count / capacity;
          int         l_byte_count;
          if(count % capacity) {
              l_page_count += 1;
          }
          l_byte_count = l_page_count * size;
          if(l_byte_count > 0) {
              void* l_byte_ptr = malloc(l_byte_count);
              if(l_byte_ptr != nullptr) {
                  l_page_ptr = reinterpret_cast<base_type*>(l_byte_ptr);
                  l_page_ptr->m_used = 0;
                  l_page_ptr->m_gto_base = 0;
                  l_page_ptr->m_gto_next = 0;
                  l_page_ptr->m_size = get_capacity(l_byte_count);
                  l_page_ptr->m_page_prev = prev;
                  if(l_page_ptr->m_page_prev != nullptr) {
                      l_page_ptr->m_gto_base = l_page_ptr->m_page_prev->m_gto_next;
                      l_page_ptr->m_gto_next = l_page_ptr->m_gto_base;
                      l_page_ptr->m_page_prev->m_page_next = l_page_ptr;
                  }
                  l_page_ptr->m_page_next = next;
                  if(l_page_ptr->m_page_next != nullptr) {
                      l_page_ptr->m_page_next->m_page_prev = l_page_ptr;
                  }
              }
              page = l_page_ptr;
          }
          return l_page_ptr != nullptr;
  }

  static  void  free_page(base_type*& page) noexcept {
          free(page);
  }
};

/*namespace uld*/ }
#endif
