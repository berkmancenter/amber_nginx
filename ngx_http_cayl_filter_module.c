#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
    ngx_hash_t                          types;
    ngx_array_t                        *types_keys;
    ngx_http_complex_value_t           *variable;
} ngx_http_cayl_loc_conf_t;


typedef struct {
    ngx_str_t                           cayl;
} ngx_http_cayl_ctx_t;

typedef struct {
    int        count;            /* Number of matching insertion positions and
                                    urls.  */
    int        *insert_pos;      /* Array - positions within the buffer where
                                    additional attributes should in inserted
                                    for matching hrefs */
    ngx_str_t  *url;             /* Array - urls within the buffer. */
} ngx_http_cayl_matches_t;

static char *ngx_http_cayl_filter(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
static void *ngx_http_cayl_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_cayl_merge_loc_conf(ngx_conf_t *cf,
    void *parent, void *child);
static ngx_int_t ngx_http_cayl_filter_init(ngx_conf_t *cf);

static void ngx_http_cayl_log_buffer(ngx_http_request_t *r, ngx_buf_t *buf);
static ngx_http_cayl_matches_t *ngx_http_cayl_find_links(ngx_http_request_t *r, ngx_buf_t *buf);
static ngx_int_t ngx_http_cayl_insert_string(ngx_chain_t *cl, u_int *pos, ngx_http_request_t *r);

static ngx_command_t  ngx_http_cayl_filter_commands[] = {

    { ngx_string("cayl"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_http_cayl_filter,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("cayl_types"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_1MORE,
      ngx_http_types_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_cayl_loc_conf_t, types_keys),
      &ngx_http_html_default_types[0] },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_cayl_filter_module_ctx = {
    NULL,                               /* proconfiguration */
    ngx_http_cayl_filter_init,        /* postconfiguration */

    NULL,                               /* create main configuration */
    NULL,                               /* init main configuration */

    NULL,                               /* create server configuration */
    NULL,                               /* merge server configuration */

    ngx_http_cayl_create_loc_conf,    /* create location configuration */
    ngx_http_cayl_merge_loc_conf      /* merge location configuration */
};


ngx_module_t  ngx_http_cayl_filter_module = {
    NGX_MODULE_V1,
    &ngx_http_cayl_filter_module_ctx, /* module context */
    ngx_http_cayl_filter_commands,    /* module directives */
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


static ngx_http_output_header_filter_pt ngx_http_next_header_filter;
static ngx_http_output_body_filter_pt   ngx_http_next_body_filter;


static ngx_int_t
ngx_http_cayl_header_filter(ngx_http_request_t *r)
{
    ngx_http_cayl_ctx_t       *ctx;
    ngx_http_cayl_loc_conf_t  *lcf;

    lcf = ngx_http_get_module_loc_conf(r, ngx_http_cayl_filter_module);

    if (lcf->variable == (ngx_http_complex_value_t *) -1
        || r->header_only
        || (r->method & NGX_HTTP_HEAD)
        || r != r->main
        || r->headers_out.status == NGX_HTTP_NO_CONTENT
        || ngx_http_test_content_type(r, &lcf->types) == NULL)
    {
        return ngx_http_next_header_filter(r);
    }

    ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_cayl_ctx_t));
    if (ctx == NULL) {
       return NGX_ERROR;
    }

    if (ngx_http_complex_value(r, lcf->variable, &ctx->cayl) != NGX_OK) {
        return NGX_ERROR;
    }

    ngx_http_set_ctx(r, ctx, ngx_http_cayl_filter_module);

    if (r->headers_out.content_length_n != -1) {
        r->headers_out.content_length_n += ctx->cayl.len;
    }

    if (r->headers_out.content_length) {
        r->headers_out.content_length->hash = 0;
        r->headers_out.content_length = NULL;
    }

    ngx_http_clear_accept_ranges(r);

    return ngx_http_next_header_filter(r);
}


static ngx_int_t
ngx_http_cayl_body_filter(ngx_http_request_t *r, ngx_chain_t *in)
{
    ngx_buf_t             *buf;
    ngx_uint_t             last;
    ngx_chain_t           *cl, *nl;
    ngx_http_cayl_ctx_t *ctx;

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http cayl body filter");

    ctx = ngx_http_get_module_ctx(r, ngx_http_cayl_filter_module);
    if (ctx == NULL) {
        return ngx_http_next_body_filter(r, in);
    }

    last = 0;
    ngx_http_cayl_matches_t *matches;

    for (cl = in; cl; cl = cl->next) {
        //  ngx_log_debug5(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
        //        "CAYL buffer processed: %p %p %p %p %d", cl->buf->temporary, cl->buf->memory, cl->buf->in_file, cl->buf->end, ngx_buf_size(cl->buf));
        matches = ngx_http_cayl_find_links(r, cl->buf);
        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                 "CAYL buffer match count: %d", (matches) ? matches->count : 0);
        if (matches && matches->count) {
            for (int i = 0; i < matches->count; i++) {
                ngx_log_debug4(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                    "CAYL buffer match [%d]: %d, %d %V", i, matches->insert_pos[i], matches->url[i].len, &matches->url[i]);
            }
        }

         if (cl->buf->last_buf) {
             last = 1;
             break;
         }
    }

    if (!last) {
        return ngx_http_next_body_filter(r, in);
    }

    buf = ngx_calloc_buf(r->pool);
    if (buf == NULL) {
        return NGX_ERROR;
    }

    buf->pos = ctx->cayl.data;
    buf->last = buf->pos + ctx->cayl.len;
    buf->start = buf->pos;
    buf->end = buf->last;
    buf->last_buf = 1;
    buf->memory = 1;

    if (ngx_buf_size(cl->buf) == 0) {
        cl->buf = buf;
    } else {
        nl = ngx_alloc_chain_link(r->pool);
        if (nl == NULL) {
            return NGX_ERROR;
        }

        nl->buf = buf;
        nl->next = NULL;
        cl->next = nl;

        cl->buf->last_buf = 0;

    }
    ngx_log_debug5(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
      "CAYL buffer pre-changes: %p %p %p %p %d", cl->buf->pos, cl->buf->last, cl->buf->start, cl->buf->end, ngx_buf_size(cl->buf));

    return ngx_http_next_body_filter(r, in);
}

static ngx_http_cayl_matches_t *
ngx_http_cayl_find_links(ngx_http_request_t *r, ngx_buf_t *buf) {
#if (NGX_PCRE)
    ngx_http_cayl_matches_t    *matches;
    u_char                    errstr[NGX_MAX_CONF_ERRSTR];
    ngx_str_t                 err;
    ngx_str_t                 pattern;
    int                       rc, n, capture_count;
    ngx_regex_t               *match_regex;

    pattern.data = (u_char*) "href=[\"'](http[^\v()<>{}\\[\\]\"']+)['\"]";
    pattern.len = sizeof(pattern.data);
    err.len = NGX_MAX_CONF_ERRSTR;
    err.data = errstr;

    // Assuming that we're dealing with nginx version > 0.8.25
    ngx_regex_compile_t  regex_compile;
    regex_compile.pattern = pattern;
    regex_compile.pool = r->pool;
    regex_compile.err = err;
    regex_compile.options = NGX_REGEX_CASELESS;
    if (ngx_regex_compile(&regex_compile) != NGX_OK) {
        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                       "CAYL regex error: %V", &regex_compile.err);
        return NULL;
    }
    match_regex = regex_compile.regex;

    // Assuming that we're dealing with nginx version > 1.2.2 (1002002)
    rc = pcre_fullinfo(match_regex->code, NULL, PCRE_INFO_CAPTURECOUNT, &capture_count);
    /* rc < 0 is an error, 0 is success */
    if (rc < 0) {
        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                       "CAYL pcre_fullinfo error: %d",rc);
        return NULL;
    }

    ngx_str_t  line;
    ngx_str_t  m;
    int           *captures;
    ngx_int_t      ncaptures;


    ncaptures = (capture_count + 1) * 3; /* Size of the vector for PCRE */
    captures = ngx_palloc(r->pool, ncaptures * sizeof(int));
    if (captures == NULL) {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                           "CAYL null captures error");

    }

    char *s;
    u_char *c;
    u_char* cur;

    int MATCHES_CHUNK_SIZE = 2;
    matches = ngx_palloc(r->pool,sizeof(ngx_http_cayl_matches_t));
    if (!matches) {
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "CAYL allocation error : matches");
    }
    matches->count = 0;
    matches->insert_pos = NULL;
    matches->url = NULL;


    cur = buf->pos;
    while ((c = memchr(cur,'\n', buf->last - cur))) {

        line.data = cur;
        line.len = c - cur;

        do {
            rc = ngx_regex_exec(match_regex, &line, captures, ncaptures);
            if (rc < NGX_REGEX_NO_MATCHED) {
                ngx_log_debug2(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                    "CAYL regex error (%d) : %V", rc, &line);
                break;
            } else if (rc == 0) {
                ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "CAYL regex error (offsets not big enough) : %V", &line);
                break;
            } else if (rc > 0) {
                /* captures[0] and captures[1] are for the capture group matching
                 * the full regex. captures[2] and captures[3] are for the first
                 * capture group within it */
                m.data = line.data + captures[2];
                m.len = captures[3] - captures[2];
                // ngx_log_debug2(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                //     "CAYL regex match (%V): %V", &m, &line);

                if (!matches->insert_pos || !matches->url) {
                    matches->insert_pos = ngx_palloc(r->pool,
                                                     sizeof(int) * MATCHES_CHUNK_SIZE);
                    matches->url = ngx_palloc(r->pool,
                                              sizeof(ngx_str_t) * MATCHES_CHUNK_SIZE);
                } else if ((matches->count % MATCHES_CHUNK_SIZE) == 0) {
                    int              *t1 = matches->insert_pos;
                    ngx_str_t        *t2 = matches->url;
                    matches->insert_pos = ngx_palloc(r->pool,
                                     sizeof(int) * (MATCHES_CHUNK_SIZE + matches->count));
                    matches->url = ngx_palloc(r->pool,
                                     sizeof(ngx_str_t) * (MATCHES_CHUNK_SIZE + matches->count));
                    memcpy(matches->insert_pos,t1,sizeof(int) * matches->count);
                    memcpy(matches->url,t2,sizeof(ngx_str_t) * matches->count);
                }
                /* Insertion point is the  beginning of the current regex match,
                 * plus the difference by which the line is offset from the
                 * beginning of the buffer */
                matches->insert_pos[matches->count] = captures[0] + line.data - buf->pos;
                matches->url[matches->count].len = m.len;
                matches->url[matches->count].data = ngx_palloc(r->pool,
                                                    sizeof(u_char) * m.len);
                memcpy(matches->url[matches->count].data, m.data, m.len);
                matches->count++;

                /* Move the pointer for the line to the end of the string that
                 * matched the regex */
                line.data = line.data + captures[1];
                line.len = line.len - captures[1];
            }
        } while ((rc > 0) && (line.len > 0));

        cur = c + 1;
        if (cur >= buf->last) {
            break;
        }
    }
    return matches;

