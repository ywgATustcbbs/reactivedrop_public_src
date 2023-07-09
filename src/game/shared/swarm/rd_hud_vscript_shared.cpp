#include "cbase.h"
#include "rd_hud_vscript_shared.h"
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
CUtlVector<CRD_HUD_VScript *> CRD_HUD_VScript::s_HUDEntities;
#else
LINK_ENTITY_TO_CLASS( rd_hud_vscript, CRD_HUD_VScript );

BEGIN_DATADESC( CRD_HUD_VScript )
	DEFINE_KEYFIELD( m_szClientVScript, FIELD_STRING, "client_vscript" ),

	DEFINE_FIELD( m_hDataEntity, FIELD_EHANDLE ),
	DEFINE_AUTO_ARRAY( m_iDataInt, FIELD_INTEGER ),
	DEFINE_AUTO_ARRAY( m_flDataFloat, FIELD_FLOAT ),
	DEFINE_AUTO_ARRAY( m_szDataString, FIELD_CHARACTER ),
END_DATADESC();
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( RD_HUD_VScript, DT_RD_HUD_VScript );

BEGIN_NETWORK_TABLE( CRD_HUD_VScript, DT_RD_HUD_VScript )
#ifdef CLIENT_DLL
	RecvPropString( RECVINFO( m_szClientVScript ) ),
	RecvPropEHandle( RECVINFO( m_hDataEntity ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_iDataInt ), RecvPropInt( RECVINFO( m_iDataInt[0] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_flDataFloat ), RecvPropFloat( RECVINFO( m_flDataFloat[0] ) ) ),
	RecvPropString( RECVINFO( m_szDataString ) ),
#else
	SendPropStringT( SENDINFO( m_szClientVScript ) ),
	SendPropEHandle( SENDINFO( m_hDataEntity ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_iDataInt ), SendPropInt( SENDINFO_ARRAY( m_iDataInt ) ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_flDataFloat ), SendPropFloat( SENDINFO_ARRAY( m_flDataFloat ) ) ),
	SendPropString( SENDINFO( m_szDataString ) ),
#endif
END_NETWORK_TABLE();

BEGIN_ENT_SCRIPTDESC( CRD_HUD_VScript, CBaseEntity, "Alien Swarm: Reactive Drop scriptable HUD" )
#ifdef CLIENT_DLL
	DEFINE_SCRIPTFUNC_NAMED( Script_LookupTexture, "LookupTexture", "Find a material for rendering on the HUD. Should not be called during Paint" )
	DEFINE_SCRIPTFUNC_NAMED( Script_LookupFont, "LookupFont", "Find a font by name. Should not be called during Paint" )
	DEFINE_SCRIPTFUNC_NAMED( Script_GetFontTall, "GetFontTall", "Returns the height of a font in pixels" )
	DEFINE_SCRIPTFUNC_NAMED( Script_GetTextWide, "GetTextWide", "Returns the width of the given text in pixels" )
	DEFINE_SCRIPTFUNC_NAMED( Script_PaintText, "PaintText", "Draw text on the heads-up display. Can only be called during Paint" )
	DEFINE_SCRIPTFUNC_NAMED( Script_PaintRectangle, "PaintRectangle", "Draw a solid-colored rectangle on the heads-up display. Can only be called during Paint" )
	DEFINE_SCRIPTFUNC_NAMED( Script_PaintRectangleFade, "PaintRectangleFade", "Draw a solid-colored rectangle with a gradient of opacity on the heads-up display. Can only be called during Paint" )
	DEFINE_SCRIPTFUNC_NAMED( Script_PaintTexturedRectangle, "PaintTexturedRectangle", "Draw a textured rectangle on the heads-up display. Can only be called during Paint" )
	DEFINE_SCRIPTFUNC_NAMED( Script_PaintTexturedRectangleAdvanced, "PaintTexturedRectangleAdvanced", "Draw a textured rectangle with advanced settings on the heads-up display. Can only be called during Paint" )
#else
	DEFINE_SCRIPTFUNC( SetEntity, "Set the value of an entity parameter." )
	DEFINE_SCRIPTFUNC( SetInt, "Set the value of an integer parameter." )
	DEFINE_SCRIPTFUNC( SetFloat, "Set the value of a float parameter." )
	DEFINE_SCRIPTFUNC( SetString, "Set the value of a string parameter." )
#endif
	DEFINE_SCRIPTFUNC( GetEntity, "Get the value of an entity parameter." )
	DEFINE_SCRIPTFUNC( GetInt, "Get the value of an integer parameter." )
	DEFINE_SCRIPTFUNC( GetFloat, "Get the value of a float parameter." )
	DEFINE_SCRIPTFUNC( GetString, "Get the value of a string parameter." )
END_SCRIPTDESC();

CRD_HUD_VScript::CRD_HUD_VScript()
{
#ifdef CLIENT_DLL
	s_HUDEntities.AddToTail( this );
	V_memset( m_szClientVScript.GetForModify(), 0, sizeof( m_szClientVScript ) );
#else
	m_szClientVScript = NULL_STRING;
#endif

	m_hDataEntity = NULL;
	FOR_EACH_VEC( m_iDataInt, i )
		m_iDataInt.Set( i, 0 );
	FOR_EACH_VEC( m_flDataFloat, i )
		m_flDataFloat.Set( i, 0.0f );
	V_memset( m_szDataString.GetForModify(), 0, sizeof( m_szDataString ) );
}

CRD_HUD_VScript::~CRD_HUD_VScript()
{
#ifdef CLIENT_DLL
	s_HUDEntities.FindAndRemove( this );

	if ( m_ScriptScope.IsInitialized() )
	{
		m_ScriptScope.ReleaseFunction( m_hUpdateFunc );
		m_ScriptScope.ReleaseFunction( m_hPaintFunc );
	}

	for ( unsigned int i = 0; i < m_Textures.Count(); i++ )
	{
		vgui::surface()->DestroyTextureID( m_Textures[i] );
	}
	m_Textures.Purge();
#endif
}

#ifdef CLIENT_DLL
void CRD_HUD_VScript::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( !m_ScriptScope.IsInitialized() && m_szClientVScript.Get()[0] != '\0' )
	{
		// has the side effect of setting m_iszScriptId
		HSCRIPT hInstance = GetScriptInstance();

		m_ScriptScope.Init( m_iszScriptId );
		m_ScriptScope.SetValue( "self", hInstance );

		VScriptRunScript( m_szClientVScript, m_ScriptScope, true );

		m_hUpdateFunc = m_ScriptScope.LookupFunction( "OnUpdate" );
		m_hPaintFunc = m_ScriptScope.LookupFunction( "Paint" );

		if ( !m_hUpdateFunc || m_hUpdateFunc == INVALID_HSCRIPT )
		{
			DevWarning( "rd_hud_vscript (%s) does not have an OnUpdate function in its script scope.\n", m_szClientVScript.Get() );
		}
		if ( !m_hPaintFunc || m_hPaintFunc == INVALID_HSCRIPT )
		{
			Warning( "rd_hud_vscript (%s) does not have a Paint function in its script scope.\n", m_szClientVScript.Get() );
		}
	}

	if ( m_hUpdateFunc != INVALID_HSCRIPT )
	{
		g_pScriptVM->Call( m_hUpdateFunc, m_ScriptScope );
	}
}

