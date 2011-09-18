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

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "instaweb header filter");

#if (NGX_HTTP_SSL)
    len = ngx_snprintf(full_uri, 512, "%s://%V/%V", (r->connection->ssl) ?
                       "https" : "http",
                       &r->headers_in.server, &r->unparsed_uri) - full_uri;
#else
    len = ngx_snprintf(full_uri, 512, "http://%V/%V",
                       &r->headers_in.server, &r->unparsed_uri) - full_uri;
#endif

    ctx = (ngx_http_instaweb_ctx_t*) ngx_pcalloc(
        r->pool, sizeof(ngx_http_instaweb_ctx_t));
    if (ctx == NULL) {
	return NGX_ERROR;
    }
    ctx->last_out = &ctx->out;

    ctx->writer = new NgxPagespeedWriter(r, ngx_http_next_body_filter, ctx);
    driver = new FilterDriver(new NgxMessageHandler(r->connection->log), ctx->writer);
    if (!driver->StartParse(StringPiece((const char*) full_uri, len))) {
        return ngx_http_next_header_filter(r);
    }
    ctx->driver = driver;

    ngx_http_set_ctx(r, ctx, ngx_http_instaweb_module);

    /* loads the content of the response in : in->buf */
    r->filter_need_in_memory = 1;

    /* clear content length */
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
    ngx_chain_t                 *cl;
    ngx_int_t                   rc;

    ctx = (ngx_http_instaweb_ctx_t*) ngx_http_get_module_ctx(
        r, ngx_http_instaweb_module);

    if (ctx == NULL) {
        return ngx_http_next_body_filter(r, in);
    }

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http instaweb filter \"%V\"", &r->uri);

    while (in) {
        ctx->driver->ParseText((const char*) in->buf->pos,
                               in->buf->last - in->buf->pos);


        if (in->buf->last_buf) {
            ctx->driver->FinishParse();

            cl = ngx_chain_get_free_buf(r->pool, &ctx->free);
            if (cl == NULL) {
                return NGX_ERROR;
            }
            cl->buf->last_buf = 1;
            *ctx->last_out = cl;
            ctx->last_out = &cl->next;
        }

        in = in->next;
    }

    if (ctx->out == NULL && ctx->busy == NULL) {
        return NGX_OK;
    }

    rc = ngx_http_next_body_filter(r, ctx->out);

    ngx_chain_update_chains(&ctx->free, &ctx->busy, &ctx->out,
			  (ngx_buf_tag_t) &ngx_http_instaweb_module);

    ctx->last_out = &ctx->out;
    ctx->out_buf = NULL;

    return rc;
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
