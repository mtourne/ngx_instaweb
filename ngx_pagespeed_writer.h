/**
 * Copyright (c) 2010, 2011 CloudFlare, Inc. (http://www.cloudflare.com)
 * Copyright (c) 2010, 2011 Ray Bejjani <ray.bejjani@gmail.com>
 *
 * @author Ray Bejjani<ray@cloudflare.com>
 */

#ifndef NGX_PAGESPEED_WRITER_H_
#define NGX_PAGESPEED_WRITER_H_


#include "net/instaweb/util/public/basictypes.h"
#include "net/instaweb/util/public/string.h"
#include "net/instaweb/util/public/string_util.h"
#include "net/instaweb/util/public/writer.h"

extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
  #include <ngx_http.h>
}

#include "ngx_http_instaweb_module.h"

namespace net_instaweb {

class MessageHandler;

// Writer implementation for directing HTML output to nginx output.
class NgxPagespeedWriter : public Writer {
 public:
  explicit NgxPagespeedWriter(ngx_http_request_t *request, ngx_http_output_body_filter_pt ngx_http_next_body_filter, ngx_http_instaweb_ctx_t *ctx);
  virtual ~NgxPagespeedWriter();
  virtual bool Write(const StringPiece& str, MessageHandler* message_handler);
  virtual bool Flush(MessageHandler* message_handler);

  // returns status of a flushes to nginx
  ngx_int_t GetWriteStatus();


 private:
  DISALLOW_COPY_AND_ASSIGN(NgxPagespeedWriter);
  ngx_http_request_t *request;
  ngx_http_output_body_filter_pt    ngx_http_next_body_filter;
  ngx_http_instaweb_ctx_t *ctx;
  ngx_int_t write_status;
};

}  // namespace net_instaweb

#endif  // NGX_PAGESPEED_WRITER_H_
