// Copyright (C) 1997 Bryan Aamot
//------------------------------------------------------------------------
// DIALOG - TUVLDialog
//------------------------------------------------------------------------
class TUVLDialog : public TDialog {
  TEdit *X_Edit,*Y_Edit,*AngleEdit;
  TCheckBox *Rotate0,*Rotate90,*Rotate180,*Rotate270;
  TButton *RotateRight,*RotateLeft;
  TButton *ShiftLeft,*ShiftRight,*ShiftDown,*ShiftUp;
//  TButton *ZoomIn,*ZoomOut,*Reset,*Align,*Fit;
  TButton *ResetMarked;
  TCheckBox *ShowTexture,*ShowChildren;
  TScrollBar *HorzScroll,*VertScroll;
  double zoom,angle,uv_point[4];
  POINT apts[4],minpt,maxpt;
  POINT minrect;
  POINT maxrect;
  POINT centerpt;
 public:
  TUVLDialog(TWindow * AParent, LPSTR name);
  virtual ~TUVLDialog();
  virtual void SetupWindow();
  LRESULT User( WPARAM, LPARAM );
// OWLCVT: Unrecognized DDVT value 1280 for 'User', generic signature used
  void EvDrawItem( UINT, DRAWITEMSTRUCT FAR & );
  void X_EditMsg( UINT );
  void Y_EditMsg( UINT );
  void AngleEditMsg( UINT );
  void Rotate0Msg();
  void Rotate90Msg();
  void Rotate180Msg();
  void Rotate270Msg();
  void ZoomInMsg();
  void ZoomOutMsg();
  void ShowTextureMsg();
  void ShowChildrenMsg();
  void ResetMsg();
  void ResetMarkedMsg();
  void AlignMsg();
  void FitMsg();
  void HorzScrollMsg( UINT );
  void VertScrollMsg( UINT );
  void RotateLeftMsg();
  void RotateRightMsg();
  void ShiftLeftMsg();
  void ShiftUpMsg();
  void ShiftRightMsg();
  void ShiftDownMsg();
  void RefreshData();
  void RefreshX();
  void RefreshY();
  void RefreshAngle();
  void RefreshChecks();
  void fill_texture(HDC hdc);
DECLARE_RESPONSE_TABLE( TUVLDialog );
};

