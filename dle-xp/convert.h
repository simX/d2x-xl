// Copyright (C) 1997 Bryan Aamot
//************************************************************************
// DIALOG - TConvertDialog
//************************************************************************
class TConvertDialog : public TDialog {
  TComboBox *TextureComboBox1,*TextureComboBox2;
  TCheckBox *Option1Check,*Option2Check,*Option3Check,*Option4Check;
  TButton   *Texture1Button,*Texture2Button;
  INT16 *d2_texture;
  HGLOBAL hd2_texture;
 public:
  TConvertDialog(TWindow * AParent, LPSTR name);
  virtual ~TConvertDialog();
  virtual void SetupWindow();
  void EvDrawItem( UINT, DRAWITEMSTRUCT FAR & );
  void Ok( UINT );
  void Texture1Msg( UINT );
  void Texture2Msg( UINT );
  void Convert();
  void RefreshData();
  int GetTextureID(char *texture_name);
DECLARE_RESPONSE_TABLE( TConvertDialog );
};

