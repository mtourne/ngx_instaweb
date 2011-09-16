/**
 * Copyright (c) 2010, 2011 CloudFlare, Inc. (http://www.cloudflare.com)
 * Copyright (c) 2010, 2011 Matthieu Tourne <matthieu.tourne@gmail.com>
 *
 * @author Matthieu Tourne <matthieu@cloudflare.com>
 */

#ifndef NGX_HTTP_INSTAWEB_MODULE_H_
#define NGX_HTTP_INSTAWEB_MODULE_H_

extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
  #include <ngx_http.h>
}

#include "ngx_message_handler.h"

namespace net_instaweb {
class FilterDriver;
class NgxPagespeedWriter;

typedef struct {
    /* IO */
    FilterDriver        *driver;
    NgxPagespeedWriter  *writer;

    ngx_chain_t   *out_head; // output chain pointers, used by NgxPagespeedWriter
    ngx_chain_t   *out_tail;

    ngx_chain_t         *in;
    ngx_chain_t         *free;
    ngx_chain_t         *busy;
    ngx_chain_t         *out;
} ngx_http_instaweb_ctx_t;

} //!namespace net_instaweb

#endif /* !NGX_HTTP_INSTAWEB_MODULE_H_ */
