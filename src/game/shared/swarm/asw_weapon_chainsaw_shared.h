#ifndef _INCLUDED_ASW_WEAPON_CHAINSAW_SHARED_H
#define _INCLUDED_ASW_WEAPON_CHAINSAW_SHARED_H
#ifdef _WIN32
#pragma once
#endif



// Chainsaw
//   Close range melee weapon that damages anything in front when it is up and running

#ifdef CLIENT_DLL
#include "c_asw_weapon.h"
#define CASW_Weapon_Chainsaw C_ASW_Weapon_Chainsaw
#define CASW_Weapon C_ASW_Weapon
#else
#include "asw_weapon.h"
#endif

enum CHAINSAW_FIRE_STATE { FIRE_OFF, FIRE_STARTUP, FIRE_CHARGE };

class CASW_Weapon_Chainsaw : public CASW_Weapon
{
public:
	DECLARE_CLASS( CASW_Weapon_Chainsaw, CASW_Weapon );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CASW_Weapon_Chainsaw( void );
	~CASW_Weapon_Chainsaw(void);

	virtual bool	Deploy( void );
	virtual void	Precache( void );
	virtual void	Spawn( void );

	void	PrimaryAttack( void );
	void	SecondaryAttack( void ) { PrimaryAttack(); }
	virtual bool SecondaryAttackEqualsPrimary() { return true; }

	void	WeaponIdle( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	virtual void			Drop( const Vector &vecVelocity );
	virtual bool HasAmmo();
	virtual bool ShouldMarineMoveSlow();
	virtual void ItemPostFrame();
	virtual bool ShouldShowLaserPointer() { return false; }

	// check if this weapon wants to perform a sync kill
	virtual bool CheckSyncKill( byte &forced_action, short &sync_kill_ent );

#ifndef CLIENT_DLL
	DECLARE_DATADESC();

	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	virtual void GetButtons(bool& bAttack1, bool& bAttack2, bool& bReload, bool& bOldReload, bool& bOldAttack1 );
	float m_fLastForcedFireTime;
#else
	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual void ClientThink();

	bool m_bShouldUpdateActivityClient;
#endif

	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_CHAINSAW; }
	virtual bool ShouldPlayFiringAnimations() { return false; }
private:
	void	EndAttack( void );
	void	Fire( const Vector &vecOrigSrc, const Vector &vecDir );
	void	UpdateEffect( const Vector &startPoint, const Vector &endPoint );
	void SetFiringState(CHAINSAW_FIRE_STATE state);

	CNetworkVar( CHAINSAW_FIRE_STATE, m_fireState );
	float				m_flShakeTime;
	float				m_flStartFireTime;
	float				m_flDmgTime;
	float				m_flFireAnimTime;
	bool				m_bPlayedIdleSound;

	void				StartChainsawSound(); 
	void				StopChainsawSound( bool bForce = false );
	void				AdjustChainsawPitch();
	float				m_flLastHitTime;

	void				StartAttackOffSound();
	void				StopAttackOffSound();

	CSoundPatch			*m_pChainsawAttackSound;
	CSoundPatch			*m_pChainsawAttackOffSound;

	float				m_flTargetChainsawPitch; // we'll pitch down to this value
};

#endif // _INCLUDED_ASW_WEAPON_CHAINSAW_SHARED_H