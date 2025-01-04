#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* if you want to define some other proto for communications, e.g. SCTP */
#include <netinet/in.h>

#include <libjrpc.h>

int main( int argc, char **argv )
{
	int err = 0;
	json_object *res = NULL;
	json_object *params = NULL;
	const char *reply = NULL;

	if ( argc < 2 ) {
		printf("Usage: %s <method> <params>\n", argv[0] );
		exit(EXIT_FAILURE);
	}

	if ( argc > 2 )
		params = json_tokener_parse( argv[2] );

	jrpc_req_t rpc_req = JRPC_CLIENT_DEFAULT;
	rpc_req.conn.proto = IPPROTO_TCP;
	rpc_req.conn.host  = "127.0.0.1";
	rpc_req.conn.port  = 8888;
	rpc_req.method     = argv[1];
	rpc_req.params     = params;
	rpc_req.res        = &res;
//	rpc_req.conn.flags |= JRPC_FLAG_TLS;
//	rpc_req.conn.tlscert= "client.crt";
//	rpc_req.conn.tlskey = "client.key";
//	rpc_req.conn.tlsca  = "rootCA.crt";
//	rpc_req.conn.tlsdh  = "dhparams2048.pem";

	err = jrpc_request( &rpc_req );
	if ( err < 0 ) {
		printf( "jrpc_request() error: %i (%m)\n", err );
		exit( EXIT_FAILURE );
	}

	if ( err == JRPC_ERR_USER )
		printf( "Received some error!\n" );
	else
		printf( "Received some result!\n" );

	reply = json_object_to_json_string_ext( res, JSON_C_TO_STRING_PRETTY_TAB );
	if ( !reply ) {
		printf( "json_object_to_json_string_ex() error\n" );
		json_object_put( res );
		exit( EXIT_FAILURE );
	}

	printf( "%s\n", reply );
	json_object_put( res );

	exit(EXIT_SUCCESS);
}
