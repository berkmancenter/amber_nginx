#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>
#include <sqlite3.h>
#include "cayl_utils.h"

typedef struct {
    ngx_hash_t                          types;
    ngx_array_t                        *types_keys;
    ngx_http_complex_value_t           *variable; //TODO: Remove this
    ngx_str_t                           db;
    ngx_str_t                           behavior_up;
    ngx_str_t                           behavior_down;
    ngx_uint_t                          hover_delay_up;
    ngx_uint_t                          hover_delay_down;
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
static cayl_options_t *ngx_http_cayl_build_options(ngx_http_request_t *r, ngx_http_cayl_loc_conf_t *config);
static void ngx_http_cayl_insert_attributes(ngx_http_request_t *r, ngx_buf_t *buf, ngx_http_cayl_matches_t *matches);
static ngx_str_t *ngx_http_cayl_get_attribute(ngx_http_request_t *r, ngx_str_t url);

static ngx_command_t  ngx_http_cayl_filter_commands[] = {

    { ngx_string("cayl"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_http_cayl_filter,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("cayl_db"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_cayl_loc_conf_t, db),
      NULL },

    { ngx_string("cayl_types"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_1MORE,
      ngx_http_types_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_cayl_loc_conf_t, types_keys),
      &ngx_http_html_default_types[0] },

    { ngx_string("cayl_behavior_up"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_cayl_loc_conf_t, behavior_up),
      NULL },

    { ngx_string("cayl_behavior_down"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_cayl_loc_conf_t, behavior_down),
      NULL },

    { ngx_string("cayl_hover_delay_up"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_cayl_loc_conf_t, hover_delay_up),
      NULL },

    { ngx_string("cayl_hover_delay_down"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_cayl_loc_conf_t, hover_delay_down),
      NULL },

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

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
         "CAYL header filter");

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

    ngx_http_set_ctx(r, ctx, ngx_http_cayl_filter_module);

    return ngx_http_next_header_filter(r);
}

/* Go through the buffer chain, and annotate external URLs based on data in
   in the cache */
static ngx_int_t
ngx_http_cayl_body_filter(ngx_http_request_t *r, ngx_chain_t *in)
{
    ngx_log_t             *log = r->connection->log;
    ngx_buf_t             *buf;
    ngx_uint_t             last;
    ngx_chain_t           *cl, *nl;
    ngx_http_cayl_ctx_t *ctx;

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, log, 0,
                   "http CAYL body filter");

    ctx = ngx_http_get_module_ctx(r, ngx_http_cayl_filter_module);
    if (ctx == NULL) {
        /* The context was not setup by the header filter, which means that
           we're not meant to operate on this request */
        return ngx_http_next_body_filter(r, in);
    }

    last = 0;
    ngx_http_cayl_matches_t *matches;

    for (cl = in; cl; cl = cl->next) {
        //  ngx_log_debug5(NGX_LOG_DEBUG_HTTP, log, 0,
        //        "CAYL buffer processed: %p %p %p %p %d", cl->buf->temporary, cl->buf->memory, cl->buf->in_file, cl->buf->end, ngx_buf_size(cl->buf));
        matches = ngx_http_cayl_find_links(r, cl->buf);
        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, log, 0,
                 "CAYL buffer match count: %d", (matches) ? matches->count : 0);
        if (matches && matches->count) {
            for (int i = 0; i < matches->count; i++) {
                ngx_log_debug4(NGX_LOG_DEBUG_HTTP, log, 0,
                    "CAYL buffer match [%d]: %d, %d %V", i, matches->insert_pos[i], matches->url[i].len, &matches->url[i]);
            }

            /* There are some things to change, so create a new buffer within
             * which we'll make the changes. Then we'll remove the current
             * buffer and replace it with the new one */
            buf = ngx_calloc_buf(r->pool);
            if (buf == NULL) {
                ngx_log_error(NGX_LOG_ERR, log, 0,
                              "[CAYL] ngx_http_cayl_body_filter error.");
                return NGX_ERROR;
            }

            /* The new buffer needs to be bigger, since we're adding HTML
             * attributes */
            // int CAYL_ATTRIBUTES_MAX_LENGTH = 200 * sizeof(u_char);
            // int buf_size = (CAYL_ATTRIBUTES_MAX_LENGTH  * matches->count)
            //                 + cl->buf->last - cl->buf->pos;
            // buf->pos = ngx_palloc(r->pool,buf_size);
            // buf->start = buf->pos;
            // buf->end = buf->pos + buf_size;
            // buf->last = buf->pos;

            /* Copy the old buffer to the new one, inserting attributes as we go */
            ngx_http_cayl_insert_attributes(r,cl->buf,matches);
            // buf->memory = 1;
            // buf->last_buf = cl->buf->last_buf;
            // cl->buf->last_buf = 0;

            /* Zero out the old buffer */
            ngx_log_debug4(NGX_LOG_DEBUG_HTTP, log, 0,
                "CAYL old buffer pos/last start/end : %i/%i %i/%i",
                cl->buf->pos, cl->buf->last, cl->buf->start, cl->buf->end);
            // cl->buf->last = cl->buf->pos + 1;
            // cl->buf->start = cl->buf->last;

            /* Create a new link to add to the buffer chain */
            // nl = ngx_alloc_chain_link(r->pool);
            // if (nl == NULL) {
            //     ngx_log_error(NGX_LOG_ERR, log, 0,
            //                   "[cayl] ngx_http_cayl_body_filter error.");
            //     return NGX_ERROR;
            // }
            // nl->buf = buf;
            // nl->next = cl->next;
            // cl->next = nl;
            //
            //
            //
            // cl = nl; /* Otherwise our loop will process the buffer just added */
        }
    }

    return ngx_http_next_body_filter(r, in);
}

