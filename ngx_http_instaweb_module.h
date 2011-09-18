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

    FilterDriver        *driver;
    NgxPagespeedWriter  *writer;

    /* IO */
    ngx_buf_t		*buf;
    u_char		*pos;

    ngx_buf_t           *out_buf;

    ngx_chain_t         *in;
    ngx_chain_t         *free;
    ngx_chain_t         *busy;
    ngx_chain_t         *out;
    ngx_chain_t         **last_out;
} ngx_http_instaweb_ctx_t;

} //!namespace net_instaweb

extern ngx_module_t ngx_http_instaweb_module;

#endif /* !NGX_HTTP_INSTAWEB_MODULE_H_ */
