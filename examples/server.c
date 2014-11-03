#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* if you want to define some other proto for communications, e.g. SCTP */
#include <netinet/in.h>

#include <liba.h>
#include "server.h"

ssize_t check_status( ipsc_t *ipsc, void *params, void *reqid )
{
	if ( !(rand()%2) ) {
		jrpc_send( ipsc, fmt_string("BUSY"),
			   (fmt_t *)reqid, JRPC_REPLY_TYPE_ERROR );
		/* return 0 to indicate that we don't want further
		   handler executions, without trigerring an error */
		return 0;
	}
	/* return positive value to enable further handler executions, if any */
	/* if we return negative jrpc_server() will consider it as an error */
	return 1;
}

ssize_t rpc_method_system_describe( ipsc_t *ipsc, void *params, void *reqid )
{
	/* simple method which doesn't take any params
	   and only returns some string */
	return jrpc_send( ipsc, fmt_string( "JSON-RPC server" ),
			  (fmt_t *)reqid, JRPC_REPLY_TYPE_RESULT );
}

ssize_t rpc_method_return_params( ipsc_t *ipsc, void *params, void *reqid )
{
	return jrpc_send( ipsc, (fmt_t *)params, (fmt_t *)reqid,
			  JRPC_REPLY_TYPE_RESULT );
}

ssize_t rpc_method_return_params2( ipsc_t *ipsc, void *params, void *reqid )
{
	fmt_t *res = (fmt_t *)params;
	/* params is set to NULL if there are no params in request */
	if ( params == NULL )
		res = fmt_string("No params set!");

	return jrpc_send( ipsc, res, (fmt_t *)reqid, JRPC_REPLY_TYPE_RESULT );
}

int main( int argc, char **argv )
{
	if ( argc < 2 || strncmp( argv[1], "-f", 3 ) )
		daemonize( "/tmp", argv[0] );

	jrpc_t json_rpc      = JRPC_SERVER_DEFAULT;
	json_rpc.conn.proto  = IPPROTO_TCP;
	json_rpc.conn.host   = "0.0.0.0";
	json_rpc.conn.port   = 8888;
	json_rpc.epsleep     = 100;
	json_rpc.methods     = rpc_methods;
//	json_rpc.conn.flags |= JRPC_FLAG_BINARIZE;

	jrpc_server( &json_rpc );

	exit(EXIT_FAILURE);
}