/*
 * Update the buffer to insert CAYL HTML attributes for each of the URLs
 * identified in matches struct.
 * Returns 0 on sucess.
 * TODO: Handle the case where the existing size of the buffer is not sufficient
 *       to accomodate the extra data we are adding
 */
static void
ngx_http_cayl_insert_attributes(ngx_http_request_t *r,
                                ngx_buf_t *buf,
                                ngx_http_cayl_matches_t *matches) {

    u_char *dest_pos = buf->pos;
    u_char *cur_pos, *src_pos, *last_pos, *end_pos;
    int copy_size;
    ngx_str_t *insertion;

    cur_pos = ngx_palloc(r->pool,buf->last - buf->start);
    memcpy(cur_pos,buf->pos,buf->last - buf->start);
    src_pos = cur_pos;
    last_pos = cur_pos + (buf->last - buf->pos);
    /* Find out where the end of the buffer is. If we go past this, BANG! */
    end_pos = cur_pos + (buf->end - buf->pos);

    for (int i = 0; i < matches->count; i++) {
        /* Copy the data up to the insertion point for the next match */
        copy_size = matches->insert_pos[i] + src_pos - cur_pos;
        insertion = ngx_http_cayl_get_attribute(r, matches->url[i]);
        /* Make sure that we're not going to overrun the end of the buffer
         * If so, log a message, and return.
         * TODO: Handle this properly */

        if (insertion && ((cur_pos + copy_size + insertion->len) > end_pos)) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
              "[CAYL] ngx_http_cayl_body_filter - buffer not big enough for attributes");
            return;
        }
        memcpy(dest_pos, cur_pos, copy_size);
        dest_pos += copy_size;
        cur_pos += copy_size;

        /* Get the attributes to be inserted, and insert them */
        if (insertion) {
            memcpy(dest_pos, insertion->data, insertion->len);
            dest_pos += insertion->len;
        }
        buf->last = dest_pos + (last_pos - cur_pos);
    }

    /* Add the text after the last match */
    if (end_pos > cur_pos) {
        memcpy(dest_pos, cur_pos, last_pos - cur_pos);
    }
    buf->last = dest_pos + (last_pos - cur_pos);
}


