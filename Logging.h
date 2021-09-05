
#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <list>
#include <sstream>
#include <string>
#include <utility>

#if defined(WEBRTC_MAC) && !defined(WEBRTC_IOS)
#include <CoreServices/CoreServices.h>
#endif
#define LOG_IS_ON 1
namespace utils {

///////////////////////////////////////////////////////////////////////////////
// ConstantLabel can be used to easily generate string names from constant
// values.  This can be useful for logging descriptive names of error messages.
// Usage:
//   const ConstantLabel LIBRARY_ERRORS[] = {
//     KLABEL(SOME_ERROR),
//     KLABEL(SOME_OTHER_ERROR),
//     ...
//     LASTLABEL
//   }
//
//   int err = LibraryFunc();
//   LOG(LS_ERROR) << "LibraryFunc returned: "
//                 << ErrorName(err, LIBRARY_ERRORS);

struct ConstantLabel { int value; const char * label; };
#define KLABEL(x) { x, #x }
#define TLABEL(x, y) { x, y }
#define LASTLABEL { 0, 0 }

const char* FindLabel(int value, const ConstantLabel entries[]);
std::string ErrorName(int err, const ConstantLabel* err_table);

#if defined(WEBRTC_MAC) && !defined(WEBRTC_IOS)
// Returns a UTF8 description from an OS X Status error.
std::string DescriptionFromOSStatus(OSStatus err);
#endif

//////////////////////////////////////////////////////////////////////

// Note that the non-standard LoggingSeverity aliases exist because they are
// still in broad use.  The meanings of the levels are:
//  LS_SENSITIVE: Information which should only be logged with the consent
//   of the user, due to privacy concerns.
//  LS_VERBOSE: This level is for data which we do not want to appear in the
//   normal debug log, but should appear in diagnostic logs.
//  LS_INFO: Chatty level used in debugging for all sorts of things, the default
//   in debug builds.
//  LS_WARNING: Something that may warrant investigation.
//  LS_ERROR: Something that should not have occurred.
//  LS_NONE: Don't log.
enum LoggingSeverity {
  LS_SENSITIVE,
  LS_VERBOSE,
  LS_INFO,
  LS_WARNING,
  LS_ERROR,
  LS_NONE,
  INFO = LS_INFO,
  WARNING = LS_WARNING,
  LERROR = LS_ERROR
};

// LogErrorContext assists in interpreting the meaning of an error value.
enum LogErrorContext {
  ERRCTX_NONE,
  ERRCTX_ERRNO,     // System-local errno
  ERRCTX_HRESULT,   // Windows HRESULT
  ERRCTX_OSSTATUS,  // MacOS OSStatus

  // Abbreviations for LOG_E macro
  ERRCTX_EN = ERRCTX_ERRNO,     // LOG_E(sev, EN, x)
  ERRCTX_HR = ERRCTX_HRESULT,   // LOG_E(sev, HR, x)
  ERRCTX_OS = ERRCTX_OSSTATUS,  // LOG_E(sev, OS, x)
};

// Virtual sink interface that can receive log messages.
class LogSink {
 public:
  LogSink() {}
  virtual ~LogSink() {}
  virtual void OnLogMessage(const std::string& message) = 0;
};

class LogMessage {
 public:
  LogMessage(const char* file, int line, LoggingSeverity sev,
             LogErrorContext err_ctx = ERRCTX_NONE, int err = 0,
             const char* module = NULL);

  LogMessage(const char* file,
             int line,
             LoggingSeverity sev,
             const std::string& tag);

  ~LogMessage();

  static inline bool Loggable(LoggingSeverity sev) { return (sev >= min_sev_); }
  std::ostream& stream() { return print_stream_; }

  // Returns the time at which this function was called for the first time.
  // The time will be used as the logging start time.
  // If this is not called externally, the LogMessage ctor also calls it, in
  // which case the logging start time will be the time of the first LogMessage
  // instance is created.
  static uint32_t LogStartTime();

  // Returns the wall clock equivalent of |LogStartTime|, in seconds from the
  // epoch.
  static uint32_t WallClockStartTime();

  //  LogThreads: Display the thread identifier of the current thread
  static void LogThreads(bool on = true);

  //  LogTimestamps: Display the elapsed time of the program
  static void LogTimestamps(bool on = true);

  // These are the available logging channels
  //  Debug: Debug console on Windows, otherwise stderr
  static void LogToDebug(LoggingSeverity min_sev);
  static LoggingSeverity GetLogToDebug() { return dbg_sev_; }

  // Sets whether logs will be directed to stderr in debug mode.
  static void SetLogToStderr(bool log_to_stderr);

