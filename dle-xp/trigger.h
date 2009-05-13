// Copyright (C) 1997 Bryan Aamot
//------------------------------------------------------------------------
// DIALOG - TTriggerDialog
//------------------------------------------------------------------------
class TTriggerDialog : public TDialog {
  TComboBox *TrigNumberCombo,*TrigTypeCombo;
  TEdit *TrigValueEdit,*TrigTimeEdit,*TrigLinkEdit;
  TCheckBox *TriggerFlags[10];
  TListBox *TrigCubeSideList;
  TButton *TrigCubeSideAdd,*TrigCubeSideDel;
  TEdit *TrigCubeSideEdit;
  TCheckBox *NoMessageCheck,*OneShotCheck;
  TEdit     *TriggerEdit;
public:
  TTriggerDialog(TWindow * AParent, LPSTR name);
  virtual void SetupWindow();
  void AddTriggerMsg();
  void DeleteTriggerMsg();
  void TrigNumberMsg();
  void TrigTypeMsg();
  void TrigValueMsg();
  void TrigTimeMsg();
  void TrigLinkMsg();
  void TriggerFlags0Msg();
  void TriggerFlags1Msg();
  void TriggerFlags2Msg();
  void TriggerFlags3Msg();
  void TriggerFlags4Msg();
  void TriggerFlags5Msg();
  void TriggerFlags6Msg();
  void TriggerFlags7Msg();
  void TriggerFlags8Msg();
  void TriggerFlags9Msg();
  void TriggerCubeSideListMsg();
  void TriggerCubeSideAddMsg();
  void TriggerCubeSideDelMsg();
  void TriggerCubeSideEditMsg();
  void NoMessageCheckMsg();
  void OneShotCheckMsg();

  LRESULT User( WPARAM, LPARAM );
// OWLCVT: Unrecognized DDVT value 1280 for 'User', generic signature used
  void RefreshData();
  void ProcessTriggerFlag(int index);
  void GrayButtons();
//  void ProcessTriggerCube(RTMessage Msg,int index);
//  void ProcessTriggerSide(RTMessage Msg,int index);
DECLARE_RESPONSE_TABLE( TTriggerDialog );
};