/* Return the CAYL attributes that should be added to the HREF with the given
   target URL, based on data from the cache
*/
static ngx_str_t *
ngx_http_cayl_get_attribute(ngx_http_request_t *r, ngx_str_t url) {

    ngx_str_t *s;
    ngx_int_t sqlite_rc;
    sqlite3 *sqlite_handle;

    const char *location_tmp;
    char *location;
    int date;
    int status;

    ngx_http_cayl_loc_conf_t  *cayl_config;
    cayl_config = ngx_http_get_module_loc_conf(r, ngx_http_cayl_filter_module);
    if (!cayl_config) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
          "[CAYL] ngx_http_cayl_get_attribute - No configuration");
        return NULL;
    }

    char *db = ngx_palloc(r->pool,(cayl_config->db.len + 1) * sizeof(char));
    strncpy(db, (char *)cayl_config->db.data, cayl_config->db.len);
    db[cayl_config->db.len] = 0;

    sqlite_rc = sqlite3_open(db, &sqlite_handle);
    if (sqlite_rc) {
        int extended_rc = sqlite3_extended_errcode(sqlite_handle);
        char * msg = sqlite3_errmsg(sqlite_handle);
        ngx_log_debug2(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
               "CAYL sqlite error details (%d,%s)", extended_rc, msg);
        sqlite3_close(sqlite_handle);
        ngx_log_debug2(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                       "CAYL error opening sqlite database (%d,%s)", sqlite_rc, db);
        return NULL;
    }

    sqlite3_stmt *sqlite_statement;
    const char *query_tail;
    char *query_template = "SELECT ca.location, ca.date, ch.status FROM cayl_cache ca, cayl_check ch WHERE ca.url = ? AND ca.id = ch.id";
    sqlite_rc = sqlite3_prepare_v2(sqlite_handle, query_template, -1, &sqlite_statement, &query_tail);
    if (sqlite_rc != SQLITE_OK) {
        sqlite3_close(sqlite_handle);
        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
               "CAYL error creating sqlite prepared statement (%d)", sqlite_rc);
        return NULL;
    }

    sqlite_rc = sqlite3_bind_text(sqlite_statement, 1, (char *)url.data, url.len, SQLITE_STATIC);
    if (sqlite_rc != SQLITE_OK) {
        sqlite3_close(sqlite_handle);
        ngx_log_debug2(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
               "CAYL error binding sqlite parameter: %V (%d)", url, sqlite_rc);
        return NULL;
    }

    sqlite_rc = sqlite3_step(sqlite_statement);
    if (sqlite_rc == SQLITE_DONE) { /* No data returned */
        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                "CAYL sqlite no results for url: (%V)", &url);
        /* TODO: Save this URL to a table to be looked up later */
        sqlite3_finalize(sqlite_statement);
        sqlite3_close(sqlite_handle);
        return NULL;
    } else if (sqlite_rc == SQLITE_ROW) {
        location_tmp = (const char *) sqlite3_column_text(sqlite_statement,0);
        /* Copy the location string, since it gets clobbered when the
           sqlite objects are closed */
        location = ngx_palloc(r->pool,(strlen(location_tmp) + 1) * sizeof(char));
        strncpy(location,location_tmp,strlen(location_tmp));
        location[strlen(location_tmp)] = 0;

        date = sqlite3_column_int(sqlite_statement,1);
        status = sqlite3_column_int(sqlite_statement,2);
        ngx_log_debug4(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
            "CAYL sqlite results for url: (%V) %s,%d,%d", &url, location, date, status);
    } else {
        sqlite3_finalize(sqlite_statement);
        sqlite3_close(sqlite_handle);
        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
               "CAYL error executing sqlite statement: (%d)", sqlite_rc);
        return NULL;
    }

    sqlite_rc = sqlite3_finalize(sqlite_statement);
    if (sqlite_rc != SQLITE_OK) {
        sqlite3_close(sqlite_handle);
        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
               "CAYL error finalizing statement (%d)", sqlite_rc);
        return NULL;
    }

    sqlite_rc = sqlite3_close(sqlite_handle);
    if (sqlite_rc != SQLITE_OK) {
        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
               "CAYL error closing sqlite database (%d)", sqlite_rc);
    }

    /* Check to see if the location is empty, which means that we don't have
       a cached version available. */
    if (strlen(location) == 0) {
        return NULL;
    }

    cayl_options_t *options = ngx_http_cayl_build_options(r,cayl_config);

    s = ngx_palloc(r->pool,sizeof(ngx_str_t));
    s->data = ngx_palloc(r->pool,CAYL_MAX_ATTRIBUTE_STRING);
    int rc = cayl_build_attribute(options, s->data, location, status, date);
    if (rc) {
        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
            "CAYL error gnerating attribute string (%d)", rc);
        return NULL;
    }
    s->len = strlen((const char*) s->data);
    return s;
}

