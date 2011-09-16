/**
 * Copyright (c) 2010, 2011 CloudFlare, Inc. (http://www.cloudflare.com)
 * Copyright (c) 2010, 2011 Ray Bejjani <ray.bejjani@gmail.com>
 *
 * @author Ray Bejjani<ray@cloudflare.com>
 */

#include <string>
#include "ngx_http_instaweb_filter.h"

namespace net_instaweb {

const char* HTMLEventFilter::Name() const { 
    return "HTMLEventFilter";
}

void HTMLEventFilter::StartElement(HtmlElement* element) {
    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, log, 0, "<%s>", element->name_str());
} 

void HTMLEventFilter::EndElement(HtmlElement* element) {
    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, log, 0, "<%s>:", element->name_str());
}

void HTMLEventFilter::Characters(HtmlCharactersNode* characters) {
    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, log, 0, "%s", characters->contents().c_str());
}

} // !namespace net_instaweb


