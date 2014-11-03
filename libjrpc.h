/**
 * This file is part of libjrpc library code.
 *
 * Copyright (C) 2014 Roman Yeryomin <roman@advem.lv>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENCE.txt file for more details.
 */
#ifndef _JRPC_H_
#define _JRPC_H_

#include <libipsc.h>
#include <libfmt.h>

#define JRPC_KEY_JSONRPC		"jsonrpc"
#define JRPC_KEY_VERSION		"2.0"
#define JRPC_KEY_ID			"id"
#define JRPC_KEY_RESULT			"result"
#define JRPC_KEY_ERROR			"error"
#define JRPC_KEY_ERROR_CODE		"code"
#define JRPC_KEY_ERROR_TEXT		"message"
#define JRPC_KEY_METHOD			"method"
#define JRPC_KEY_PARAMS			"params"
#define JRPC_ERR_PARSE_ERROR		"Parse error"
#define JRPC_ERR_INVALID_REQUEST	"Invalid request"
#define JRPC_ERR_METHOD_NOT_FOUND	"Method not found"
#define JRPC_ERR_INVALID_PARAMS		"Invalid params"
#define JRPC_ERR_INTERNAL_ERROR		"Internal error"
#define JRPC_ERR_NOT_IMPLEMENTED	"Not implemented"
#define JRPC_CODE_PARSE_ERROR		-32700
#define JRPC_CODE_INVALID_REQUEST	-32600
#define JRPC_CODE_METHOD_NOT_FOUND	-32601
#define JRPC_CODE_INVALID_PARAMS	-32602
#define JRPC_CODE_INTERNAL_ERROR	-32603
/* -32000 to -32099 are reserved for implementation-defined server errors */
#define JRPC_CODE_NOT_IMPLEMENTED	-32000

#define JRPC_DEFAULT_EPOLL_USLEEP	1000
#define JRPC_DEFAULT_TIMEOUT		3000
#define JRPC_DEFAULT_RCVBUF_STREAM	4096
#define JRPC_DEFAULT_RCVBUF_DGRAM	65535
#define JRPC_DEFAULT_PROTO		0
#define JRPC_DEFAULT_HOST		IPSC_HOST_LOCAL
#define JRPC_DEFAULT_SOCKTYPE		SOCK_STREAM
#define JRPC_DEFAULT_MAXQUEUE		IPSC_MAX_QUEUE_DEFAULT

/* TODO */
#define JRPC_FLAG_SIGN			0x01
#define JRPC_FLAG_ENCRYPT		0x02
#define JRPC_FLAG_BINARIZE		0x04

/* return codes */
#define JRPC_SUCCESS			1
#define JRPC_ERR_USER			0
#define JRPC_ERR_GENERIC		-1
#define JRPC_ERR_RECV			-2
#define JRPC_ERR_SEND			-3
#define JRPC_ERR_NORESULT		-4
#define JRPC_ERR_UNKNOWN_REPLY_TYPE	-5

/* param availability flags */
enum {
	JRPC_CB_NO_PARAMS,
	JRPC_CB_HAS_PARAMS,
	JRPC_CB_OPT_PARAMS
};

/* reply types */
enum {
	JRPC_REPLY_TYPE_ERROR,
	JRPC_REPLY_TYPE_RESULT
};

/* connection register callback, useful for joinable threads */
typedef void (*jrpc_connreg_t)( void *ptr );

/* method handler */
typedef ssize_t (*jrpc_cb_t)( ipsc_t *ipsc, void *params, void *reqid );

/* method structure */
typedef struct jrpc_method_t {
	char *name;
	int params;
	jrpc_cb_t *handlers;
} jrpc_method_t;

/* TODO */
typedef struct jrpc_runtime_t {
	void *sign_ctx;
	void *encr_ctx;
	void *bin_ctx;
} jrpc_runtime_t;

typedef struct jrpc_conn_t {
	int   socktype;
	int   proto;
	char *host;
	int   port;
	int   timeout;
	int   flags;
} jrpc_conn_t;

/* server parameters */
typedef struct jrpc_t {
	jrpc_conn_t conn;
	int   maxqueue;
	int   epsleep;
	jrpc_method_t *methods;
	jrpc_connreg_t connreg;
	jrpc_runtime_t rt;
} jrpc_t;

/* client/request parameters */
typedef struct jrpc_req_t {
	jrpc_conn_t conn;
	char  *method;
	fmt_t *params;
	fmt_t *id;
	fmt_t *res;
	jrpc_runtime_t rt;
} jrpc_req_t;

/* handlers caster */
#define JRPC_CBS		(jrpc_cb_t [])
/* methods array terminator */
#define JRPC_METHODS_END	{ 0, 0, JRPC_CBS{0} }

#define JRPC_DEFAULT_CONN {			\
	.socktype = JRPC_DEFAULT_SOCKTYPE,	\
	.proto    = JRPC_DEFAULT_PROTO,		\
	.host     = JRPC_DEFAULT_HOST,		\
	.timeout  = JRPC_DEFAULT_TIMEOUT,	\
	.flags    = 0				\
}

/* server init macro */
#define JRPC_SERVER_DEFAULT {			\
	.conn     = JRPC_DEFAULT_CONN,		\
	.maxqueue = JRPC_DEFAULT_MAXQUEUE,	\
	.epsleep  = JRPC_DEFAULT_EPOLL_USLEEP,	\
	.methods  = NULL,			\
	.connreg  = NULL			\
}

/* client init macro */
#define JRPC_CLIENT_DEFAULT {			\
	.conn   = JRPC_DEFAULT_CONN,		\
	.method = NULL,				\
	.params = NULL,				\
	.id     = NULL,				\
	.res    = NULL				\
}

/* server thread */
void *jrpc_server( void *args );

/* client */
ssize_t jrpc_request( jrpc_req_t *req );

/* to be used in method handlers */
ssize_t jrpc_send( ipsc_t *ipsc, fmt_t *obj, fmt_t *id, int type );

/* error helpers */
ssize_t jrpc_error( ipsc_t *ipsc, fmt_t *id, int code, const char *message );
ssize_t jrpc_invalid_params( ipsc_t *ipsc, fmt_t *id );
ssize_t jrpc_internal_error( ipsc_t *ipsc, fmt_t *id );
ssize_t jrpc_not_implemented( ipsc_t *ipsc, fmt_t *id );

#endif /* _JRPC_H_ */
