/**
 * Copyright (c) 2010, 2011 CloudFlare, Inc. (http://www.cloudflare.com)
 * Copyright (c) 2010, 2011 Matthieu Tourne <matthieu.tourne@gmail.com>
 *
 * @author Matthieu Tourne <matthieu@cloudflare.com>
 */

#include "ngx_message_handler.h"

namespace {

const char kModuleName[] = "instaweb";

}

namespace net_instaweb {

NgxMessageHandler::NgxMessageHandler(ngx_log_t *log)
    : log_(log) {
    // empty
}

ngx_uint_t
NgxMessageHandler::GetNgxLogLevel(MessageType type) {
    switch (type) {
        case kInfo:
            return NGX_LOG_INFO;
        case kWarning:
            return NGX_LOG_WARN;
        case kError:
            return NGX_LOG_ERR;
        case kFatal:
            return NGX_LOG_CRIT;
    }
}

void
NgxMessageHandler::MessageVImpl(MessageType type, const char* msg, va_list args) {
    ngx_uint_t  log_level = GetNgxLogLevel(type);
    std::string formatted_message = Format(msg, args);

    ngx_log_error(log_level, log_, 0, "[%s] %s",
                  kModuleName, formatted_message.c_str());
}

void
NgxMessageHandler::FileMessageVImpl(MessageType type, const char* filename,
                                    int line, const char* msg, va_list args) {

    ngx_uint_t  log_level = GetNgxLogLevel(type);
    std::string formatted_message = Format(msg, args);

    ngx_log_error(log_level, log_, 0, "[%s] %s:%d: %s",
                  kModuleName, filename, line,
                  formatted_message.c_str());
}

// copied from apache_message_handler.cc :
std::string NgxMessageHandler::Format(const char* msg, va_list args) {
  std::string buffer;

  // Ignore the name of this routine: it formats with vsnprintf.
  // See base/stringprintf.cc.
  StringAppendV(&buffer, msg, args);
  return buffer;
}

}
