// Copyright (C) 1997 Bryan Aamot
//************************************************************************
// DIALOG - TWallDialog
//************************************************************************
class TWallDialog : public TDialog {
	TComboBox *WallNumberCombo,*WallTypeCombo;
//  TComboBox *WallTriggerCombo;
	TEdit *WallStrengthEdit,*WallCloakEdit;
	TComboBox *WallClipCombo;
	TCheckBox *WallFlags[7],*KeyFlags[4];
	TStatic *CubeSideStatic;
public:
	TWallDialog(TWindow * AParent, LPSTR name);
	virtual void SetupWindow();
	void AddWallMsg();
	void DeleteWallMsg();
	void OtherSideMsg();
	void WallNumberMsg();
	void WallTypeMsg();
	void WallStrengthMsg();
	void WallClipMsg();
	void WallCloakMsg();
	void WallFlagsMsg();
	void KeyFlags0Msg();
	void KeyFlags1Msg();
	void KeyFlags2Msg();
	void KeyFlags3Msg();
	LRESULT User( WPARAM, LPARAM );
	void RefreshData();
DECLARE_RESPONSE_TABLE( TWallDialog );
};
