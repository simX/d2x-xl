// Copyright (C) 1997 Bryan Aamot
//************************************************************************
// DIALOG - TTexturePickerDialog
//************************************************************************
class TTexturePickerDialog : public TDialog {
  int TotalTextures; // calculated in setupwindow
  int *TextureList; // list of non-animated textures
  TGroupBox *TextureGroupBox;
  TCheckBox *TextureCheckBox;
public:
  TTexturePickerDialog(TWindow * AParent, LPSTR name);
  virtual ~TTexturePickerDialog();
  virtual void SetupWindow();
  void EvVScroll(uint scrollCode, uint thumbPos, HWND hWndCtl);
  void EvSize(uint sizeType, TSize& size);
  void EvPaint();
  void EvLButtonDown(uint modKeys, TPoint& point);
  void EvRButtonDown(uint modKeys, TPoint& point);
  LRESULT User( WPARAM, LPARAM );

  void RefreshData();
  int pick_texture(TPoint &point,INT16 &tmap_num);
  int  texture_index(INT16 tmap_num);
  void set_show_texture(UINT8 *show_texture,BOOL show_all_textures);

  DECLARE_RESPONSE_TABLE( TTexturePickerDialog );
};


//************************************************************************
// DIALOG - TTextureDialog
//************************************************************************
class TTextureDialog : public TDialog {
  HBITMAP hTextureBitmap;
  TComboBox *TextureComboBox1,*TextureComboBox2;
  TButton *TextureButton,*LightButton,*EditButton,*PickButton;
  TCheckBox *EnableFrames;
  TStatic *SaveTextureStatic,*NotesStatic;
  TEdit *LightEdit[4];
  int last_texture1,last_texture2,last_mode;
  int save_texture1,save_texture2;
  uvl save_uvls[4];
  int frame[2];

 public:
  TTextureDialog(TWindow * AParent, LPSTR name);
  virtual ~TTextureDialog();
  virtual void SetupWindow();
  void EvDrawItem( UINT, DRAWITEMSTRUCT FAR & );
  LRESULT User( WPARAM, LPARAM );
  void Texture1Msg();
  void Texture2Msg();
  void SaveTextureMsg();
  void PasteTextureMsg();
  void PasteAllTextureMsg();
  void PasteMarkedMsg();
  void PickTextureMsg();
  void RotatationMsg();
  void WallCheckMsg();
  void RobotCheckMsg();
  void Light1Msg();
  void Light2Msg();
  void Light3Msg();
  void Light4Msg();
  void EditMsg();
  void LightButtonMsg();
  void EvTimer(uint);

  void LoadTextureListBoxes();
  void RefreshData();
  int GetTextureID(char *texture_name);
  void set_light_edit(int i);
  void paste_texture(INT16 segnum, INT16 sidenum, INT16 recursion_level);
  void draw_texture(INT16 texture1,INT16 texture2, int x0, int y0);
  DECLARE_RESPONSE_TABLE( TTextureDialog );
};

