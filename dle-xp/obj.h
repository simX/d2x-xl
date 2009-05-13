// Copyright (C) 1997 Bryan Aamot
//------------------------------------------------------------------------
// DIALOG - TObjectDialog
//------------------------------------------------------------------------
class TObjectDialog : public TDialog {
  TComboBox *TypeComboBox,*IdComboBox;
  TComboBox *ContainsTypeComboBox,*ContainsIdComboBox;
  TEdit *ContainsQtyEdit;
  TComboBox *ObjectNumberComboBox,*OptionComboBox;
  TStatic *ObjectStatic;
  TButton *ImageButton,*ContainsButton;
  TComboBox *TextureComboBox;
  TButton *TextureButton;
  TButton *EditObjectButton;
  TButton *AddObjectButton;
  TButton *DeleteObjectButton;
public:
  TObjectDialog(TWindow * AParent, LPSTR name);
  void EvDrawItem( UINT, DRAWITEMSTRUCT FAR & );
  virtual void SetupWindow();
  void AddObjectMsg();
  void DeleteObjectMsg();
  void EditObjectMsg();
  void AdvancedMsg();
  void ResetObjectMsg();
  void MoveObjectMsg();
  void ObjectNumberMsg();
  void TypeMsg();
  void IdMsg();
  void ContainsQtyMsg();
  void ContainerTypeMsg();
  void ContainsIdMsg();
  void OptionMsg();
  void TextureMsg();
  LRESULT User( WPARAM, LPARAM );
// OWLCVT: Unrecognized DDVT value 1280 for 'User', generic signature used
  void RefreshData();
  void SetObjectId(TComboBox *TheComboBox,INT16 type,INT16 id, INT16 flag=0);
  void DrawObjectImage();
  void DrawObject(int type, int id, int xoffset, int yoffset);
  void SetTextureOverride();

DECLARE_RESPONSE_TABLE( TObjectDialog );
};

//------------------------------------------------------------------------
// DIALOG - TAdvObjectDialog
//------------------------------------------------------------------------
class TAdvObjectDialog : public TDialog {
	TEdit *MassEdit;
	TEdit *DragEdit;
	TEdit *BrakesEdit;
	TEdit *TurnRollEdit;
	TEdit *FlagsEdit;
	TEdit *SizeEdit;
	TEdit *ShieldEdit;
	TEdit *VelocityXEdit;
	TEdit *VelocityYEdit;
	TEdit *VelocityZEdit;
	TEdit *ThrustXEdit;
	TEdit *ThrustYEdit;
	TEdit *ThrustZEdit;
	TEdit *RotVelXEdit;
	TEdit *RotVelYEdit;
	TEdit *RotVelZEdit;
	TEdit *RotThrustXEdit;
	TEdit *RotThrustYEdit;
	TEdit *RotThrustZEdit;
	TEdit *ModelClipEdit;
	TEdit *FrameEdit;
	TEdit *FrameNumEdit;
public:
	TAdvObjectDialog(TWindow * AParent, LPSTR name);
	virtual void SetupWindow();
	LRESULT User( WPARAM, LPARAM );
	void RefreshData();
	void SetEdit(TEdit *edit,long value);
	void GetEdit(TEdit *edit,long *value);
	void ResetEdit(TEdit *edit);
	void MassMsg();
	void DragMsg();
	void BrakesMsg();
	void TurnRollMsg();
	void FlagsMsg();
	void SizeMsg();
	void ShieldMsg();
	void VelocityXMsg();
	void VelocityYMsg();
	void VelocityZMsg();
	void ThrustXMsg();
	void ThrustYMsg();
	void ThrustZMsg();
	void RotVelXMsg();
	void RotVelYMsg();
	void RotVelZMsg();
	void RotThrustXMsg();
	void RotThrustYMsg();
	void RotThrustZMsg();
	void ModelClipMsg();
	void FrameMsg();
	void FrameNumMsg();

	DECLARE_RESPONSE_TABLE( TAdvObjectDialog );
};



