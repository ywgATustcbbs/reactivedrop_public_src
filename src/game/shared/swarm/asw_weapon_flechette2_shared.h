#ifndef _INCLUDED_ASW_WEAPON_FLECHETTE2_H
#define _INCLUDED_ASW_WEAPON_FLECHETTE2_H
#pragma once

#ifdef CLIENT_DLL
#define CASW_Weapon_Flechette2 C_ASW_Weapon_Flechette2
#define CASW_Weapon_Rifle C_ASW_Weapon_Rifle
#include "c_asw_weapon_rifle.h"
#else
#include "npc_combine.h"
#include "asw_weapon_rifle.h"
#endif

class CASW_Weapon_Flechette2 : public CASW_Weapon_Rifle
{
public:
	DECLARE_CLASS( CASW_Weapon_Flechette2, CASW_Weapon_Rifle );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CASW_Weapon_Flechette2();
	virtual ~CASW_Weapon_Flechette2();
	void Precache();

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();

	virtual const float GetAutoAimAmount() { return 0.26f; }
	virtual bool ShouldFlareAutoaim() { return true; }
	virtual bool SupportsDamageModifiers() { return false; }

#ifndef CLIENT_DLL
#else
	virtual bool HasSecondaryExplosive( void ) const { return false; }
#endif
	virtual const char *GetMagazineGibModelName() const override { return "models/weapons/empty_clips/flachette_empty_clip.mdl"; }
	virtual bool HasSecondaryAttack() override { return false; } // weapon has no secondary fire

	// Classification
	virtual Class_T		Classify( void ) { return ( Class_T )CLASS_ASW_FLECHETTE2; }
};

#endif /* _INCLUDED_ASW_WEAPON_FLECHETTE2_H */
