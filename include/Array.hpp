//
// Array.hpp
// GOLRenderer
//
// Created by Usama Alshughry 10.07.2025.
// Copyright © 2025 Usama Alshughry. All rights reserved.
//

#ifndef ARRAY_HPP_
#define ARRAY_HPP_

#include <cstddef>
#include <cstdio>
#include <cstdlib>

template <typename T, size_t N>
class Array
{
  T* raw_data = nullptr;
  // T raw_data[N];
public:
  Array()
  : raw_data{0}
  {
    raw_data = new T[N];
    memset(raw_data, 0, sizeof(T) * N);
  }

  Array(const Array&) = delete;
  Array& operator=(const Array&) = delete;

  ~Array() {
    delete[] raw_data;
  }

  static constexpr size_t ByteCapacity() { return sizeof(T) * N; }
  static constexpr size_t size() { return N; }

  T& operator[](size_t i) { return raw_data[i]; }
  T const& operator[](size_t i) const { return raw_data[i]; }

  T* begin() { return &raw_data[0]; }
  T* end  () { return &raw_data[N]; }
  T const* begin() const { return &raw_data[0]; }
  T const* end  () const { return &raw_data[N]; }
  T const* cbegin() const { return &raw_data[0]; }
  T const* cend  () const { return &raw_data[N]; }
  T* data() { return raw_data; }
  T const* data() const { return raw_data; }

};


#endif // ARRAY_HPP_
