#pragma once

#include <stdint.h>

#if defined( _WIN32 )
#	if defined( ASRD_GNS_WRAPPER_EXPORTS )
#		define ASRD_GNS_WRAPPER_API __declspec( dllexport )
#	else
#		define ASRD_GNS_WRAPPER_API __declspec( dllimport )
#	endif
#else
#	define ASRD_GNS_WRAPPER_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// This is an opaque game-side token.  The wrapper owns the actual GNS handle.
typedef uint32_t ASRD_GNS_Connection;

enum
{
	ASRD_GNS_CONNECTION_INVALID = 0,
	ASRD_GNS_CONNECTION_ACTIVE = 1,
};

// The game-facing ABI deliberately contains no GameNetworkingSockets types or headers.
ASRD_GNS_WRAPPER_API int ASRD_GNS_Initialize( int serverRole );
ASRD_GNS_WRAPPER_API int ASRD_GNS_Listen( uint16_t port );
ASRD_GNS_WRAPPER_API ASRD_GNS_Connection ASRD_GNS_Connect( const char *ipv4, uint16_t port );
ASRD_GNS_WRAPPER_API void ASRD_GNS_RunFrame( void );
ASRD_GNS_WRAPPER_API int ASRD_GNS_SendReliable( ASRD_GNS_Connection connection, const void *data, uint32_t size );
ASRD_GNS_WRAPPER_API int ASRD_GNS_Receive( ASRD_GNS_Connection connection, void *buffer, uint32_t capacity, uint32_t *size );
ASRD_GNS_WRAPPER_API void ASRD_GNS_Close( ASRD_GNS_Connection connection );
ASRD_GNS_WRAPPER_API void ASRD_GNS_Shutdown( void );

#ifdef __cplusplus
}
#endif