  //  Stream: Any non-blocking stream interface.  LogMessage takes ownership of
  //   the stream. Multiple streams may be specified by using AddLogToStream.
  //   LogToStream is retained for backwards compatibility; when invoked, it
  //   will discard any previously set streams and install the specified stream.
  //   GetLogToStream gets the severity for the specified stream, of if none
  //   is specified, the minimum stream severity.
  //   RemoveLogToStream removes the specified stream, without destroying it.
  static int GetLogToStream(LogSink* stream = NULL);
  static void AddLogToStream(LogSink* stream, LoggingSeverity min_sev);
  static void RemoveLogToStream(LogSink* stream);

  // Testing against MinLogSeverity allows code to avoid potentially expensive
  // logging operations by pre-checking the logging level.
  static int GetMinLogSeverity() { return min_sev_; }

  // Parses the provided parameter stream to configure the options above.
  // Useful for configuring logging from the command line.
  static void ConfigureLogging(const char* params);

 private:
  typedef std::pair<LogSink*, LoggingSeverity> StreamAndSeverity;
  typedef std::list<StreamAndSeverity> StreamList;

  // Updates min_sev_ appropriately when debug sinks change.
  static void UpdateMinLogSeverity();

  // These write out the actual log messages.
  static void OutputToDebug(const std::string& msg,
                            LoggingSeverity severity,
                            const std::string& tag);

  // The ostream that buffers the formatted message before output
  std::ostringstream print_stream_;

  // The severity level of this message
  LoggingSeverity severity_;

  // The Android debug output tag.
  std::string tag_;

  // String data generated in the constructor, that should be appended to
  // the message before output.
  std::string extra_;

  // dbg_sev_ is the thresholds for those output targets
  // min_sev_ is the minimum (most verbose) of those levels, and is used
  //  as a short-circuit in the logging macros to identify messages that won't
  //  be logged.
  // ctx_sev_ is the minimum level at which file context is displayed
  static LoggingSeverity min_sev_, dbg_sev_, ctx_sev_;

  // The output streams and their associated severities
  static StreamList streams_;

  // Flags for formatting options
  static bool thread_, timestamp_;

  // Determines if logs will be directed to stderr in debug mode.
  static bool log_to_stderr_;