void CRD_HUD_VScript::Paint()
{
	Assert( !m_bIsPainting );
	if ( m_hPaintFunc != INVALID_HSCRIPT )
	{
		m_bIsPainting = true;
		g_pScriptVM->Call( m_hPaintFunc, m_ScriptScope );
		m_bIsPainting = false;
	}
}

int CRD_HUD_VScript::Script_LookupTexture( const char *name )
{
	if ( m_bIsPainting )
		DevWarning( "rd_hud_vscript (%s): LookupTexture should not be called during Paint!\n", m_szClientVScript );

	int i = m_Textures.Find( name );
	if ( !m_Textures.IsValidIndex( i ) )
	{
		i = m_Textures.Insert( name );
		m_Textures[i] = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_Textures[i], name, 1, false );
	}

	return m_Textures[i];
}

int CRD_HUD_VScript::Script_LookupFont( const char *name )
{
	if ( m_bIsPainting )
		DevWarning( "rd_hud_vscript (%s): LookupFont should not be called during Paint!\n", m_szClientVScript );

	vgui::HScheme hScheme = vgui::scheme()->LoadSchemeFromFile( "resource/SwarmSchemeNew.res", "SwarmSchemeNew" );
	vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( hScheme );
	Assert( pScheme );
	if ( !pScheme )
		return vgui::INVALID_FONT;

	vgui::HFont hFont = pScheme->GetFont( name, true );
	if ( !hFont )
	{
		Warning( "rd_hud_vscript (%s): LookupFont did not find a font named %s\n", m_szClientVScript, name );
	}

	return hFont;
}

