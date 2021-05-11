#pragma once

#include <sstream>


enum class logflag : std::uint32_t
{
  info                = (1 << 0),
  debug               = (1 << 1),
  warn                = (1 << 2),
  error               = (1 << 3),
  nostdout            = (1 << 4),
  spaces              = (1 << 5),
  newline             = (1 << 6),
  round_brackets      = (1 << 7),
  box_bracekts        = (1 << 8),
  curly_brackets      = (1 << 9),
  chevrons            = (1 << 10),
  unixtime            = (1 << 11),
  no_sync_with_stdio  = (1 << 12),
#if defined(__linux__) || defined(__FreeBSD__)
  red                 = (1 << 13),
  green               = (1 << 14),
  yellow              = (1 << 15),
  purple              = (1 << 16),
  gray                = (1 << 17)
#endif
};

inline logflag operator | (logflag lhs, logflag rhs) noexcept
{
  return static_cast<logflag>(static_cast<std::uint32_t>(lhs) | static_cast<std::uint32_t>(rhs));
}

inline std::uint32_t operator |= (std::uint32_t& lhs, logflag rhs) noexcept
{
  return lhs |= static_cast<std::uint32_t>(rhs);
}

inline std::uint32_t operator &= (std::uint32_t& lhs, logflag rhs) noexcept
{
  return lhs &= static_cast<std::uint32_t>(rhs);
}

inline std::uint32_t operator ~ (logflag lhs) noexcept
{
  return ~static_cast<std::uint32_t>(lhs);
}

inline std::uint32_t operator & (std::uint32_t lhs, logflag rhs) noexcept
{
  return lhs & static_cast<std::uint32_t>(rhs);
}

class logger
{
public:
  logger()                          = delete;
  logger(const logger&)             = delete;
  logger(logger&&)                  = delete;
  logger& operator=(logger&&)       = delete;
  logger& operator=(const logger&)  = delete;

  explicit logger(logflag);
  explicit logger(std::string_view, logflag);
  ~logger();

  template <typename T>
  inline logger&  operator<<(T&& data) & = delete;
  template <typename T>
  inline logger&& operator<<(T&& data) &&;

private:
  inline std::string type()                     & noexcept;
  inline std::string colorize(std::string_view) &;
  inline std::string unixtime()           const &;
  inline std::string time()               const &;
  inline void        logging()                  &;

#if !defined(__linux__) && !defined(__FreeBSD__)
  constexpr
#endif
  inline std::string_view begin_color()   const & noexcept;

#if !defined(__linux__) && !defined(__FreeBSD__)
  constexpr
#endif
  inline std::string_view end_color()     const & noexcept;

  static inline
  std::stringstream   ss;
  std::uint32_t       flags;
  std::string_view    path;

#if defined(__linux__) || defined(__FreeBSD__)
  static inline constexpr std::string_view red    = "\033[0;31m";
  static inline constexpr std::string_view green  = "\033[0;32m";
  static inline constexpr std::string_view yellow = "\033[0;33m";
  static inline constexpr std::string_view purple = "\033[0;35m";
  static inline constexpr std::string_view gray   = "\033[0;90m";
  static inline constexpr std::string_view eoc    = "\033[0;0m";
#endif
};

template<typename T>
inline logger&& logger::operator<<(T&& data) &&
{
  if ( (flags & logflag::newline         ) > 0) ss << '\n';

  if ( (flags & logflag::box_bracekts    ) > 0) ss << '[';
  if ( (flags & logflag::chevrons        ) > 0) ss << '<';
  if ( (flags & logflag::curly_brackets  ) > 0) ss << '{';
  if ( (flags & logflag::round_brackets  ) > 0) ss << '(';

  ss << data;

  if ( (flags & logflag::round_brackets  ) > 0) ss << ')';
  if ( (flags & logflag::curly_brackets  ) > 0) ss << '}';
  if ( (flags & logflag::chevrons        ) > 0) ss << '>';
  if ( (flags & logflag::box_bracekts    ) > 0) ss << ']';

  if ( (flags & logflag::spaces          ) > 0) ss << ' ';

  return std::move(*this);
}


