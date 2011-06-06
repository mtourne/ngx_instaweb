/**
 * Copyright (c) 2010, 2011 CloudFlare, Inc. (http://www.cloudflare.com)
 * Copyright (c) 2010, 2011 Matthieu Tourne <matthieu.tourne@gmail.com>
 *
 * @author Matthieu Tourne <matthieu@cloudflare.com>
 */

#ifndef NGX_MESSAGE_HANDLER_H_
#define NGX_MESSAGE_HANDLER_H_

#include <string>
#include "ngx_http_instaweb_module.h"
#include "net/instaweb/util/public/message_handler.h"
#include "base/string_util.h"

namespace net_instaweb {

class NgxMessageHandler : public MessageHandler {
 public:
    NgxMessageHandler(ngx_log_t *log);

 protected:
      virtual void MessageVImpl(MessageType type, const char* msg, va_list args);

      virtual void FileMessageVImpl(MessageType type, const char* filename,
                                    int line, const char* msg, va_list args);
 private:
      ngx_uint_t GetNgxLogLevel(MessageType type);
      std::string Format(const char* msg, va_list args);

      ngx_log_t *log_;
};

}

#endif // !NGX_MESSAGE_HANDLER_H_
