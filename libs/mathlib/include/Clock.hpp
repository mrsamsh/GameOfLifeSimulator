//
// Clock.hpp
// mathlib
//
// Created by Usama Alshughry 21.07.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#ifndef CLOCK_HPP_
#define CLOCK_HPP_

#include "Time.hpp"
#include <chrono>

namespace math
{

class Clock
{
private:
  using clock = std::chrono::high_resolution_clock;
  using time_point = std::chrono::high_resolution_clock::time_point;
public:
  static Time Now(bool reset = false)
  {
    static time_point start = clock::now();
    if (reset)
    {
      Time ret_val = nanoseconds((clock::now() - start).count());
      start = clock::now();
      return ret_val;
    }
    return nanoseconds((clock::now() - start).count());
  }

  static constexpr auto Reset = [](){ return Now(true);};
};

} // namespace math

#endif // CLOCK_HPP_
