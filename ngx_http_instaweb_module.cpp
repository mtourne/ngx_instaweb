/**
 * Copyright (c) 2010, 2011 CloudFlare, Inc. (http://www.cloudflare.com)
 * Copyright (c) 2010, 2011 Matthieu Tourne <matthieu.tourne@gmail.com>
 *
 * @author Matthieu Tourne <matthieu@cloudflare.com>
 */

#include "ngx_http_instaweb_module.h"
#include "net/instaweb/htmlparse/public/html_parse.h"
#include "net/instaweb/htmlparse/public/html_filter.h"
#include "net/instaweb/htmlparse/public/empty_html_filter.h"
#include "net/instaweb/rewriter/public/rewrite_driver.h"
#include "net/instaweb/rewriter/public/rewrite_options.h"

#include "net/instaweb/htmlparse/public/html_writer_filter.h"
#include "net/instaweb/util/public/message_handler.h"
#include "ngx_filter_driver.h"
#include "ngx_pagespeed_writer.h"
#include "ngx_message_handler.h"
#include "ngx_http_instaweb_filter.h"


namespace net_instaweb {

typedef struct {
    ngx_flag_t  enable;
} ngx_http_instaweb_loc_conf_t;

static ngx_int_t ngx_http_instaweb_init(ngx_conf_t *cf);

static ngx_http_output_header_filter_pt  ngx_http_next_header_filter;
static ngx_http_output_body_filter_pt    ngx_http_next_body_filter;

static ngx_int_t ngx_http_instaweb_header_filter(ngx_http_request_t *r);
static ngx_int_t ngx_http_instaweb_body_filter(ngx_http_request_t *r,
                                               ngx_chain_t *in);

static void *ngx_http_instaweb_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_instaweb_merge_loc_conf(ngx_conf_t *cf,
                                              void *parent, void *child);

static ngx_command_t ngx_http_instaweb_module_commands[] = {
    { ngx_string("instaweb"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF
      |NGX_HTTP_LIF_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_instaweb_loc_conf_t, enable),
      NULL },

    ngx_null_command
};


static ngx_http_module_t ngx_http_instaweb_module_ctx = {
    NULL,                               /* preconfiguration */
    ngx_http_instaweb_init,		/* postconfiguration */

    NULL,                               /* create main configuration */
    NULL,                               /* init main configuration */

    NULL,                               /* create server configuration */
    NULL,                               /* merge server configuration */

    ngx_http_instaweb_create_loc_conf,  /* create location configuration */
    ngx_http_instaweb_merge_loc_conf    /* merge location configuration */
};

extern "C" {
ngx_module_t  ngx_http_instaweb_module = {
    NGX_MODULE_V1,
    &ngx_http_instaweb_module_ctx,	/* module context */
    ngx_http_instaweb_module_commands,  /* module directives */
    NGX_HTTP_MODULE,                    /* module type */
    NULL,                               /* init master */
    NULL,                               /* init module */
    NULL,                               /* init process */
    NULL,                               /* init thread */
    NULL,                               /* exit thread */
    NULL,                               /* exit process */
    NULL,                               /* exit master */
    NGX_MODULE_V1_PADDING
};
}

static ngx_int_t
ngx_http_instaweb_header_filter(ngx_http_request_t *r) {
    ngx_http_instaweb_loc_conf_t        *lcf;
    ngx_http_instaweb_ctx_t             *ctx;
    FilterDriver                        *driver;
    // TODO (mtourne) : review this
    u_char                              full_uri[512];
    ngx_uint_t                          len;

    lcf = (ngx_http_instaweb_loc_conf_t *)
        ngx_http_get_module_loc_conf(r, ngx_http_instaweb_module);

    if (!lcf->enable) {
        return ngx_http_next_header_filter(r);
    }

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "instaweb header filter");

#if (NGX_HTTP_SSL)
    len = ngx_snprintf(full_uri, 512, "%s://%V/%V", (r->connection->ssl) ? "https" : "http",
                       &r->headers_in.server, &r->unparsed_uri) - full_uri;
#else
    len = ngx_snprintf(full_uri, 512, "http://%V/%V",
                       &r->headers_in.server, &r->unparsed_uri) - full_uri;
#endif