#endif
}

static void
ngx_http_cayl_log_buffer(ngx_http_request_t *r, ngx_buf_t *buf) {

    u_char *c;
    u_char* cur;
    char *s;

    cur = buf->pos;
    while ((c = memchr(cur,'\n', buf->last - cur))) {
        // ngx_log_debug3(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
        //       "CAYL buffer pos: %d %d %d", c, cur, buf->last);

        s = ngx_pcalloc(r->pool, c - cur);
        memcpy(s,cur,c - cur);
        ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                      "CAYL buffer: %s", s);
        cur = c + 1;
        if (cur >= buf->last) {
            break;
        }
    }

}


// static ngx_int_t
// ngx_http_cayl_insert_string(ngx_chain_t *cl, u_int *pos, ngx_http_request_t *r)
// {
//     ngx_chain_t   *added_link, *added_link2;
//     ngx_buf_t             *buf, *buf2;
//
//     buf = ngx_calloc_buf(r->pool);
//     if (buf == NULL) {
//       return NGX_ERROR;
//     }
//     buf->pos = (u_char *) "DONUTS";
//     buf->end = buf->last = buf->pos + sizeof("DONUTS");
//     buf->start = buf->pos;
//     buf->memory = 1;
//
// buf2 = ngx_calloc_buf(r->pool);
// if (buf2 == NULL) {
//   return NGX_ERROR;
// }
// buf2->pos = pos + 1;
// buf2->end = buf2->last = pos+20;
// buf2->start = buf2->pos;
// buf2->memory = 1;
//
//
//     cl->buf->last = pos;
//     ngx_log_debug5(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
//       "CAYL buffer truncated: %p %p %p %p %d", cl->buf->pos, cl->buf->last, cl->buf->start, cl->buf->end, ngx_buf_size(cl->buf));
//
//     added_link = ngx_alloc_chain_link(r->pool);
//     if (added_link == NULL)
//         return NGX_ERROR;
//
//     added_link->buf = buf;
//     added_link->next = cl->next;
//     if (cl->buf->last_buf) {
//       added_link->buf->last_buf = 1;
//     }
//     cl->buf->last_buf = 0;
//     cl->next = added_link;
//
// added_link2 = ngx_alloc_chain_link(r->pool);
// if (added_link2 == NULL)
//     return NGX_ERROR;
//
// added_link2->buf = buf2;
// added_link2->next = added_link->next;
// if (added_link->buf->last_buf) {
//   added_link2->buf->last_buf = 1;
// }
// added_link->buf->last_buf = 0;
// added_link->next = added_link2;
//
//
//
//
// }


