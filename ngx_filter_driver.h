/**
 * Copyright (c) 2010, 2011 CloudFlare, Inc. (http://www.cloudflare.com)
 * Copyright (c) 2010, 2011 Ray Bejjani <ray.bejjani@gmail.com>
 *
 * @author Ray Bejjani<ray@cloudflare.com>
 */

#ifndef NGX_FILTER_DRIVER_H_
#define NGX_FILTER_DRIVER_H_

#include <map>
#include <set>
#include <vector>
#include "base/scoped_ptr.h"
#include "net/instaweb/htmlparse/public/html_element.h"
#include "net/instaweb/htmlparse/public/html_parse.h"
#include "net/instaweb/http/public/http_cache.h"
#include "net/instaweb/http/public/url_async_fetcher.h"
#include "net/instaweb/http/public/user_agent_matcher.h"
#include "net/instaweb/rewriter/public/output_resource_kind.h"
#include "net/instaweb/rewriter/public/resource.h"
#include "net/instaweb/rewriter/public/resource_manager.h"
#include "net/instaweb/rewriter/public/resource_slot.h"
#include "net/instaweb/rewriter/public/rewrite_options.h"
#include "net/instaweb/rewriter/public/scan_filter.h"
#include "net/instaweb/util/public/basictypes.h"
#include "net/instaweb/util/public/google_url.h"
#include "net/instaweb/util/public/scheduler.h"
#include "net/instaweb/util/public/string.h"
#include "net/instaweb/util/public/string_util.h"
#include "net/instaweb/util/public/url_segment_encoder.h"

extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
  #include <ngx_http.h>
}

#include "net/instaweb/htmlparse/public/html_writer_filter.h"
#include "net/instaweb/util/public/message_handler.h"
#include "ngx_filter_driver.h"
#include "ngx_pagespeed_writer.h"
#include "ngx_message_handler.h"

namespace net_instaweb {

struct ContentType;

class AbstractMutex;
class AddInstrumentationFilter;
class CommonFilter;
class DomainRewriteFilter;
class FileSystem;
class HtmlFilter;
class HtmlWriterFilter;
class MessageHandler;
class RequestHeaders;
class ResourceContext;
class ResponseHeaders;
class RewriteContext;
class RewriteFilter;
class Statistics;
class Writer;

// Also note that ResourceManager should be renamed ServerContext, as
// it no longer contains much logic about resources.
class FilterDriver : public HtmlParse {
 public:
  FilterDriver(NgxMessageHandler *msg_handler, NgxPagespeedWriter *writer);
  // Need explicit destructors to allow destruction of scoped_ptr-controlled
  // instances without propagating the include files.
  virtual ~FilterDriver();

  void SetupFilters();

  ngx_int_t GetWriterStatus();

 private:
  friend class ResourceManagerTestBase;
  friend class ResourceManagerTest;

  void AddFilter(HtmlFilter* filter);
  NgxMessageHandler	*msg_handler;
  NgxPagespeedWriter *writer;

  DISALLOW_COPY_AND_ASSIGN(FilterDriver);
};

}  // namespace net_instaweb

#endif  // NGX_FILTER_DRIVER_H_
