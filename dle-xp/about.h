// Copyright (C) 1997 Bryan Aamot
//************************************************************************
// DIALOG - TAboutDialog
//************************************************************************
class TAboutDialog : public TDialog {
  HDC hDC;
  HBITMAP hBitmapBitmap;
  TButton *AboutButton;
  TStatic *VersionStatic;
  TStatic *WarningStatic;
  TEdit *PasswordEdit;
  int animation_clip;
  int got_button_down;
 public:
  TAboutDialog(TWindow * AParent, LPSTR name);
  virtual ~TAboutDialog();
  virtual void SetupWindow();
  void CmOk();
  void EvDrawItem( UINT, DRAWITEMSTRUCT FAR & );
  void EvTimer( UINT );
  void EvLButtonUp( UINT, TPoint& );
  void EvLButtonDown( UINT, TPoint& );
DECLARE_RESPONSE_TABLE( TAboutDialog );
};