    ctx = (ngx_http_instaweb_ctx_t*) ngx_pcalloc(r->pool, sizeof(ngx_http_instaweb_ctx_t));
    if (ctx == NULL) {
        return NGX_ERROR;
    }

    ctx->writer = new NgxPagespeedWriter(r, ngx_http_next_body_filter, ctx);
    driver = new FilterDriver(new NgxMessageHandler(r->connection->log), ctx->writer);
    if (!driver->StartParse(StringPiece((const char*) full_uri, len))) {
        return ngx_http_next_header_filter(r);
    }
    ctx->driver = driver;

    ngx_http_set_ctx(r, ctx, ngx_http_instaweb_module);

    // clear content length & modified data.
    // rbejjani - why?
    if (r == r->main) {
        ngx_http_clear_content_length(r);
        ngx_http_clear_last_modified(r);
    }

    return ngx_http_next_header_filter(r);
}

#define HERE printf("%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__)

static ngx_int_t
ngx_http_instaweb_body_filter(ngx_http_request_t *r, ngx_chain_t *in) {
    ngx_http_instaweb_ctx_t     *ctx;

    // extract context
    ctx = (ngx_http_instaweb_ctx_t*) ngx_http_get_module_ctx(
        r, ngx_http_instaweb_module);

    // exit early if we can't find context
    if (ctx == NULL) {
        return ngx_http_next_body_filter(r, in);
    }

    // parse each new input
    for(ngx_chain_t *current = in; current != NULL; current = in->next) {
        ctx->driver->ParseText((const char*) current->buf->start, current->buf->end - current->buf->start);

        // if last buffer, end parsing, inducing flush, which writes out chain to filters after us
        if(current->buf->last_buf) {
            // FinishParse will finalise any filters, output chain should be ready after it returns.
            ctx->driver->FinishParse();

            // set the last buffer to be the ngx last buffer
            // TODO (rbejjani) make this more elegant, we can be flushed any time
            ctx->out_tail->buf->last_buf = 1;
        }
    }
    // actually flush to next filter if we have outstanding output buffers
    if(ctx->out_tail->buf->last_buf) {
        // stash a pointer to use in output, clear out our local handles.
        ngx_chain_t   *out_head = ctx->out_head;
        ctx->out_head = ctx->out_tail = NULL;

        return ngx_http_next_body_filter(r, out_head);
    }


    return NGX_OK;
}

static void*
ngx_http_instaweb_create_loc_conf(ngx_conf_t *cf) {
    ngx_http_instaweb_loc_conf_t        *conf;

    conf = (ngx_http_instaweb_loc_conf_t *)
        ngx_pcalloc(cf->pool, sizeof(ngx_http_instaweb_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    /*
     * set by ngx_pcalloc():
     *
     */

    conf->enable = NGX_CONF_UNSET;

    return conf;
}

static char*
ngx_http_instaweb_merge_loc_conf(ngx_conf_t *cf,
                                 void *parent, void *child) {
    ngx_http_instaweb_loc_conf_t        *prev =
        (ngx_http_instaweb_loc_conf_t *) parent;

    ngx_http_instaweb_loc_conf_t        *conf =
        (ngx_http_instaweb_loc_conf_t *) child;

    ngx_conf_merge_value(conf->enable, prev->enable, 0);

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_instaweb_init(ngx_conf_t *cf) {

    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_instaweb_header_filter;

    ngx_http_next_body_filter = ngx_http_top_body_filter;
    ngx_http_top_body_filter = ngx_http_instaweb_body_filter;

    return NGX_OK;
}

} // !namespace net_instaweb
