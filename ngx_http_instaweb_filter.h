/**
 * Copyright (c) 2010, 2011 CloudFlare, Inc. (http://www.cloudflare.com)
 * Copyright (c) 2010, 2011 Ray Bejjani <ray.bejjani@gmail.com>
 *
 * @author Ray Bejjani<ray@cloudflare.com>
 */

#include "ngx_http_instaweb_module.h"
#include "net/instaweb/htmlparse/public/html_parse.h"
#include "net/instaweb/htmlparse/public/empty_html_filter.h"
#include "net/instaweb/rewriter/public/rewrite_driver.h"
#include "net/instaweb/rewriter/public/rewrite_options.h"

namespace net_instaweb {

    class HTMLEventFilter : public EmptyHtmlFilter {
        public:
            HTMLEventFilter(ngx_log_t* const log) { 
                this->log = log;
            }
            virtual const char* Name() const;

            // Overrides
            virtual void StartElement(HtmlElement* element);
            virtual void EndElement(HtmlElement* element);
            virtual void Characters(HtmlCharactersNode* characters);
            // ngx functions

        private:
            DISALLOW_COPY_AND_ASSIGN(HTMLEventFilter);
            ngx_log_t          *log;
            std::vector<HtmlFilter*> filters_;
    };

} // !namespace net_instaweb
