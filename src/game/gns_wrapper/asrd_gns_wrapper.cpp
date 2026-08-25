#include "asrd_gns_wrapper.h"

#include <winsock2.h>
#include <windows.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// This translation unit is the only place in the game source tree that includes
// GameNetworkingSockets headers.  The library is linked statically into this DLL.
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

namespace
{
	static bool s_initialized = false;
	static bool s_serverRole = false;
	static ISteamNetworkingSockets *s_sockets = NULL;
	static HSteamListenSocket s_listenSocket = k_HSteamListenSocket_Invalid;
	static HSteamNetConnection s_connection = k_HSteamNetConnection_Invalid;

	static void Log( const char *format, ... )
	{
		char buffer[ 1024 ];
		va_list args;
		va_start( args, format );
		_vsnprintf_s( buffer, sizeof( buffer ), _TRUNCATE, format, args );
		va_end( args );

		OutputDebugStringA( buffer );
		fputs( buffer, stdout );
		fflush( stdout );
	}

	static HSteamNetConnection ResolveConnection( ASRD_GNS_Connection connection )
	{
		if ( connection != ASRD_GNS_CONNECTION_ACTIVE )
			return k_HSteamNetConnection_Invalid;
		return s_connection;
	}

	static void OnConnectionStatusChanged( SteamNetConnectionStatusChangedCallback_t *info )
	{
		if ( !info || !s_sockets )
			return;

		if ( info->m_info.m_eState == k_ESteamNetworkingConnectionState_Connecting )
		{
			if ( s_serverRole && info->m_info.m_hListenSocket != k_HSteamListenSocket_Invalid )
			{
				if ( s_connection != k_HSteamNetConnection_Invalid )
				{
					s_sockets->CloseConnection( info->m_hConn, 0, "single-connection wrapper", false );
					Log( "[ASRD-GNS] server rejected extra connection\n" );
					return;
				}

				const EResult result = s_sockets->AcceptConnection( info->m_hConn );
				if ( result == k_EResultOK )
				{
					s_connection = info->m_hConn;
					Log( "[ASRD-GNS] server accepted connection\n" );
				}
				else
				{
					Log( "[ASRD-GNS] server AcceptConnection failed result=%d\n", (int)result );
					s_sockets->CloseConnection( info->m_hConn, 0, "AcceptConnection failed", false );
				}
			}
			return;
		}

		if ( info->m_info.m_eState == k_ESteamNetworkingConnectionState_Connected )
		{
			s_connection = info->m_hConn;
			Log( "[ASRD-GNS] connected role=%s\n", s_serverRole ? "server" : "client" );
			return;
		}

		if ( info->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer ||
			 info->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally )
		{
			Log( "[ASRD-GNS] connection closed state=%d reason=%d\n",
				(int)info->m_info.m_eState, info->m_info.m_eEndReason );
			if ( s_connection == info->m_hConn )
				s_connection = k_HSteamNetConnection_Invalid;
			s_sockets->CloseConnection( info->m_hConn, 0, "wrapper cleanup", false );
		}
	}
}

extern "C" ASRD_GNS_WRAPPER_API int ASRD_GNS_Initialize( int serverRole )
{
	if ( s_initialized )
		return s_serverRole == ( serverRole != 0 ) ? 1 : 0;

	SteamNetworkingErrMsg errorMessage = {};
	if ( !GameNetworkingSockets_Init( NULL, errorMessage ) )
	{
		Log( "[ASRD-GNS] initialize failed: %s\n", errorMessage );
		return 0;
	}

	s_serverRole = serverRole != 0;
	s_sockets = SteamNetworkingSockets();
	if ( !s_sockets || !SteamNetworkingUtils()->SetGlobalCallback_SteamNetConnectionStatusChanged( OnConnectionStatusChanged ) )
	{
		Log( "[ASRD-GNS] initialize failed: missing standalone interfaces\n" );
		GameNetworkingSockets_Kill();
		s_sockets = NULL;
		return 0;
	}

	s_initialized = true;
	Log( "[ASRD-GNS] initialized role=%s\n", s_serverRole ? "server" : "client" );
	return 1;
}

extern "C" ASRD_GNS_WRAPPER_API int ASRD_GNS_Listen( uint16_t port )
{
	if ( !s_initialized || !s_serverRole || !s_sockets )
		return ASRD_GNS_CONNECTION_INVALID;

	if ( s_listenSocket != k_HSteamListenSocket_Invalid )
		return ASRD_GNS_CONNECTION_ACTIVE;

	SteamNetworkingIPAddr address;
	address.Clear();
	address.SetIPv4( 0x7f000001, port );
	s_listenSocket = s_sockets->CreateListenSocketIP( address, 0, NULL );
	if ( s_listenSocket == k_HSteamListenSocket_Invalid )
	{
		Log( "[ASRD-GNS] server listen failed port=%u\n", (unsigned)port );
		return ASRD_GNS_CONNECTION_INVALID;
	}

	Log( "[ASRD-GNS] server ready listen=127.0.0.1:%u\n", (unsigned)port );
	return ASRD_GNS_CONNECTION_ACTIVE;
}

