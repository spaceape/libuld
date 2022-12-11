#ifndef uld_image_table_h
#define uld_image_table_h
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
#include "pool.h"
#include <cstring>

namespace uld {

template<typename Xt, int PageSize = page_size>
class table: public pool<Xt, PageSize>
{
  public:
  using   pool_type = pool<Xt, PageSize>;
  using   page_type = pool_type::page_type;
  using   node_type = pool_type::node_type;

  class iterator
  {
    page_type* m_page;
    node_type* m_node;

    friend  class table<Xt, PageSize>;
    private:
    inline  iterator(page_type* page, node_type* node) noexcept:
            m_page(page),
            m_node(node) {
    }

    public:
    inline  iterator() noexcept:
            m_page(nullptr),
            m_node(nullptr) {
    }

    inline  iterator(const iterator& copy) noexcept:
            m_page(copy.m_page),
            m_node(copy.m_node) {
    }

    inline  iterator(iterator&& copy) noexcept:
            iterator(copy) {
    }

    inline  ~iterator() {
    }

    inline  node_type* get_node_ptr() const noexcept {
            if((m_page != nullptr) &&
                (m_node != nullptr) &&
                (m_node >= m_page->get_base_ptr()) &&
                (m_node < m_page->get_next_ptr())) {
                return m_node;
            }
            return nullptr;
    }

    inline  void  advance() noexcept {
            while(m_page) {
                ++m_node;
                if(m_node < m_page->get_next_ptr()) {
                    return;
                }
                m_page = m_page->m_page_next;
                if(m_page) {
                    m_node = m_page->get_node_ptr(-1);
                } else
                    m_node = nullptr;
            }
    }

    inline  void  retreat() noexcept {
            while(m_page) {
                if(m_node > m_page->get_base_ptr()) {
                    --m_node;
                    return;
                }
                m_page = m_page->m_page_prev;
                if(m_page) {
                    m_node = m_page->get_free_ptr();
                }
            }
    }

    inline  void  advance(int count) noexcept {
            // TODO: write this properly
            while(count > 0) {
                advance();
                --count;
            }
            while(count < 0) {
                retreat();
                ++count;
            }
    }

    inline  bool  is_defined() const noexcept {
            return get_node_ptr() != nullptr;
    }

    inline  bool  is_defined(bool expect) const noexcept {
            if(expect) {
                return get_node_ptr() != nullptr;
            } else
                return get_node_ptr() == nullptr;
    }

    inline  operator node_type*() const noexcept {
            return  get_node_ptr();
    }

    inline  node_type& operator*() const noexcept {
            return  *get_node_ptr();
    }

    inline  node_type* operator->() const noexcept {
            return  get_node_ptr();
    }

    inline  iterator& operator++() noexcept {
            advance();
            return *this;
    }

    inline  iterator operator++(int) noexcept {
            advance();
            return *this;
    }

    inline  iterator& operator--() noexcept {
            retreat();
            return *this;
    }

    inline  iterator operator--(int) noexcept {
            retreat();
            return *this;
    }

    inline  iterator& operator=(const iterator& rhs) noexcept {
            m_page = rhs.m_page;
            m_node = rhs.m_node;
    }

    inline  iterator& operator=(const iterator&& rhs) noexcept {
            m_page = rhs.m_page;
            m_node = rhs.m_node;
    }
  };

  public:
  inline  table() noexcept:
          pool_type() {
  }

  inline  ~table() {
  }

  inline  iterator begin() const noexcept {
          return   iterator(pool_type::m_page_head, pool_type::m_page_head->get_base_ptr());
  }

  inline  iterator end() const noexcept {
          return   iterator(nullptr, nullptr);
  }
};

/*namespace uld*/ }
#endif
