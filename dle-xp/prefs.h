// Copyright (C) 1997 Bryan Aamot
//************************************************************************
// DIALOG - TPreferencesDialog
//************************************************************************
class TPreferencesDialog : public TDialog {
  TComboBox *DepthComboBox;
  TEdit *GameDirectory,*GameDirectory2,*LevelsDirectory;
  TEdit *Grid,*MoveRate,*MineName;
  TCheckBox *Rotate1,*Rotate2,*Rotate3,*Rotate4;
  TCheckBox *FilledCheck;
  TCheckBox *PrefCheck[32];
public:
  TPreferencesDialog(TWindow * AParent, LPSTR name);
  virtual void SetupWindow();
  void Ok( UINT );
  void Browse1Msg( UINT );
  void Browse2Msg( UINT );
  void Rotate1Msg( UINT );
  void Rotate2Msg( UINT );
  void Rotate3Msg( UINT );
  void Rotate4Msg( UINT );
DECLARE_RESPONSE_TABLE( TPreferencesDialog );
};