extern "C" ASRD_GNS_WRAPPER_API ASRD_GNS_Connection ASRD_GNS_Connect( const char *ipv4, uint16_t port )
{
	if ( !s_initialized || s_serverRole || !s_sockets )
		return ASRD_GNS_CONNECTION_INVALID;

	if ( s_connection != k_HSteamNetConnection_Invalid )
		s_sockets->CloseConnection( s_connection, 0, "replacing wrapper connection", false );

	const char *target = ( ipv4 && ipv4[0] ) ? ipv4 : "127.0.0.1";
	const unsigned long packedAddress = inet_addr( target );
	if ( packedAddress == INADDR_NONE && strcmp( target, "255.255.255.255" ) != 0 )
	{
		Log( "[ASRD-GNS] client rejected non-IPv4 target=%s\n", target );
		return ASRD_GNS_CONNECTION_INVALID;
	}

	SteamNetworkingIPAddr address;
	address.Clear();
	address.SetIPv4( ntohl( packedAddress ), port );
	s_connection = s_sockets->ConnectByIPAddress( address, 0, NULL );
	if ( s_connection == k_HSteamNetConnection_Invalid )
	{
		Log( "[ASRD-GNS] client connect failed target=%s:%u\n", target, (unsigned)port );
		return ASRD_GNS_CONNECTION_INVALID;
	}

	Log( "[ASRD-GNS] client connecting target=%s:%u\n", target, (unsigned)port );
	return ASRD_GNS_CONNECTION_ACTIVE;
}

extern "C" ASRD_GNS_WRAPPER_API void ASRD_GNS_RunFrame( void )
{
	if ( s_initialized && s_sockets )
		s_sockets->RunCallbacks();
}

extern "C" ASRD_GNS_WRAPPER_API int ASRD_GNS_SendReliable( ASRD_GNS_Connection connection, const void *data, uint32_t size )
{
	if ( size > 0 && !data )
		return 0;

	const HSteamNetConnection nativeConnection = ResolveConnection( connection );
	if ( !s_sockets || nativeConnection == k_HSteamNetConnection_Invalid )
		return 0;

	const EResult result = s_sockets->SendMessageToConnection(
		nativeConnection, data, (uint32)size, k_nSteamNetworkingSend_Reliable, NULL );
	return result == k_EResultOK ? 1 : 0;
}

extern "C" ASRD_GNS_WRAPPER_API int ASRD_GNS_Receive( ASRD_GNS_Connection connection, void *buffer, uint32_t capacity, uint32_t *size )
{
	if ( size )
		*size = 0;

	const HSteamNetConnection nativeConnection = ResolveConnection( connection );
	if ( !s_sockets || nativeConnection == k_HSteamNetConnection_Invalid )
		return -1;

	SteamNetworkingMessage_t *message = NULL;
	const int count = s_sockets->ReceiveMessagesOnConnection( nativeConnection, &message, 1 );
	if ( count <= 0 || !message )
		return count < 0 ? -1 : 0;

	const bool fits = message->m_cbSize >= 0 && (uint32_t)message->m_cbSize <= capacity &&
		( message->m_cbSize == 0 || buffer != NULL );
	if ( fits )
	{
		if ( message->m_cbSize > 0 )
			memcpy( buffer, message->m_pData, (size_t)message->m_cbSize );
		if ( size )
			*size = (uint32_t)message->m_cbSize;
	}

	message->Release();
	return fits ? 1 : -1;
}

extern "C" ASRD_GNS_WRAPPER_API void ASRD_GNS_Close( ASRD_GNS_Connection connection )
{
	const HSteamNetConnection nativeConnection = ResolveConnection( connection );
	if ( s_sockets && nativeConnection != k_HSteamNetConnection_Invalid )
	{
		s_sockets->CloseConnection( nativeConnection, 0, "game wrapper close", false );
		if ( s_connection == nativeConnection )
			s_connection = k_HSteamNetConnection_Invalid;
	}
}

extern "C" ASRD_GNS_WRAPPER_API void ASRD_GNS_Shutdown( void )
{
	if ( !s_initialized )
		return;

	if ( s_sockets && s_connection != k_HSteamNetConnection_Invalid )
		s_sockets->CloseConnection( s_connection, 0, "wrapper shutdown", false );
	if ( s_sockets && s_listenSocket != k_HSteamListenSocket_Invalid )
		s_sockets->CloseListenSocket( s_listenSocket );

	GameNetworkingSockets_Kill();
	s_connection = k_HSteamNetConnection_Invalid;
	s_listenSocket = k_HSteamListenSocket_Invalid;
	s_sockets = NULL;
	s_initialized = false;
	s_serverRole = false;
	Log( "[ASRD-GNS] shutdown\n" );
}
