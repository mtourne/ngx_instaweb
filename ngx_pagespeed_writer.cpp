/**
 * Copyright (c) 2010, 2011 CloudFlare, Inc. (http://www.cloudflare.com)
 * Copyright (c) 2010, 2011 Ray Bejjani <ray.bejjani@gmail.com>
 *
 * @author Ray Bejjani<ray@cloudflare.com>
 */

extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
  #include <ngx_http.h>
}

#include "ngx_pagespeed_writer.h"

#include "net/instaweb/util/public/string.h"
#include "net/instaweb/util/public/string_util.h"

namespace net_instaweb {

class MessageHandler;

NgxPagespeedWriter::NgxPagespeedWriter(ngx_http_request_t *request, ngx_http_output_body_filter_pt ngx_http_next_body_filter, ngx_http_instaweb_ctx_t *ctx) :
        request(request),
        ngx_http_next_body_filter(ngx_http_next_body_filter),
        ctx(ctx) {
    write_status = NGX_OK;
}

NgxPagespeedWriter::~NgxPagespeedWriter() {
}

bool NgxPagespeedWriter::Write(const StringPiece& str, MessageHandler* handler) {
    const char  *cdata;
    ngx_chain_t *cl;
    size_t      bytes_remaining;
    size_t      write_size;

    cdata = str.data();
    bytes_remaining = str.size();

    while (bytes_remaining) {

        if (ctx->out_buf == NULL
            || !ctx->out_buf->temporary // mtourne: why ?
            || ctx->out_buf->last >= ctx->out_buf->end) {
            if (ctx->free) {
                cl = ctx->free;
                ctx->free = ctx->free->next;
                ctx->out_buf = cl->buf;
            } else {
                ctx->out_buf = ngx_create_temp_buf(request->pool, ngx_pagesize);
                if (ctx->out_buf == NULL) {
                    // TODO (mtourne): log
                    return false;
                }

                cl = ngx_alloc_chain_link(request->pool);
                if (cl == NULL) {
                    return false;
                }

                ctx->out_buf->tag = (ngx_buf_tag_t) &ngx_http_instaweb_module;
                cl->buf = ctx->out_buf;
            }
            cl->next = NULL;
            *ctx->last_out = cl;
            ctx->last_out = &cl->next;
        }
        write_size = ctx->out_buf->end - ctx->out_buf->last;
        if (write_size > bytes_remaining) {
            write_size = bytes_remaining;
        }
        ngx_memcpy(ctx->out_buf->last, cdata, write_size);
        cdata += write_size;
        ctx->out_buf->last += write_size;
        bytes_remaining -= write_size;
    }

    return true;
}


bool NgxPagespeedWriter::Flush(MessageHandler* message_handler) {
    return true;
}

ngx_int_t NgxPagespeedWriter::GetWriteStatus() {
    return write_status;
}

}  // namespace net_instaweb