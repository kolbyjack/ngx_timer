
/*
 * Copyright (C) Jonathan Kolb
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>

typedef struct {
    ngx_event_t event;
} ngx_http_timer_conf_t;


static void *ngx_http_timer_create_conf(ngx_conf_t *cf);
static ngx_int_t ngx_http_timer_init_worker(ngx_cycle_t *cycle);
static void ngx_http_timer_exit_worker(ngx_cycle_t *cycle);
static void ngx_http_timer_tick(ngx_event_t *ev);


static ngx_http_module_t  ngx_http_timer_module_ctx = {
    NULL,                                  /* preconfiguration */
    NULL,                                  /* postconfiguration */

    ngx_http_timer_create_conf,            /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL                                   /* merge location configuration */
};


ngx_module_t  ngx_http_timer_module = {
    NGX_MODULE_V1,
    &ngx_http_timer_module_ctx,            /* module context */
    NULL,                                  /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    ngx_http_timer_init_worker,            /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    ngx_http_timer_exit_worker,            /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static void *
ngx_http_timer_create_conf(ngx_conf_t *cf)
{
    ngx_http_timer_conf_t   *tcf;

    tcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_timer_conf_t));
    if (NULL == tcf) {
        return NULL;
    }

    tcf->event.handler = ngx_http_timer_tick;
    tcf->event.data = tcf;
    tcf->event.log = cf->log;

    return tcf;
}


static ngx_int_t
ngx_http_timer_init_worker(ngx_cycle_t *cycle)
{
    ngx_http_timer_conf_t   *tcf;

    if (NGX_PROCESS_WORKER != ngx_process) {
        return NGX_OK;
    }

    tcf = ngx_http_cycle_get_module_main_conf(cycle, ngx_http_timer_module);

    ngx_add_timer(&tcf->event, 5000);

    return NGX_OK;
}


static void
ngx_http_timer_exit_worker(ngx_cycle_t *cycle)
{
    ngx_http_timer_conf_t   *tcf;

    if (NGX_PROCESS_WORKER != ngx_process) {
        return;
    }

    tcf = ngx_http_cycle_get_module_main_conf(cycle, ngx_http_timer_module);

    if (tcf->event.timer_set) {
        ngx_del_timer(&tcf->event);
    }
}


static void
ngx_http_timer_tick(ngx_event_t *ev)
{
    if (ngx_terminate || ngx_exiting) {
        return;
    }

    ngx_log_error(NGX_LOG_INFO, ngx_cycle->log, 0, "tick");

    ngx_add_timer(ev, 5000);
}

