/**
 * Copyright (c) 2010, 2011 CloudFlare, Inc. (http://www.cloudflare.com)
 * Copyright (c) 2010, 2011 Ray Bejjani <ray.bejjani@gmail.com>
 *
 * @author Ray Bejjani<ray@cloudflare.com>
 */

#include "ngx_filter_driver.h"
#include "ngx_pagespeed_writer.h"

#include "net/instaweb/htmlparse/public/html_writer_filter.h"

#include "net/instaweb/rewriter/public/remove_comments_filter.h"
#include "net/instaweb/rewriter/public/collapse_whitespace_filter.h"

namespace net_instaweb {

class FileSystem;

FilterDriver::FilterDriver(NgxMessageHandler *msg_handler, NgxPagespeedWriter *writer) :
    HtmlParse(msg_handler),
    msg_handler(msg_handler),
    writer(writer) {
  SetupFilters();
}

FilterDriver::~FilterDriver() {
}

void FilterDriver::SetupFilters() {
    // Add transform filters

    // Remove comments in HTML
    AddFilter(new RemoveCommentsFilter(this));

    // Remove excess whitespace in HTML
    AddFilter(new CollapseWhitespaceFilter(this));


    // add output filters
    HtmlWriterFilter *out = new HtmlWriterFilter(this);
    // add writer filter. This writes it all out!
    out->set_writer(writer);
    AddFilter(out);
}

void FilterDriver::AddFilter(HtmlFilter* filter) {
  HtmlParse::AddFilter(filter);
  // printf("Adding filter: %s\n", filter->Name());
}

}  // namespace net_instaweb