#include <iostream>
#include <fstream>
#include <ctime>
#include <mutex>
#include <cstring>


logger::logger(logflag logflags)
  : flags   (static_cast<std::uint32_t>(logflags))
  , path    ("")
{ }

logger::logger(std::string_view filename, logflag logflags)
  : flags   (static_cast<std::uint32_t>(logflags))
  , path    (filename)
{ }

std::string logger::time() const &
{
  long now = std::time(nullptr);

  std::string time(25, 0);
  std::strftime(&time[0], time.size(), "[%Y-%m-%d %H:%M:%S]", std::localtime(&now));

  return time;
}

std::string logger::unixtime() const &
{
  return "[" + std::to_string(std::time(nullptr)) + "]";
}

#if !defined(__linux__) && !defined(__FreeBSD__)
constexpr
#endif
std::string_view logger::begin_color() const & noexcept
{
#if defined(__linux__) || defined(__FreeBSD__)
  if ( (flags & logflag::red     ) > 0) return red;
  if ( (flags & logflag::green   ) > 0) return green;
  if ( (flags & logflag::purple  ) > 0) return purple;
  if ( (flags & logflag::yellow  ) > 0) return yellow;
  if ( (flags & logflag::gray    ) > 0) return gray;
#endif
  return "";
}

#if !defined(__linux__) && !defined(__FreeBSD__)
constexpr
#endif
std::string_view logger::end_color() const & noexcept
{
#if defined(__linux__) || defined(__FreeBSD__)
  if ( (flags & logflag::red     ) > 0) return eoc;
  if ( (flags & logflag::green   ) > 0) return eoc;
  if ( (flags & logflag::purple  ) > 0) return eoc;
  if ( (flags & logflag::yellow  ) > 0) return eoc;
  if ( (flags & logflag::gray    ) > 0) return eoc;
#endif
  return "";
}

std::string logger::type() & noexcept
{
  static const logflag all_except_green  = logflag::gray | logflag::red   | logflag::purple | logflag::yellow;
  static const logflag all_except_gray   = logflag::red  | logflag::green | logflag::purple | logflag::yellow;
  static const logflag all_except_yellow = logflag::red  | logflag::green | logflag::purple | logflag::gray;
  static const logflag all_except_red    = logflag::gray | logflag::green | logflag::purple | logflag::yellow;

  auto any_of = [&](auto... args) -> bool { return (static_cast<bool>(flags & args) | ... ); };

  std::string flaglist;

  if ( (flags & logflag::info ) > 0) { flaglist += "[ INFO   ]"; if (!any_of(all_except_green  )) { flags |= logflag::green;  } }
  if ( (flags & logflag::debug) > 0) { flaglist += "[ DEBUG  ]"; if (!any_of(all_except_gray   )) { flags |= logflag::gray;   } }
  if ( (flags & logflag::warn ) > 0) { flaglist += "[ WARN   ]"; if (!any_of(all_except_yellow )) { flags |= logflag::yellow; } }
  if ( (flags & logflag::error) > 0) { flaglist += "[ ERROR  ]"; if (!any_of(all_except_red    )) { flags |= logflag::red;    } }

  return flaglist;
}

std::string logger::colorize(std::string_view text) &
{
  return std::string { begin_color() } + std::string { text } + std::string { end_color() };
}

void logger::logging() &
{
  std::string source;
  (((flags & logflag::unixtime) > 0))
    ? source = unixtime() + ' ' + type() + ' ' + ss.str() + '\n'
    : source =     time() + ' ' + type() + ' ' + ss.str() + '\n';

  if (!((flags & logflag::no_sync_with_stdio) > 0))
  {
    std::ios::sync_with_stdio(false);
  }
  if (!((flags & logflag::nostdout) > 0))
  {
    std::cout << colorize(source) << std::flush;
  }

  if (!path.empty())
  {
    static std::ofstream ofs(path.data(), std::ios::app);
    ofs.write(source.c_str(), source.size());
  }
  ss.str("");
  ss.clear();
}

logger::~logger()
{
  static std::mutex mtx;
  std::lock_guard<decltype(mtx)> guard(mtx);
  if (ss.rdbuf()->in_avail())
    logging();
}