static int
ngx_http_cayl_convert_behavior_config(ngx_str_t s) {
    if (!strncmp("cache",(const char*) s.data,s.len)) {
        return CAYL_ACTION_CACHE;
    } else if (!strncmp("popup",(const char*)s.data,s.len)) {
        return CAYL_ACTION_POPUP;
    } else if (!strncmp("hover",(const char*)s.data,s.len)) {
        return CAYL_ACTION_HOVER;
    } else {
        return CAYL_ACTION_NONE;
    }
}

/* Copy the options required to determine the appropriate behavior for each link
   to a struct that is not nginx specific */
static cayl_options_t *
ngx_http_cayl_build_options(ngx_http_request_t *r, ngx_http_cayl_loc_conf_t *config) {

    cayl_options_t *options;

    options = ngx_palloc(r->pool,sizeof(cayl_options_t));
    if (!options) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
          "[CAYL] ngx_http_cayl_build_options - Cannot allocate memory");
        return NULL;
    }

    options->behavior_up = ngx_http_cayl_convert_behavior_config(config->behavior_up);
    options->behavior_down = ngx_http_cayl_convert_behavior_config(config->behavior_down);
    options->hover_delay_down = config->hover_delay_down;
    options->hover_delay_up = config->hover_delay_up;

    return options;
}

/* Find all HREFs in a buffer and return information about the URLs found, and the
   location in the buffer where additional attributes can be added.

   TODO: This does not find links that span the boundary between two buffers
*/
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

/* Print the contents of a buffer from teh buffer chaing to the debug log.
   Used only for debugging */
static void
ngx_http_cayl_log_buffer(ngx_http_request_t *r, ngx_buf_t *buf) {

    u_char *c;
    u_char* cur;
    char *s;

    cur = buf->pos;
    while ((c = memchr(cur,'\n', buf->last - cur))) {
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

/***********************************************************************/
/* Everything below here is currently standard nginx module setup code */
/***********************************************************************/

static char *
ngx_http_cayl_filter(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
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

    conf->db.data = NULL;
    conf->behavior_up.data = NULL;
    conf->behavior_down.data = NULL;
    conf->hover_delay_up = NGX_CONF_UNSET_UINT;
    conf->hover_delay_down = NGX_CONF_UNSET_UINT;

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

    ngx_conf_merge_str_value(conf->db,                   prev->db,"cayl.db");
    ngx_conf_merge_str_value(conf->behavior_up,          prev->behavior_up,"none");
    ngx_conf_merge_str_value(conf->behavior_down,        prev->behavior_down,"popup");
    ngx_conf_merge_uint_value(conf->hover_delay_up,      prev->hover_delay_up,5);
    ngx_conf_merge_uint_value(conf->hover_delay_down,    prev->hover_delay_down,2);

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
