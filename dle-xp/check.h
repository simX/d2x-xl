// Copyright (C) 1997 Bryan Aamot
//--------------------------------------------------------------------------
// DIALOG - TCheckDialog
//--------------------------------------------------------------------------
class TCheckDialog : public TDialog {
  TListBox *CheckList;
  TStatic *StatusStatic,*WarningsStatic,*ErrorsStatic;
  int error_count,warning_count;
  int sub_errors,sub_warnings;
  int check_state;
public:
  TCheckDialog(TWindow * AParent, LPSTR name);
  virtual void SetupWindow();
  void EvTimer( UINT );
  bool update_stats(char *message,int error,int warning);
  bool check_segments();
  bool check_objects();
  bool check_triggers();
  bool check_walls();
  bool check_misc();
DECLARE_RESPONSE_TABLE( TCheckDialog );
};