static char *
ngx_http_cayl_filter(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_cayl_loc_conf_t *flcf = conf;

    ngx_str_t                    *value;
    ngx_http_complex_value_t    **cv;

    cv = &flcf->variable;

    if (*cv != NULL) {
        return "is duplicate";
    }

    value = cf->args->elts;

    if (value[1].len) {
        cmd->offset = offsetof(ngx_http_cayl_loc_conf_t, variable);
        return ngx_http_set_complex_value_slot(cf, cmd, conf);
    }

    *cv = (ngx_http_complex_value_t *) -1;

    return NGX_OK;
}


static void *
ngx_http_cayl_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_cayl_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_cayl_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    /*
     * set by ngx_pcalloc():
     *
     *     conf->types = { NULL };
     *     conf->types_keys = NULL;
     *     conf->variable = NULL;
     */

    return conf;
}


static char *
ngx_http_cayl_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_cayl_loc_conf_t  *prev = parent;
    ngx_http_cayl_loc_conf_t  *conf = child;

    if (ngx_http_merge_types(cf, &conf->types_keys, &conf->types,
                             &prev->types_keys,&prev->types,
                             ngx_http_html_default_types)
        != NGX_OK)
    {
       return NGX_CONF_ERROR;
    }

    if (conf->variable == NULL) {
        conf->variable = prev->variable;
    }

    if (conf->variable == NULL) {
        conf->variable = (ngx_http_complex_value_t *) -1;
    }

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_cayl_filter_init(ngx_conf_t *cf)
{
    ngx_http_next_body_filter = ngx_http_top_body_filter;
    ngx_http_top_body_filter = ngx_http_cayl_body_filter;

    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_cayl_header_filter;

    return NGX_OK;
}
