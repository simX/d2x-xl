// Copyright (C) 1997 Bryan Aamot
//************************************************************************
// DIALOG - TCubwDialog
//************************************************************************
class TModifyDialog : public TDialog {
 TButton *MoveToward,*MoveAway;
 TButton *RollPlus,*RollMinus,*PitchPlus,*PitchMinus,*YawPlus,*YawMinus;
 TButton *Enlarge,*Reduce;
 TComboBox *ModeComboBox;
 char mode_string[20];
 public:
  TModifyDialog(TWindow * AParent, LPSTR name);
  virtual void SetupWindow();
  virtual ~TModifyDialog();
  void MoveTowardsMsg();
  void MoveAwayMsg();
  void RollPlusMsg();
  void RollMinusMsg();
  void EnlargeMsg();
  void ReduceMsg();
  void PitchPlusMsg();
  void PitchMinusMsg();
  void YawPlusMsg();
  void YawMinusMsg();
  void ModeMsg( UINT );

  void move_towards();
  void move_away();
  void roll_plus();
  void roll_minus();
  void enlarge();
  void reduce();
  void pitch_plus();
  void pitch_minus();
  void yaw_plus();
  void yaw_minus();

  void refresh();
DECLARE_RESPONSE_TABLE( TModifyDialog );
};

