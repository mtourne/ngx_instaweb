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
    ctx->out_head = ctx->out_tail = NULL;
    write_status = NGX_OK;
}

NgxPagespeedWriter::~NgxPagespeedWriter() {
}

bool NgxPagespeedWriter::Write(const StringPiece& str, MessageHandler* handler) {
  char* data;
  ngx_buf_t    *b;
  ngx_chain_t   *added_link;

  data = (char *)ngx_pcalloc(request->pool, str.size());
  if (data == NULL) {
    ngx_log_error(NGX_LOG_ERR, request->connection->log, 0, "NgxPagespeedWriter::Write Failed to allocate response buffer.");
    return NGX_HTTP_INTERNAL_SERVER_ERROR;
  }
  memcpy(data, str.data(), str.size());

  b = (ngx_buf_t *)ngx_pcalloc(request->pool, sizeof(ngx_buf_t));
  if (b == NULL) {
    ngx_log_error(NGX_LOG_ERR, request->connection->log, 0, "NgxPagespeedWriter::Write Failed to allocate response buffer holder.");
    return NGX_HTTP_INTERNAL_SERVER_ERROR;
  }

  b->pos = b->start = (u_char *)str.data();
  b->last = b->end = b->start + str.size();

  added_link = ngx_alloc_chain_link(request->pool);
  if (added_link == NULL) {
      return NGX_ERROR;
  }

  added_link->buf = b;
  added_link->next = NULL;

  // if this is the first element on output, setup the head pointer
  if(ctx->out_head == NULL) {
      ctx->out_head = ctx->out_tail = added_link;
  } else {
      ctx->out_tail->next = added_link;
      ctx->out_tail->buf->last_in_chain = 0;
      ctx->out_tail = added_link;
  }
  ctx->out_tail->buf->last_in_chain = 1; // mark last in chain as such

  return true;
}


bool NgxPagespeedWriter::Flush(MessageHandler* message_handler) {
    return true;
}

ngx_int_t NgxPagespeedWriter::GetWriteStatus() {
    return write_status;
}

}  // namespace net_instaweb
