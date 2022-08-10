#pragma once

#include "tabbedgriddetails.h"
#include "steam/steam_api.h"

class CASW_WeaponInfo;
class CASW_Model_Panel;

void LaunchCollectionsFrame();

class CRD_Collection_Tab_Equipment : public TGD_Tab
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Tab_Equipment, TGD_Tab );
public:
	CRD_Collection_Tab_Equipment( TabbedGridDetails *parent, const char *szLabel, bool bExtra );

	virtual TGD_Grid *CreateGrid() override;
	virtual TGD_Details *CreateDetails() override;

	bool m_bExtra;
};

class CRD_Collection_Details_Equipment : public TGD_Details
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Details_Equipment, TGD_Details );
public:
	CRD_Collection_Details_Equipment( CRD_Collection_Tab_Equipment *parent );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void OnThink() override;
	virtual void DisplayEntry( TGD_Entry *pEntry ) override;

	float m_flZOffset;
	CASW_Model_Panel *m_pModelPanel;
	vgui::Label *m_pWeaponNameLabel;
	vgui::Label *m_pWeaponDescLabel;
};

class CRD_Collection_Entry_Equipment : public TGD_Entry
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Entry_Equipment, TGD_Entry );
public:
	CRD_Collection_Entry_Equipment( TGD_Grid *parent, const char *panelName, int iEquipIndex, const char *szEquipClass );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void ApplyEntry() override;

	int m_iEquipIndex;
	CASW_WeaponInfo *m_pWeaponInfo;

	vgui::ImagePanel *m_pIcon;
	vgui::ImagePanel *m_pLockedIcon;
	vgui::Label *m_pLockedOverlay;
	vgui::Label *m_pLockedLabel;
	vgui::ImagePanel *m_pClassIcon;
	vgui::Label *m_pClassLabel;
};

class CRD_Collection_Tab_Inventory : public TGD_Tab
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Tab_Inventory, TGD_Tab );
public:
	CRD_Collection_Tab_Inventory( TabbedGridDetails *parent, const char *szLabel, const char *szSlot );
	virtual ~CRD_Collection_Tab_Inventory();

	virtual TGD_Details *CreateDetails() override;
	virtual void OnThink() override;

	void UpdateErrorMessage( TGD_Grid *pGrid );

	const char *m_szSlot;
	SteamInventoryResult_t m_hResult;
	bool m_bUnavailable;
	bool m_bForceUpdateMessage;
};

class CRD_Collection_Details_Inventory : public TGD_Details
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Details_Inventory, TGD_Details );
public:
	CRD_Collection_Details_Inventory( CRD_Collection_Tab_Inventory *parent );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void OnThink() override;
	virtual void DisplayEntry( TGD_Entry *pEntry ) override;

	vgui::ImagePanel *m_pIcon;
	vgui::RichText *m_pTitle;
	vgui::RichText *m_pDescription;
};

class CRD_Collection_Entry_Inventory : public TGD_Entry
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Entry_Inventory, TGD_Entry );
public:
	CRD_Collection_Entry_Inventory( TGD_Grid *parent, const char *panelName, int index, SteamItemDetails_t details );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void ApplyEntry() override;

	CRD_Collection_Tab_Inventory *GetTab();

	vgui::ImagePanel *m_pIcon;
	vgui::ImagePanel *m_pEquippedMarker;

	int m_Index;
	SteamItemDetails_t m_Details;
};