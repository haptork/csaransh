/*!
 * @file
 * helper functions for general usage
 * */

#ifndef LOGGER_CSARANSH_HPP
#define LOGGER_CSARANSH_HPP

#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace csaransh {

enum class LogMode : int {
  none = 0x00,
  info = 0x01,
  warning = 0x02,
  error = 0x04,
  debug = 0x08,
  all = 0x0f,
};

using LogModeT = std::underlying_type_t<LogMode>;

inline LogModeT operator|(LogMode lhs, LogMode rhs) {
  return (LogModeT)(static_cast<LogModeT>(lhs) | static_cast<LogModeT>(rhs));
}

inline LogModeT operator|(LogMode lhs, LogModeT rhs) {
  return (LogModeT)(static_cast<LogModeT>(lhs) | rhs);
}

inline LogModeT operator|(LogModeT rhs, LogMode lhs) {
  return (LogModeT)(static_cast<LogModeT>(lhs) | rhs);
}

inline auto operator&(LogMode lhs, LogMode rhs) {
  return bool(static_cast<LogModeT>(lhs) & static_cast<LogModeT>(rhs));
}

inline auto operator&(LogMode lhs, LogModeT rhs) {
  return bool(static_cast<LogModeT>(lhs) & rhs);
}

inline auto operator&(LogModeT rhs, LogMode lhs) {
  return bool(static_cast<LogModeT>(lhs) & rhs);
}

class Logger {

private:
  LogModeT _logMode{LogMode::error | LogMode::warning};
  std::string _fname;
  std::unique_ptr<std::filebuf> _fb;
  std::ostream *_os;

  Logger() {
    _os = &std::cout;
    _fb = nullptr;
  }

  ~Logger() {
    if (_fb) _fb->close();
    // resetting
    _fb = nullptr;
    if (_os != &std::cout) { delete _os; }
  }

  auto printTime() const {
    std::time_t result = std::time(nullptr);
    (*_os) << std::asctime(std::localtime(&result));
  }

public:
  static Logger &inst() {
    static Logger inst;
    return inst;
  };

  auto file(std::string fname) {
    if (fname.empty()) return;
    _fname = fname;
    if (!_fb) _fb = std::make_unique<std::filebuf>();
    if (_fb->is_open()) { _fb->close(); }
    _fb->open(_fname, std::fstream::out | std::fstream::app);
    if (!_fb->is_open()) {
      log_error("Can not write to file " + _fname);
      return;
    }
    _os = new std::ostream(_fb.get());
  }

  // A rudimentary log to be extended later.
  void log_info(std::string msg) const {
    if (!(LogMode::info & _logMode)) return;
    (*_os) << "info ";
    printTime();
    (*_os) << msg << '\n' << std::endl;
  }

  // A rudimentary log to be extended later.
  void log_warning(std::string msg) const {
    if (!(LogMode::warning & _logMode)) return;
    (*_os) << "warning ";
    printTime();
    (*_os) << msg << '\n' << std::endl;
  }

  // A rudimentary log to be extended later.
  void log_error(std::string msg) const {
    if (!(LogMode::error & _logMode)) return;
    (*_os) << "error ";
    printTime();
    (*_os) << msg << '\n' << std::endl;
  }

  // A rudimentary log to be extended later.
  void log_debug(std::string msg) const {
    if (!(LogMode::info & _logMode)) return;
    (*_os) << "debug ";
    printTime();
    (*_os) << msg << '\n' << std::endl;
  }

  void mode(LogMode mode) { _logMode = (mode | LogMode::none); }

  void mode(LogModeT mode) { _logMode = mode; }

  const LogModeT &mode() const { return _logMode; }
};
} // namespace csaransh
#endif
