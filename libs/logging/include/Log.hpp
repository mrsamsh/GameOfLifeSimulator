//
// Log.hpp
// logging
//
// Created by Usama Alshughry 08.07.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#ifndef LOG_HPP_
#define LOG_HPP_

#include <fmt/format.h>
#include <iostream>
#include <ctime>

namespace log
{

static inline char const* getTime()
{
  static char time[9];
  std::time_t t = std::time(nullptr);
  std::strftime(time, 8, "%T", std::localtime(&t));
  return time;
}

template <typename ... Args>
inline
void logHelper(std::string_view level, std::string_view file, int line,
    fmt::format_string<Args...> fmt, Args&& ... args)
{
  std::cout << fmt::format("[{}] ({:>15}:{:<5})[{}] ",
        getTime(),
        file, line,
        level);
  std::cout << fmt::format(fmt, std::forward<Args>(args)...) << std::endl;
}

template <typename ... Args>
inline void println(fmt::format_string<Args...> fmt = "", Args&& ... args)
{
  std::cout << fmt::format(fmt, std::forward<Args>(args)...) << std::endl;
}

#define TRACE(message, ...)                                                    \
  log::logHelper("Trace", __FILE_NAME__, __LINE__, message, ##__VA_ARGS__)

#define FATAL(message, ...)                                                    \
  do {                                                                         \
    log::logHelper("Fatal", __FILE_NAME__, __LINE__, message, ##__VA_ARGS__);  \
    abort(); } while(0)

} // namespace log

#endif // LOG_HPP_
