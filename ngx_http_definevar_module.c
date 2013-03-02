
/*
 * Copyright (C) Jonathan Kolb
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


static char *ngx_http_definevar_define(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
static ngx_int_t ngx_http_definevar_value(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);


static ngx_command_t  ngx_http_definevar_commands[] = {

    { ngx_string("define"),
      NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE2,
      ngx_http_definevar_define,
      NGX_HTTP_MAIN_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_definevar_module_ctx = {
    NULL,       /* preconfiguration */
    NULL,       /* postconfiguration */

    NULL,       /* create main configuration */
    NULL,       /* init main configuration */

    NULL,       /* create server configuration */
    NULL,       /* merge server configuration */

    NULL,       /* create location configuration */
    NULL        /* merge location configuration */
};


ngx_module_t  ngx_http_definevar_module = {
    NGX_MODULE_V1,
    &ngx_http_definevar_module_ctx,        /* module context */
    ngx_http_definevar_commands,           /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static char *
ngx_http_definevar_define(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t                          *value;
    ngx_http_complex_value_t           *cv;
    ngx_http_compile_complex_value_t    ccv;
    ngx_http_variable_t                *v;

    value = cf->args->elts;

    if (value[1].data[0] != '$') {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "invalid variable name \"%V\"", &value[1]);
        return NGX_CONF_ERROR;
    }

    value[1].len--;
    value[1].data++;

    v = ngx_http_add_variable(cf, &value[1], NGX_HTTP_VAR_NOCACHEABLE);
    if (v == NULL) {
        return NGX_CONF_ERROR;
    }

    cv = ngx_palloc(cf->pool, sizeof(ngx_http_complex_value_t));
    if (cv == NULL) {
        return NGX_CONF_ERROR;
    }

    ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));

    ccv.cf = cf;
    ccv.value = &value[2];
    ccv.complex_value = cv;

    if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
        return NGX_CONF_ERROR;
    }

    v->get_handler = ngx_http_definevar_value;
    v->data = (uintptr_t) cv;

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_definevar_value(ngx_http_request_t *r, ngx_http_variable_value_t *v,
    uintptr_t data)
{
    ngx_http_complex_value_t *cv = (ngx_http_complex_value_t *) data;
    ngx_str_t                 value;

    if (NGX_OK != ngx_http_complex_value(r, cv, &value)) {
        return NGX_ERROR;
    }

    v->len = value.len;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = value.data;

    return NGX_OK;
}

