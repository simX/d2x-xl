// Copyright (C) 1997 Bryan Aamot
//************************************************************************
// DIALOG - TControlDialog
//************************************************************************
class TControlDialog : public TDialog {
  TComboBox *TriggerCombo;
  TListBox  *TriggerList;
  TEdit     *TriggerEdit;
  TButton   *AddTrigger,*DelTrigger;
  TEdit     *ReactorTimeEdit, *ReactorStrengthEdit;
  TEdit	    *SecretCubeEdit, *SecretSideEdit;

public:
  TControlDialog(TWindow * AParent, LPSTR name);
  virtual void SetupWindow();
  void NumberMsg();
  void TriggerListMsg();
  void AddTriggerMsg();
  void DelTriggerMsg();
  void TriggerEditMsg();
  void ReactorTimeEditMsg();
  void ReactorStrengthEditMsg();
  void SecretCubeEditMsg();
  void SecretSideEditMsg();
  LRESULT User( WPARAM, LPARAM );
// OWLCVT: Unrecognized DDVT value 1280 for 'User', generic signature used
  void RefreshData();
  void GrayButtons();
DECLARE_RESPONSE_TABLE( TControlDialog );
};
