#ifndef _YAMMER_LOGGER_H_
#define _YAMMER_LOGGER_H_

#include <time.h>
#include <stdio.h>
#include <sstream>

// A global FILE pointer and macro for logging a message along with
// the time, file, and line number.
//
// Example:
//   Yammer::Logger::fp_ = fopen("log", "w"); 
//   LOG("debug msg");

namespace Yammer {

  struct Logger {
    static FILE *fp_;
  };
 
}

#define LOG(msg)                                     \
{                                                    \
  time_t t_ = time(0);                               \
  tm tm_;                                            \
  localtime_r(&t_, &tm_);                            \
  char ts_[32];                                      \
  strftime(ts_, sizeof ts_, "[%Y-%m-%d %T ", &tm_);  \
  std::stringstream ss_;                             \
  ss_ << ts_ << __FILE__ << ':' << __LINE__ << "] "  \
      << msg << '\n';                                \
  fprintf(Yammer::Logger::fp_, ss_.str().c_str());   \
  fflush(Yammer::Logger::fp_);                       \
}

#endif