int CRD_HUD_VScript::Script_GetFontTall( int font )
{
	return vgui::surface()->GetFontTall( font );
}

int CRD_HUD_VScript::Script_GetTextWide( int font, const char *text )
{
	wchar_t wszText[2048];
	V_UTF8ToUnicode( text, wszText, sizeof( wszText ) );

	int wide, tall;
	vgui::surface()->GetTextSize( font, wszText, wide, tall );

	return wide;
}

void CRD_HUD_VScript::Script_PaintText( int x, int y, int r, int g, int b, int a, int font, const char *text )
{
	if ( !m_bIsPainting )
	{
		Warning( "rd_hud_vscript (%s): PaintText cannot be called outside of Paint!\n", m_szClientVScript );
		return;
	}

	wchar_t wsz[2048];
	V_UTF8ToUnicode( text, wsz, sizeof( wsz ) );

	vgui::surface()->DrawSetTextFont( font );
	vgui::surface()->DrawSetTextColor( r, g, b, a );
	vgui::surface()->DrawSetTextPos( x, y );
	vgui::surface()->DrawUnicodeString( wsz );
}

void CRD_HUD_VScript::Script_PaintRectangle( int x0, int y0, int x1, int y1, int r, int g, int b, int a )
{
	if ( !m_bIsPainting )
	{
		Warning( "rd_hud_vscript (%s): PaintRectangle cannot be called outside of Paint!\n", m_szClientVScript );
		return;
	}

	vgui::surface()->DrawSetColor( r, g, b, a );
	vgui::surface()->DrawFilledRect( x0, y0, x1, y1 );
}

void CRD_HUD_VScript::Script_PaintRectangleFade( int x0, int y0, int x1, int y1, int r, int g, int b, int a0, int a1, int fadeStart, int fadeEnd, bool horizontal )
{
	if ( !m_bIsPainting )
	{
		Warning( "rd_hud_vscript (%s): PaintRectangleFade cannot be called outside of Paint!\n", m_szClientVScript );
		return;
	}

	vgui::surface()->DrawSetColor( r, g, b, 255 );
	vgui::surface()->DrawFilledRectFastFade( x0, y0, x1, y1, fadeStart, fadeEnd, a0, a1, horizontal );
}

void CRD_HUD_VScript::Script_PaintTexturedRectangle( int x0, int y0, int x1, int y1, int r, int g, int b, int a, int texture )
{
	if ( !m_bIsPainting )
	{
		Warning( "rd_hud_vscript (%s): PaintTexturedRectangle cannot be called outside of Paint!\n", m_szClientVScript );
		return;
	}

	vgui::surface()->DrawSetColor( r, g, b, a );
	vgui::surface()->DrawSetTexture( texture );
	vgui::surface()->DrawTexturedRect( x0, y0, x1, y1 );
}

