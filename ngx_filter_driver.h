/**
 * Copyright (c) 2010, 2011 CloudFlare, Inc. (http://www.cloudflare.com)
 * Copyright (c) 2010, 2011 Ray Bejjani <ray.bejjani@gmail.com>
 *
 * @author Ray Bejjani<ray@cloudflare.com>
 */

#ifndef NGX_FILTER_DRIVER_H_
#define NGX_FILTER_DRIVER_H_

#include "ngx_http_instaweb_module.h"

#include "net/instaweb/htmlparse/public/html_parse.h"

namespace net_instaweb {

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
