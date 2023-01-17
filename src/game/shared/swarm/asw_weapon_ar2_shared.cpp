#include "cbase.h"
#include "asw_weapon_ar2_shared.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_asw_player.h"
#include "c_asw_weapon.h"
#include "c_asw_marine.h"
#define CASW_Marine C_ASW_Marine
#else
#include "asw_marine.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "npcevent.h"
#include "shot_manipulator.h"
#include "prop_combine_ball.h"
#include "asw_fail_advice.h"
#include "effect_dispatch_data.h"
#endif
#include "asw_marine_skills.h"
#include "asw_weapon_parse.h"
#include "particle_parse.h"
#include "asw_deathmatch_mode_light.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_AR2, DT_ASW_Weapon_AR2 )

BEGIN_NETWORK_TABLE( CASW_Weapon_AR2, DT_ASW_Weapon_AR2 )
#ifdef CLIENT_DLL
	// recvprops
#else
	// sendprops
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_AR2 )
	
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_ar2, CASW_Weapon_AR2 );
PRECACHE_WEAPON_REGISTER( asw_weapon_ar2 );

#ifndef CLIENT_DLL
extern ConVar asw_debug_marine_damage;

ConVar sk_weapon_ar2_alt_fire_radius( "sk_weapon_ar2_alt_fire_radius", "10", FCVAR_CHEAT );
ConVar sk_weapon_ar2_alt_fire_duration( "sk_weapon_ar2_alt_fire_duration", "2", FCVAR_CHEAT );
ConVar sk_weapon_ar2_alt_fire_mass( "sk_weapon_ar2_alt_fire_mass", "150", FCVAR_CHEAT );

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon_AR2 )
	DEFINE_FIELD( m_flDelayedFire, FIELD_TIME ),
	DEFINE_FIELD( m_bShotDelayed, FIELD_BOOLEAN ),
END_DATADESC()

#endif /* not client */

CASW_Weapon_AR2::CASW_Weapon_AR2()
{
	m_fMinRange1 = 65;
	m_fMaxRange1 = 2048;

	m_fMinRange2 = 256;
	m_fMaxRange2 = 1024;
}

CASW_Weapon_AR2::~CASW_Weapon_AR2()
{
}

void CASW_Weapon_AR2::Precache()
{
	BaseClass::Precache();

#ifdef GAME_DLL
	UTIL_PrecacheOther( "prop_combine_ball" );
	UTIL_PrecacheOther( "env_entity_dissolver" );
#endif
}

float CASW_Weapon_AR2::GetWeaponDamage()
{
	float flDamage = GetWeaponInfo()->m_flBaseDamage;

	extern ConVar rd_ar2_dmg_base;
	if ( rd_ar2_dmg_base.GetFloat() > 0 )
	{
		flDamage = rd_ar2_dmg_base.GetFloat();
	}

	if ( GetMarine() )
	{
		flDamage += MarineSkills()->GetSkillBasedValueByMarine( GetMarine(), ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_AR2_DMG );
	}

	return flDamage;
}

void CASW_Weapon_AR2::ItemPostFrame()
{
	// See if we need to fire off our secondary round
	if ( m_bShotDelayed && gpGlobals->curtime > m_flDelayedFire )
	{
		DelayedAttack();
	}

	BaseClass::ItemPostFrame();
}

void CASW_Weapon_AR2::SecondaryAttack()
{
	if ( m_bShotDelayed )
		return;

	CBaseCombatCharacter *pOwner = GetOwner();
	if ( !pOwner )
		return;

	// Must have ammo
	bool bUsesSecondary = UsesSecondaryAmmo();
	bool bUsesClips = UsesClipsForAmmo2();
	int iAmmoCount = pOwner->GetAmmoCount( m_iSecondaryAmmoType );
	if ( ( bUsesSecondary &&
		( ( bUsesClips && m_iClip2 <= 0 ) ||
			( !bUsesClips && iAmmoCount <= 0 )
			) )
		|| m_bInReload )
	{
		SendWeaponAnim( ACT_VM_DRYFIRE );
		BaseClass::WeaponSound( EMPTY );
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
		return;
	}

	// MUST call sound before removing a round from the clip of a CMachineGun
	BaseClass::WeaponSound( SPECIAL1 );

	m_bShotDelayed = true;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flDelayedFire = gpGlobals->curtime + 0.5f;
}

void CASW_Weapon_AR2::DelayedAttack( void )
{
	m_bShotDelayed = false;

	CBaseCombatCharacter *pOwner = GetOwner();
	if ( !pOwner )
		return;

	m_flNextSecondaryAttack = pOwner->m_flNextAttack = gpGlobals->curtime + 0.66f;

	WeaponSound( WPN_DOUBLE );

#ifdef GAME_DLL
	Vector vecSrc = pOwner->Weapon_ShootPosition();
	Vector vecAiming = pOwner->BodyDirection2D();
	Vector impactPoint = vecSrc + ( vecAiming * MAX_TRACE_LENGTH );

	Vector vecVelocity = vecAiming * 1000.0f;

	// Fire the combine ball
	CreateCombineBall( vecSrc,
		vecVelocity,
		sk_weapon_ar2_alt_fire_radius.GetFloat(),
		sk_weapon_ar2_alt_fire_mass.GetFloat(),
		sk_weapon_ar2_alt_fire_duration.GetFloat(),
		pOwner,
		this );
#endif

	if ( UsesClipsForAmmo2() )
	{
		m_iClip2 -= 1;
	}
	else
	{
		pOwner->RemoveAmmo( 1, m_iSecondaryAmmoType );
	}

	// Can shoot again immediately
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	// Can blow up after a short delay (so have time to release mouse button)
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;
}

bool CASW_Weapon_AR2::CanHolster()
{
	if ( m_bShotDelayed )
		return false;

	return BaseClass::CanHolster();
}