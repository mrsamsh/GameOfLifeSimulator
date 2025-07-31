//
// Random.hpp
// mathlib
//
// Created by Usama Alshughry 24.07.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#ifndef RANDOM_HPP_
#define RANDOM_HPP_

#include <random>
#include <algorithm>
#include "MyTypes.hpp"
#include "MathConcepts.hpp"

namespace math
{

class RandomGeneraor
{
  template <Arithmetic T>
  using distribution_t = std::conditional_t<
      FloatingPoint<T>,
      std::uniform_real_distribution<T>,
      std::uniform_int_distribution<T>
  >;
public:
  RandomGeneraor() : m_randomDevice{}, m_generator{m_randomDevice()} { }

  template <Arithmetic T>
  inline constexpr T generate(T const min, T const max)
  {
    distribution_t<T> distribution(min, max);
    return distribution(m_generator);
  }

  inline constexpr u32 generate()
  {
    return m_generator();
  }

  template <Arithmetic T>
  inline constexpr T pseudoGenerate(T const min, T const max)
  {
    T const range = max - min;
    if constexpr (FloatingPoint<T>)
    {
      return (static_cast<T>(rand()) / static_cast<T>(RAND_MAX)) * range + min;
    }
    else
    {
      return (rand() % range) + min;
    }
  }

  template <typename Container>
  inline constexpr void shuffle(Container&& container)
  {
    std::shuffle(container.begin(), container.end(), m_generator);
  }

  template <typename Container>
  inline constexpr auto& chooseOne(Container&& container)
  {
    return container[generate<usz>(0, container.size() - 1)];
  }

private:
  std::random_device m_randomDevice;
  std::mt19937 m_generator;
};

} // namespace math

#endif // RANDOM_HPP_
