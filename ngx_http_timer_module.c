
/*
 * Copyright (C) Jonathan Kolb
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>

typedef struct {
    ngx_event_t event;
    ngx_flag_t  enabled;
    ngx_msec_t  period;
} ngx_http_timer_conf_t;


static void *ngx_http_timer_create_main_conf(ngx_conf_t *cf);
static char *ngx_http_timer_init_main_conf(ngx_conf_t *cf, void *conf);
static ngx_int_t ngx_http_timer_init_worker(ngx_cycle_t *cycle);
static void ngx_http_timer_exit_worker(ngx_cycle_t *cycle);
static void ngx_http_timer_tick(ngx_event_t *ev);


static ngx_command_t  ngx_http_timer_commands[] = {

    { ngx_string("timer_enabled"),
      NGX_HTTP_MAIN_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_HTTP_MAIN_CONF_OFFSET,
      offsetof(ngx_http_timer_conf_t, enabled),
      NULL },

    { ngx_string("timer_period"),
      NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_HTTP_MAIN_CONF_OFFSET,
      offsetof(ngx_http_timer_conf_t, period),
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_timer_module_ctx = {
    NULL,                                  /* preconfiguration */
    NULL,                                  /* postconfiguration */

    ngx_http_timer_create_main_conf,       /* create main configuration */
    ngx_http_timer_init_main_conf,         /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL                                   /* merge location configuration */
};


ngx_module_t  ngx_http_timer_module = {
    NGX_MODULE_V1,
    &ngx_http_timer_module_ctx,            /* module context */
    ngx_http_timer_commands,               /* module directives */
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
ngx_http_timer_create_main_conf(ngx_conf_t *cf)
{
    ngx_http_timer_conf_t *tcf;

    tcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_timer_conf_t));
    if (NULL == tcf) {
        return NULL;
    }

    tcf->event.handler = ngx_http_timer_tick;
    tcf->event.data = tcf;
    tcf->event.log = cf->log;
    tcf->enabled = NGX_CONF_UNSET;
    tcf->period = NGX_CONF_UNSET_MSEC;

    return tcf;
}


static char *
ngx_http_timer_init_main_conf(ngx_conf_t *cf, void *conf)
{
    ngx_http_timer_conf_t *tcf = conf;

    ngx_conf_init_value(tcf->enabled, 1);
    ngx_conf_init_msec_value(tcf->period, 5000);

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_timer_init_worker(ngx_cycle_t *cycle)
{
    ngx_http_timer_conf_t *tcf;

    if (NGX_PROCESS_WORKER != ngx_process) {
        return NGX_OK;
    }

    tcf = ngx_http_cycle_get_module_main_conf(cycle, ngx_http_timer_module);

    if (tcf->enabled) {
        ngx_add_timer(&tcf->event, tcf->period);
    }

    return NGX_OK;
}


static void
ngx_http_timer_exit_worker(ngx_cycle_t *cycle)
{
    ngx_http_timer_conf_t *tcf;

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
    ngx_http_timer_conf_t *tcf;

    if (ngx_terminate || ngx_exiting) {
        return;
    }

    tcf = ev->data;

    ngx_log_error(NGX_LOG_INFO, ngx_cycle->log, 0, "tick");

    ngx_add_timer(ev, tcf->period);
}