  LogMessage(const LogMessage&) = delete;
  LogMessage& operator=(const LogMessage&) = delete;
};

//////////////////////////////////////////////////////////////////////
// Logging Helpers
//////////////////////////////////////////////////////////////////////

class LogMultilineState {
 public:
  size_t unprintable_count_[2];
  LogMultilineState() {
    unprintable_count_[0] = unprintable_count_[1] = 0;
  }
};

// When possible, pass optional state variable to track various data across
// multiple calls to LogMultiline.  Otherwise, pass NULL.
void LogMultiline(LoggingSeverity level, const char* label, bool input,
                  const void* data, size_t len, bool hex_mode,
                  LogMultilineState* state);

#ifndef LOG

// The following non-obvious technique for implementation of a
// conditional log stream was stolen from google3/base/logging.h.

// This class is used to explicitly ignore values in the conditional
// logging macros.  This avoids compiler warnings like "value computed
// is not used" and "statement has no effect".

class LogMessageVoidify {
 public:
  LogMessageVoidify() { }
  // This has to be an operator with a precedence lower than << but
  // higher than ?:
  void operator&(std::ostream&) { }
};

#define LOG_SEVERITY_PRECONDITION(sev) \
  !(utils::LogMessage::Loggable(sev)) \
    ? (void) 0 \
    : utils::LogMessageVoidify() &

#define LOG(sev) \
  LOG_SEVERITY_PRECONDITION(utils::sev) \
    utils::LogMessage(__FILE__, __LINE__, utils::sev).stream()

// The _V version is for when a variable is passed in.  It doesn't do the
// namespace concatination.
#define LOG_V(sev) \
  LOG_SEVERITY_PRECONDITION(sev) \
    utils::LogMessage(__FILE__, __LINE__, sev).stream()

// The _F version prefixes the message with the current function name.
#if (defined(__GNUC__) && !defined(NDEBUG)) || defined(WANT_PRETTY_LOG_F)
#define LOG_F(sev) LOG(sev) << __PRETTY_FUNCTION__ << ": "
#define LOG_T_F(sev) LOG(sev) << this << ": " << __PRETTY_FUNCTION__ << ": "
#else
#define LOG_F(sev) LOG(sev) << __FUNCTION__ << ": "
#define LOG_T_F(sev) LOG(sev) << this << ": " << __FUNCTION__ << ": "
#endif

#define LOG_CHECK_LEVEL(sev) \
  utils::LogCheckLevel(utils::sev)
#define LOG_CHECK_LEVEL_V(sev) \
  utils::LogCheckLevel(sev)

inline bool LogCheckLevel(LoggingSeverity sev) {
  return (LogMessage::GetMinLogSeverity() <= sev);
}

#define LOG_E(sev, ctx, err, ...) \
  LOG_SEVERITY_PRECONDITION(utils::sev) \
    utils::LogMessage(__FILE__, __LINE__, utils::sev, \
                          utils::ERRCTX_ ## ctx, err , ##__VA_ARGS__) \
        .stream()

#define LOG_T(sev) LOG(sev) << this << ": "

#define LOG_ERRNO_EX(sev, err) \
  LOG_E(sev, ERRNO, err)
#define LOG_ERRNO(sev) \
  LOG_ERRNO_EX(sev, errno)

#if defined(WEBRTC_WIN)
#define LOG_GLE_EX(sev, err) \
  LOG_E(sev, HRESULT, err)
#define LOG_GLE(sev) \
  LOG_GLE_EX(sev, GetLastError())
#define LOG_GLEM(sev, mod) \
  LOG_E(sev, HRESULT, GetLastError(), mod)
#define LOG_ERR_EX(sev, err) \
  LOG_GLE_EX(sev, err)
#define LOG_ERR(sev) \
  LOG_GLE(sev)
#define LAST_SYSTEM_ERROR \
  (::GetLastError())
#elif __native_client__
#define LOG_ERR_EX(sev, err) \
  LOG(sev)
#define LOG_ERR(sev) \
  LOG(sev)
#define LAST_SYSTEM_ERROR \
  (0)
#elif defined(WEBRTC_POSIX)
#define LOG_ERR_EX(sev, err) \
  LOG_ERRNO_EX(sev, err)
#define LOG_ERR(sev) \
  LOG_ERRNO(sev)
#define LAST_SYSTEM_ERROR \
  (errno)
#endif  // WEBRTC_WIN

#define LOG_TAG(sev, tag) \
  LOG_SEVERITY_PRECONDITION(sev) \
    utils::LogMessage(NULL, 0, sev, tag).stream()

#define PLOG(sev, err) \
  LOG_ERR_EX(sev, err)

// TODO(?): Add an "assert" wrapper that logs in the same manner.

#endif  // LOG

}  // namespace utils

#ifndef __ANDROID__
#define LOGV if (LOG_IS_ON && (utils::LoggingSeverity::LS_VERBOSE <= utils::LogMessage::GetMinLogSeverity())) \
		utils::LogMessage(__FILE__, __LINE__, utils::LoggingSeverity::LS_VERBOSE).stream()
#define LOGI if (LOG_IS_ON && (utils::LoggingSeverity::LS_INFO <= utils::LogMessage::GetMinLogSeverity())) \
		utils::LogMessage(__FILE__, __LINE__, utils::LoggingSeverity::LS_INFO).stream()
#define LOGW if (LOG_IS_ON && (utils::LoggingSeverity::LS_WARNING <= utils::LogMessage::GetMinLogSeverity())) \
		utils::LogMessage(__FILE__, __LINE__, utils::LoggingSeverity::LS_WARNING).stream()
#define LOGE if (LOG_IS_ON) utils::LogMessage(__FILE__, __LINE__, utils::LoggingSeverity::LS_ERROR).stream()
#define LOGS if (LOG_IS_ON) utils::LogMessage(__FILE__, __LINE__, utils::LoggingSeverity::LS_ERROR, utils::LogErrorContext::ERRCTX_ERRNO, errno).stream()
#else
#define LOGV(TAG) if (LOG_IS_ON && (utils::LoggingSeverity::LS_VERBOSE <= utils::LogMessage::GetMinLogSeverity())) \
		utils::LogMessage(__FILE__, __LINE__, LoggingSeverity::LS_VERBOSE, TAG).stream()
#define LOGI(TAG) if (LOG_IS_ON && (utils::LoggingSeverity::LS_INFO <= utils::LogMessage::GetMinLogSeverity())) \
		utils::LogMessage(__FILE__, __LINE__, LoggingSeverity::LS_INFO, TAG).stream()
#define LOGW(TAG) if (LOG_IS_ON && (utils::LoggingSeverity::LS_WARNING <= utils::LogMessage::GetMinLogSeverity())) \
		utils::LogMessage(__FILE__, __LINE__, utils::LoggingSeverity::LS_WARNING, TAG).stream()
#define LOGE(TAG) if (LOG_IS_ON) utils::LogMessage(__FILE__, __LINE__, utils::LoggingSeverity::LS_ERROR, TAG).stream()
#define LOGS(TAG) if (LOG_IS_ON) utils::LogMessage(__FILE__, __LINE__, utils::LoggingSeverity::LS_ERROR, utils::LogErrorContext::ERRCTX_ERRNO, errno).stream()
#endif



#endif  // __LOGGING_H__
