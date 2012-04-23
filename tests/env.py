#!/usr/bin/env python
#
# Copyright (C) 2012 CloudFlare
# @author Matthieu Tourne <matthieu@cloudflare.com>

import argparse

import ngxtest.env as env

class PagespeedEnv(env.NginxEnv):
    _config = '''
daemon         off;

worker_processes 1;

error_log /tmp/nginx_error.log {log_level};

worker_rlimit_nofile 16384;
events {{
   use epoll;
   worker_connections 16384;
}}

http {{
    variables_hash_max_size       1024;
    variables_hash_bucket_size    128;
    server_names_hash_bucket_size 256;

    access_log         off;


    sendfile           on;
    tcp_nopush         on;
    tcp_nodelay        on;
    directio           2m;
    lingering_close    always;

    keepalive_timeout  46;

    ignore_invalid_headers on;
    client_header_timeout 15;
    client_body_timeout 15;
    #output_buffers 8 4k;

    ## Visitor limits
    large_client_header_buffers      4 16k;
    client_max_body_size             50M;
    client_body_buffer_size          128k;
    client_header_buffer_size        32k;


    server {{
        listen 32080;
        server_name proxy;

        location / {{
            instaweb on;

            proxy_pass_request_headers    on;
            proxy_buffering               off;
            proxy_pass  http://127.0.0.1:32081/;
        }}


    }}


    server {{
        listen 32081;
        server_name web;

        location / {{
            root {testdir};
        }}


    }}
}}

'''

def main():
    parser = argparse.ArgumentParser(description='nginx env to test pagespeed')
    parser.add_argument('nginx', type=str, nargs=1, help='nginx binary')
    args = parser.parse_args()

    env = PagespeedEnv(createEnv=False)
    env.setVar('log_level', 'debug_http')
    env.createEnv()
    env.run(None, nginx_binary=args.nginx[0], debug=True)

if __name__ == '__main__':
    main()