void CRD_HUD_VScript::Script_PaintTexturedRectangleAdvanced( HSCRIPT table )
{
	if ( !m_bIsPainting )
	{
		Warning( "rd_hud_vscript (%s): PaintTexturedRectangleAdvanced cannot be called outside of Paint!\n", m_szClientVScript );
		return;
	}

	ScriptVariant_t value;
	vgui::DrawTexturedRectParms_t params;

	if ( g_pScriptVM->GetValue( table, "x0", &value ) )
		value.AssignTo( &params.x0 );
	if ( g_pScriptVM->GetValue( table, "y0", &value ) )
		value.AssignTo( &params.y0 );
	if ( g_pScriptVM->GetValue( table, "x1", &value ) )
		value.AssignTo( &params.x1 );
	if ( g_pScriptVM->GetValue( table, "y1", &value ) )
		value.AssignTo( &params.y1 );

	int a = 255;
	if ( g_pScriptVM->GetValue( table, "a0", &value ) )
	{
		value.AssignTo( &a );
		params.alpha_ul = a;
	}
	if ( g_pScriptVM->GetValue( table, "a1", &value ) )
	{
		value.AssignTo( &a );
		params.alpha_ur = a;
	}
	if ( g_pScriptVM->GetValue( table, "a2", &value ) )
	{
		value.AssignTo( &a );
		params.alpha_ll = a;
	}
	if ( g_pScriptVM->GetValue( table, "a3", &value ) )
	{
		value.AssignTo( &a );
		params.alpha_lr = a;
	}

	if ( g_pScriptVM->GetValue( table, "s0", &value ) )
		value.AssignTo( &params.s0 );
	if ( g_pScriptVM->GetValue( table, "t0", &value ) )
		value.AssignTo( &params.t0 );
	if ( g_pScriptVM->GetValue( table, "s1", &value ) )
		value.AssignTo( &params.s1 );
	if ( g_pScriptVM->GetValue( table, "t1", &value ) )
		value.AssignTo( &params.t1 );

	if ( g_pScriptVM->GetValue( table, "angle", &value ) )
		value.AssignTo( &params.angle );

	int r = 255, g = 255, b = 255, texture = 0;
	if ( g_pScriptVM->GetValue( table, "r", &value ) )
		value.AssignTo( &r );
	if ( g_pScriptVM->GetValue( table, "g", &value ) )
		value.AssignTo( &g );
	if ( g_pScriptVM->GetValue( table, "b", &value ) )
		value.AssignTo( &b );
	if ( g_pScriptVM->GetValue( table, "texture", &value ) )
		value.AssignTo( &texture );

	vgui::surface()->DrawSetColor( r, g, b, 255 );
	vgui::surface()->DrawSetTexture( texture );
	vgui::surface()->DrawTexturedRectEx( &params );
}
#else
int CRD_HUD_VScript::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	return FL_EDICT_ALWAYS;
}

int CRD_HUD_VScript::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

void CRD_HUD_VScript::SetEntity( int i, HSCRIPT entity )
{
	if ( i != 0 )
	{
		Warning( "Entity index %d is not allowed for rd_hud_vscript (%s)\n", i, STRING( m_szClientVScript.Get() ) );
		return;
	}

	m_hDataEntity.Set( ToEnt( entity ) );
}

void CRD_HUD_VScript::SetInt( int i, int value )
{
	if ( i < 0 || i >= m_iDataInt.Count() )
	{
		Warning( "Integer index %d is not allowed for rd_hud_vscript (%s)\n", i, STRING( m_szClientVScript.Get() ) );
		return;
	}

	m_iDataInt.Set( i, value );
}

void CRD_HUD_VScript::SetFloat( int i, float value )
{
	if ( i < 0 || i >= m_flDataFloat.Count() )
	{
		Warning( "Float index %d is not allowed for rd_hud_vscript (%s)\n", i, STRING( m_szClientVScript.Get() ) );
		return;
	}

	m_flDataFloat.Set( i, value );
}

void CRD_HUD_VScript::SetString( int i, const char *string )
{
	if ( i != 0 )
	{
		Warning( "String index %d is not allowed for rd_hud_vscript (%s)\n", i, STRING( m_szClientVScript.Get() ) );
		return;
	}

	if ( V_strcmp( m_szDataString.Get(), string ) )
	{
		V_strncpy( m_szDataString.GetForModify(), string, sizeof( m_szDataString ) );
	}
}
#endif

HSCRIPT CRD_HUD_VScript::GetEntity( int i ) const
{
	if ( i != 0 )
	{
		Warning( "Entity index %d is not allowed for rd_hud_vscript (%s)\n", i, STRING( m_szClientVScript.Get() ) );
		return NULL;
	}

	return ToHScript( m_hDataEntity.Get() );
}

int CRD_HUD_VScript::GetInt( int i ) const
{
	if ( i < 0 || i >= m_iDataInt.Count() )
	{
		Warning( "Integer index %d is not allowed for rd_hud_vscript (%s)\n", i, STRING( m_szClientVScript.Get() ) );
		return 0;
	}

	return m_iDataInt.Get( i );
}

float CRD_HUD_VScript::GetFloat( int i ) const
{
	if ( i < 0 || i >= m_flDataFloat.Count() )
	{
		Warning( "Float index %d is not allowed for rd_hud_vscript (%s)\n", i, STRING( m_szClientVScript.Get() ) );
		return 0.0f;
	}

	return m_flDataFloat.Get( i );
}

const char *CRD_HUD_VScript::GetString( int i ) const
{
	if ( i != 0 )
	{
		Warning( "String index %d is not allowed for rd_hud_vscript (%s)\n", i, STRING( m_szClientVScript.Get() ) );
		return "";
	}

	return m_szDataString.Get();
}