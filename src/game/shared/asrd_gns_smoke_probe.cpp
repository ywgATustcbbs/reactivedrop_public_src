#include "cbase.h"
#include "tier0/icommandline.h"
#include "asrd_gns_smoke_probe.h"

#include "../gns_wrapper/asrd_gns_wrapper.h"

#include <string.h>

namespace
{
	const uint16_t kSmokePort = 27016;
	const char kProbeMessage[] = "ASRD_GNS_SMOKE";
	const char kAckMessage[] = "ASRD_GNS_ACK";

	static bool s_enabled = false;
	static bool s_serverRole = false;
	static bool s_probeSent = false;
	static bool s_ackSent = false;
	static bool s_initialized = false;
	static ASRD_GNS_Connection s_connection = ASRD_GNS_CONNECTION_INVALID;
}

bool ASRD_GNS_SmokeInit( bool serverRole )
{
	if ( !CommandLine()->CheckParm( "-gns-test" ) )
		return true;

	s_enabled = true;
	s_serverRole = serverRole;
	if ( !ASRD_GNS_Initialize( serverRole ? 1 : 0 ) )
	{
		Warning( "[ASRD-GNS] game probe initialization failed\n" );
		return false;
	}

	s_initialized = true;
	s_connection = serverRole
		? (ASRD_GNS_Connection)ASRD_GNS_Listen( kSmokePort )
		: ASRD_GNS_Connect( "127.0.0.1", kSmokePort );
	if ( s_connection == ASRD_GNS_CONNECTION_INVALID )
	{
		Warning( "[ASRD-GNS] game probe endpoint setup failed role=%s\n", serverRole ? "server" : "client" );
		ASRD_GNS_SmokeShutdown();
		return false;
	}

	Warning( "[ASRD-GNS] game probe enabled role=%s\n", serverRole ? "server" : "client" );
	return true;
}

void ASRD_GNS_SmokeFrame( void )
{
	if ( !s_enabled || !s_initialized || s_connection == ASRD_GNS_CONNECTION_INVALID )
		return;

	ASRD_GNS_RunFrame();

	if ( !s_serverRole && !s_probeSent )
	{
		if ( ASRD_GNS_SendReliable( s_connection, kProbeMessage, (uint32_t)( sizeof( kProbeMessage ) - 1 ) ) )
		{
			s_probeSent = true;
			Warning( "[ASRD-GNS] client probe sent\n" );
		}
	}

	char message[ 256 ];
	uint32_t messageSize = 0;
	const int receiveResult = ASRD_GNS_Receive( s_connection, message, (uint32_t)sizeof( message ), &messageSize );
	if ( receiveResult != 1 )
		return;

	if ( s_serverRole && messageSize == sizeof( kProbeMessage ) - 1 &&
		!memcmp( message, kProbeMessage, sizeof( kProbeMessage ) - 1 ) )
	{
		Warning( "[ASRD-GNS] server received probe message\n" );
		if ( !s_ackSent && ASRD_GNS_SendReliable( s_connection, kAckMessage, (uint32_t)( sizeof( kAckMessage ) - 1 ) ) )
		{
			s_ackSent = true;
			Warning( "[ASRD-GNS] server ack sent\n" );
		}
	}
	else if ( !s_serverRole && messageSize == sizeof( kAckMessage ) - 1 &&
		!memcmp( message, kAckMessage, sizeof( kAckMessage ) - 1 ) )
	{
		Warning( "[ASRD-GNS] client received ack\n" );
	}
}

void ASRD_GNS_SmokeShutdown( void )
{
	if ( s_connection != ASRD_GNS_CONNECTION_INVALID )
		ASRD_GNS_Close( s_connection );
	if ( s_initialized )
		ASRD_GNS_Shutdown();

	s_connection = ASRD_GNS_CONNECTION_INVALID;
	s_initialized = false;
	s_enabled = false;
	s_serverRole = false;
	s_probeSent = false;
	s_ackSent = false;
}
