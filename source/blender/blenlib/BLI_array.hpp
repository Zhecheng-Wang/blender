/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/** \file
 * \ingroup bli
 *
 * This is a container that contains a fixed size array. Note however, the size of the array is not
 * a template argument. Instead it can be specified at the construction time.
 */

#pragma once

#include "BLI_utildefines.h"
#include "BLI_allocator.hpp"
#include "BLI_array_ref.hpp"
#include "BLI_memory.hpp"

namespace BLI {

template<typename T, typename Allocator = GuardedAllocator> class Array {
 private:
  T *m_data;
  uint m_size;
  Allocator m_allocator;

 public:
  Array()
  {
    m_data = nullptr;
    m_size = 0;
  }

  explicit Array(uint size)
  {
    m_data = this->allocate(size);
    m_size = size;

    for (uint i = 0; i < m_size; i++) {
      new (m_data + i) T();
    }
  }

  Array(const Array &other)
  {
    m_size = 0;
    m_allocator = other.m_allocator;

    if (m_size == 0) {
      m_data = nullptr;
      return;
    }
    else {
      m_data = this->allocate(m_size);
      copy_n(other.begin(), m_size, m_data);
    }
  }

  Array(Array &&other)
  {
    m_data = other.m_data;
    m_size = other.m_size;
    m_allocator = other.m_allocator;

    other.m_data = nullptr;
    other.m_size = 0;
  }

  ~Array()
  {
    destruct_n(m_data, m_size);
    if (m_data != nullptr) {
      m_allocator.deallocate((void *)m_data);
    }
  }

  Array &operator=(const Array &other)
  {
    if (this == &other) {
      return *this;
    }

    this->~Array();
    new (this) Array(other);
    return *this;
  }

  Array &operator=(Array &&other)
  {
    if (this == &other) {
      return *this;
    }

    this->~Array();
    new (this) Array(std::move(other));
    return *this;
  }

  operator ArrayRef<T>() const
  {
    return ArrayRef<T>(m_data, m_size);
  }

  operator MutableArrayRef<T>()
  {
    return MutableArrayRef<T>(m_data, m_size);
  }

  T &operator[](uint index)
  {
    BLI_assert(index < m_size);
    return m_data[index];
  }

  uint size() const
  {
    return m_size;
  }

  void fill(const T &value)
  {
    MutableArrayRef<T>(*this).fill(value);
  }

  void fill_indices(ArrayRef<uint> indices, const T &value)
  {
    MutableArrayRef<T>(*this).fill_indices(indices, value);
  }

  T *begin()
  {
    return m_data;
  }

  T *end()
  {
    return m_data + m_size;
  }

 private:
  T *allocate(uint size)
  {
    return (T *)m_allocator.allocate_aligned(
        size * sizeof(T), std::alignment_of<T>::value, __func__);
  }
};

template<typename T> using TemporaryArray = Array<T, TemporaryAllocator>;

}  // namespace BLI
