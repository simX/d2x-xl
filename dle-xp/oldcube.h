// Copyright (C) 1997 Bryan Aamot
//************************************************************************
// DIALOG - TCubwDialog
//************************************************************************
class TCubeDialog : public TDialog {
  TEdit *LightEdit;
  TComboBox *SpecialCombo,*CubeNumberCombo,*SideNumberCombo,*PointNumberCombo;
  TListBox *UsedRobotsList,*UnusedRobotsList;
  TListBox  *TriggerList;
  TButton   *AddRobot,*DelRobot,*AddTrigger,*DelTrigger,*AddCube,*DelCube;
  TListBox  *TriggeredByList;
  TButton   *OtherCube;
  TCheckBox *EndOfExitTunnel;
  int LastSeg,LastSide;
public:
  TCubeDialog(TWindow * AParent, LPSTR name);
  virtual void SetupWindow();
  void EditPointMsg();
  void AddCubeMsg();
  void DeleteCubeMsg();
  void UsedRobotMsg();
  void UnusedRobotMsg();
  void AddMsg();
  void DelMsg();
  void OtherCubeMsg();
  void EndOfExitTunnelMsg();
  void SpecialMsg();
  void CubeNumberMsg();
  void SideNumberMsg();
  void PointNumberMsg();
  void LightMsg();
  void WallButtonMsg();
  void TriggerButtonMsg();
  LRESULT User( WPARAM, LPARAM );
  void RefreshData();
  void GrayButtons();
DECLARE_RESPONSE_TABLE( TCubeDialog );
};

//************************************************************************
// DIALOG - TPointDialog
//************************************************************************
class TPointDialog : public TDialog {
  TEdit *XEdit,*YEdit,*ZEdit;
public:
  TPointDialog(TWindow * AParent, LPSTR name);
  virtual void SetupWindow();
  void ApplyMsg();
  void RefreshData();
  LRESULT User( WPARAM, LPARAM );
DECLARE_RESPONSE_TABLE( TPointDialog );
};

