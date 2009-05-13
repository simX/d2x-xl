// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "dlc.h"
#include "mine.h"
#include "global.h"
#include "toolview.h"

// local prototypes
FIX fix_exp(int x);
int fix_log(FIX x);
void InitRobotData();
int WriteHxmFile(FILE *fp);
int ReadHxmFile(FILE *fp);
int ReadHamFile(char *fname,int type);
void analyze(FILE *file,int data_type,UINT8 *data);
void SetSelItemData(TComboBox *box, int data);
void ReadRobotResource(int robot_number);

// list box tables
int model_num_list[N_D2_ROBOT_TYPES] = {
  0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x0f, 0x11,
  0x13, 0x14, 0x15, 0x16, 0x17, 0x19, 0x1a, 0x1c, 0x1d, 0x1f,
  0x21, 0x23, 0x25, 0x27, 0x28, 0x29, 0x2b, 0x2c, 0x2d, 0x2e,
  0x2f, 0x31, 0x32, 0x33, 0x34, 0x36, 0x37, 0x38, 0x3a, 0x3c,
  0x3e, 0x40, 0x41, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
  0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x50, 0x52, 0x53, 0x55, 0x56,
  0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c
};

#define MAX_EXP2_VCLIP_NUM_TABLE 4
UINT8 exp2_vclip_num_table [MAX_EXP2_VCLIP_NUM_TABLE] = {
  0x00, 0x03, 0x07, 0x3c
};

#define MAX_WEAPON_TYPE_TABLE 30
UINT8 weapon_type_table[MAX_WEAPON_TYPE_TABLE] = {
	0x00, 0x05, 0x06, 0x0a, 0x0b, 0x0e, 0x11, 0x14, 0x15, 0x16,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
	0x2e, 0x30, 0x32, 0x34, 0x35, 0x37, 0x39, 0x3a, 0x3c, 0x3d
};

#define MAX_BEHAVIOR_TABLE 7
UINT8 behavior_table[MAX_BEHAVIOR_TABLE] = {
  AIB_STILL, AIB_NORMAL, AIB_HIDE, AIB_RUN_FROM, AIB_FOLLOW_PATH, AIB_STATION, AIB_SNIPE
};


//==========================================================================
// TRobotDialog - SetupWindow
//==========================================================================
void TRobotDialog::SetupWindow() {
  int i,index;
  TDialog::SetupWindow();

  // setup list boxes
  for (i=0;i<ROBOT_IDS2;i++) {
    LoadString(GetModule()->GetInstance(),ROBOT_STRING_TABLE+i,string,sizeof(string));
    robot_type_box->AddString(string);
  }

  for (i=0;i<MAX_EXP2_VCLIP_NUM_TABLE;i++) {
    sprintf(message,"explosion %d",i);
    index = exp2_vclip_num_box->AddString(message);
    exp2_vclip_num_box->SetItemData(index,exp2_vclip_num_table[i]);
  }

  index = weapon_type2_box->AddString("(none)");
  weapon_type2_box->SetItemData(index,-1);
  for (i=0;i<MAX_WEAPON_TYPE_TABLE;i++) {
    LoadString(hInst,7000 + i,message,sizeof(message));
    index = weapon_type_box->AddString(message);
    weapon_type_box->SetItemData(index,weapon_type_table[i]);
    index = weapon_type2_box->AddString(message);
    weapon_type2_box->SetItemData(index,weapon_type_table[i]);
  }
  for (i=0;i<((file_type == RDL_FILE) ? MAX_D1_AI_OPTIONS:MAX_D2_AI_OPTIONS);i++) {
    index = behavior_box->AddString(ai_options[i]);
    behavior_box->SetItemData(index,behavior_table[i]);
  }
/*
  for (i=0;i<MAX_EXP2_SOUND_NUM_TABLE;i++) {
    sprintf(message,"sound %d",i);
    index = exp2_sound_box->AddString(message);
    exp2_sound_box->SetItemData(index,exp2_sound_num_table[i]);
  }

  for (i=0;i<MAX_SEE_SOUND_TABLE;i++) {
    sprintf(message,"sound %d",i+1);
    index = see_sound_box->AddString(message);
    see_sound_box->SetItemData(index,see_sound_table[i]);
  }
  for (i=0;i<MAX_ATTACK_SOUND_TABLE;i++) {
    sprintf(message,"sound %d",i+1);
    index = attack_sound_box->AddString(message);
    attack_sound_box->SetItemData(index,attack_sound_table[i]);
  }
  for (i=0;i<MAX_CLAW_SOUND_TABLE;i++) {
    sprintf(message,"sound %d",i+1);
    index = claw_sound_box->AddString(message);
    claw_sound_box->SetItemData(index,claw_sound_table[i]);
  }

  for (i=0;i<MAX_DEATHROLL_SOUND_TABLE;i++) {
    sprintf(message,"sound %d",i+1);
    index = deathroll_sound_box->AddString(message);
    deathroll_sound_box->SetItemData(index,deathroll_sound_table[i]);
  }
*/
  index = contains_type_box->AddString("Robot");
  contains_type_box->SetItemData(index,OBJ_ROBOT);
  index = contains_type_box->AddString("Powerup");
  contains_type_box->SetItemData(index,OBJ_POWERUP);

  // set scroll bar ranges
  player_skill_bar->SetRange(0,4);
  player_skill_factor = 1;

  score_value_bar->SetRange(0,600); // 0 to 30000 (normal max 15000) 1/22/97
  score_value_factor = 50;
  strength_bar->SetRange(13,20); // 5.0 to 7000.0 (use exponential)
  mass_bar->SetRange(10,20);     // 0.25 to 10000.0 (use exponential)
  drag_bar->SetRange(1,13);     // 0.01 to 0.13 (12 steps)
  drag_factor = F1_0/100;
  energy_blobs_bar->SetRange(0,100);
  energy_blobs_factor = 1;
  lightcast_bar->SetRange(0,10);
  lightcast_factor = 1;
  glow_bar->SetRange(0,12);
  glow_factor = 1;
  aim_bar->SetRange(2,4);
  aim_factor = 0x40;
  field_of_view_bar->SetRange(-10,10); // -1.0 to 1.0
  field_of_view_factor = F1_0/10;
  firing_wait_bar->SetRange(1,35); // 0.2 to 7.0
  firing_wait_factor = F1_0/5;
  firing_wait2_bar->SetRange(1,35); // 0.2 to 7.0
  firing_wait2_factor = F1_0/5;
  badass_bar->SetRange(0,100); // 0 to 100 (normal max=20) 1/22/97
  badass_factor = 1;
  death_roll_bar->SetRange(0,10); // 0 to 10 (normal max=3) 1/22/97
  death_roll_factor = 1;
  turn_time_bar->SetRange(0,10);   // 0 to 1.0
  turn_time_factor = F1_0/10;
  max_speed_bar->SetRange(0,140);  // 0 to 140.0
  max_speed_factor = F1_0;
  circle_distance_bar->SetRange(0,60);  // 0 to 60.0
  circle_distance_factor = F1_0;
  rapidfire_count_bar->SetRange(1,18);
  rapidfire_count_factor = 1;
  evade_speed_bar->SetRange(0,6);
  evade_speed_factor = 1;
  contains_prob_bar->SetRange(0,16);
  contains_prob_factor = 1;
  contains_count_bar->SetRange(0,100);
  contains_count_factor = 1;

  // now set robot number and update the box
  if (Objects ()[current->object]->type == OBJ_ROBOT &&
      Objects ()[current->object]->id < N_D2_ROBOT_TYPES) {
    robot_type_box->SetSelIndex(Objects ()[current->object]->id);
  } else {
    robot_type_box->SetSelIndex(0);
  }

  // setup sound list boxes
  char sound[100];
  for (i=0;i<196;i++) {
    LoadString(hInst,6000 + i,sound,sizeof(sound));
    int value = (sound[0] - '0')*100 + (sound[1] - '0')*10 + (sound[2] - '0');
    index = exp2_sound_box->AddString(&sound[3]);
    exp2_sound_box->SetItemData(index,value);
    index = see_sound_box->AddString(&sound[3]);
    see_sound_box->SetItemData(index,value);
    index = attack_sound_box->AddString(&sound[3]);
    attack_sound_box->SetItemData(index,value);
    index = claw_sound_box->AddString(&sound[3]);
    claw_sound_box->SetItemData(index,value);
    index = deathroll_sound_box->AddString(&sound[3]);
    deathroll_sound_box->SetItemData(index,value);
  }
  RefreshData();
}

//==========================================================================
// TRobotDialog - Timer Message
//==========================================================================
void TRobotDialog::EvTimer( UINT ) {
//  CloseWindow();
}

//==========================================================================
// TRobotDialog - DrawItem  Message
//==========================================================================
void TRobotDialog::EvDrawItem( UINT, DRAWITEMSTRUCT FAR & ) {
}


//------------------------------------------------------------------------
// TRobotDialog - GetInput
//------------------------------------------------------------------------
void TRobotDialog::GetInput() {
  ROBOT_INFO *RInfo;
  int i,j,index;

  // get selection
  i = robot_type_box->GetSelIndex();
  if (i<0 || i>= ROBOT_IDS2) {
    return;
  }
  RInfo = &Robot_info[i];

  // use pad[0] to mark data as modified.
  // this will get cleared when a level is loaded
  RInfo->pad[0] |= 1;

  j = player_skill_bar->GetPosition();

  // read scroll bars
  RInfo->score_value        = (INT16)(score_value_bar->GetPosition() * score_value_factor);
  RInfo->strength           = fix_exp(strength_bar->GetPosition());
  RInfo->mass               = fix_exp(mass_bar->GetPosition());
  RInfo->drag               = drag_bar->GetPosition() * drag_factor;
  RInfo->energy_blobs       = (INT16)(energy_blobs_bar->GetPosition() * energy_blobs_factor);
  RInfo->lightcast          = (INT16)(lightcast_bar->GetPosition() * lightcast_factor);
  RInfo->glow               = (UINT8)(glow_bar->GetPosition() * glow_factor);
  RInfo->aim                = (UINT8)((aim_bar->GetPosition() * aim_factor) - 1);
  RInfo->field_of_view[j]   = field_of_view_bar->GetPosition() * field_of_view_factor;
  RInfo->firing_wait[j]     = firing_wait_bar->GetPosition() * firing_wait_factor;
  RInfo->firing_wait2[j]    = firing_wait2_bar->GetPosition() * firing_wait2_factor;
  RInfo->badass             = (INT8)(badass_bar->GetPosition() * badass_factor);
  RInfo->death_roll         = (INT8)(death_roll_bar->GetPosition() * death_roll_factor);
  RInfo->turn_time[j]       = turn_time_bar->GetPosition() * turn_time_factor;
  RInfo->max_speed[j]       = max_speed_bar->GetPosition() * max_speed_factor;
  RInfo->circle_distance[j] = circle_distance_bar->GetPosition() * circle_distance_factor;
  RInfo->rapidfire_count[j] = (INT8)(rapidfire_count_bar->GetPosition() * rapidfire_count_factor);
  RInfo->evade_speed[j]     = (INT8)(evade_speed_bar->GetPosition() * evade_speed_factor);
  RInfo->contains_prob      = (INT8)(contains_prob_bar->GetPosition() * contains_prob_factor);
  RInfo->contains_count     = (INT8)(contains_count_bar->GetPosition() * contains_count_factor);

  // get check boxes
  RInfo->kamikaze    =(kamikaze_check->GetCheck()    == BF_CHECKED) ? 1:0;
  RInfo->companion   =(companion_check->GetCheck()   == BF_CHECKED) ? 1:0;
  RInfo->thief       =(thief_check->GetCheck()       == BF_CHECKED) ? 1:0;
  RInfo->smart_blobs =(smart_blobs_check->GetCheck() == BF_CHECKED) ? 1:0;
  RInfo->pursuit     =(pursuit_check->GetCheck()     == BF_CHECKED) ? 1:0;
  RInfo->attack_type =(attack_type_check->GetCheck() == BF_CHECKED) ? 1:0;
  RInfo->energy_drain=(energy_drain_check->GetCheck()== BF_CHECKED) ? 1:0;
//  RInfo->death_roll  =(death_roll_check->GetCheck()  == BF_CHECKED) ? 1:0;
//  RInfo->badass      =(badass_check->GetCheck()      == BF_CHECKED) ? 1:0;
  RInfo->boss_flag   =(boss_flag_check->GetCheck()   == BF_CHECKED) ? 1:0;
  RInfo->lighting    =(lighting_check->GetCheck()    == BF_CHECKED) ? 1:0;
  RInfo->cloak_type  =(cloak_type_check->GetCheck()  == BF_CHECKED) ? 1:0;

  // get list box changes
  index = weapon_type_box->GetSelIndex();
  if (index >= 0)
    RInfo->weapon_type = (UINT8)weapon_type_box->GetItemData(index);
  index = weapon_type2_box->GetSelIndex();
  if (index >= 0)
    RInfo->weapon_type2 = (UINT8)weapon_type2_box->GetItemData(index);
  index = exp2_sound_box->GetSelIndex();
  if (index >= 0)
    RInfo->exp2_sound_num = (UINT8) exp2_sound_box->GetItemData(index);
  index = see_sound_box->GetSelIndex();
  if (index >= 0)
    RInfo->see_sound  = (UINT8)see_sound_box->GetItemData(index);
  index = attack_sound_box->GetSelIndex();
  if (index >= 0)
    RInfo->attack_sound = (UINT8)attack_sound_box->GetItemData(index);
  index = claw_sound_box->GetSelIndex();
  if (index >= 0)
    RInfo->claw_sound = (UINT8)claw_sound_box->GetItemData(index);
  index = deathroll_sound_box->GetSelIndex();
  if (index >= 0)
    RInfo->deathroll_sound = (UINT8)deathroll_sound_box->GetItemData(index);
  index = behavior_box->GetSelIndex();
  if (index >= 0)
    RInfo->behavior = (UINT8)behavior_box->GetItemData(index);
  index = exp2_vclip_num_box->GetSelIndex();
  if (index >= 0)
    RInfo->exp2_vclip_num = (UINT8)exp2_vclip_num_box->GetItemData(index);
  index = contains_type_box->GetSelIndex();
  if (index >= 0)
    RInfo->contains_type = (UINT8)contains_type_box->GetItemData(index);
  index = contains_id_box->GetSelIndex();
  if (index >= 0)
    RInfo->contains_id = (UINT8)contains_id_box->GetItemData(index);

  // update scroll bar text
//  UpdateScrollText();
  RefreshData();
}



//------------------------------------------------------------------------
// TRobotDialog - RefreshData
//------------------------------------------------------------------------
void TRobotDialog::RefreshData() {
  int i,j,k,index;
  ROBOT_INFO *RInfo;

  // get selection
  i = robot_type_box->GetSelIndex();
  if (i<0 || i >= ROBOT_IDS2) {
    i = 0;
  }
  RInfo = &Robot_info[i];
  if (RInfo->pad[0]) {
    modified_text->SetText("Modified");
  } else {
    modified_text->SetText("Default");
  }

  j = player_skill_bar->GetPosition();
  player_skill_text->SetText(szSkills[j]);

  contains_id_box->ClearList();
  switch(RInfo->contains_type) {
    case OBJ_ROBOT    : /* an evil enemy */
	  for (k=0;k<ROBOT_IDS2;k++) {
	LoadString(GetModule()->GetInstance(),ROBOT_STRING_TABLE+k,string,sizeof(string));
	index = contains_id_box->AddString(string);
	contains_id_box->SetItemData(index,k);
      }
      break;
    case OBJ_POWERUP  : // a powerup you can pick up
      for (k=0;k<MAX_POWERUP_IDS;k++) {
	LoadString(GetModule()->GetInstance(),POWERUP_STRING_TABLE+k,string,sizeof(string));
	index = contains_id_box->AddString(string);
	contains_id_box->SetItemData(index,k);
      }
      break;
  }
  contains_id_box->SetSelIndex((int)RInfo->contains_id);

  // update list boxes
  SetSelItemData(weapon_type_box,    (int)RInfo->weapon_type);
  SetSelItemData(weapon_type2_box,   (int)RInfo->weapon_type2);
  SetSelItemData(exp2_sound_box,     (int)RInfo->exp2_sound_num);
  SetSelItemData(see_sound_box,      (int)RInfo->see_sound);
  SetSelItemData(attack_sound_box,   (int)RInfo->attack_sound);
  SetSelItemData(claw_sound_box,     (int)RInfo->claw_sound);
  SetSelItemData(deathroll_sound_box,(int)RInfo->deathroll_sound);
  SetSelItemData(behavior_box,       (int)RInfo->behavior);
  SetSelItemData(exp2_vclip_num_box, (int)RInfo->exp2_vclip_num);
  SetSelItemData(contains_type_box,  (int)RInfo->contains_type);

  // update check boxes
  kamikaze_check->SetCheck((RInfo->kamikaze)?BF_CHECKED:BF_UNCHECKED);
  companion_check->SetCheck((RInfo->companion)?BF_CHECKED:BF_UNCHECKED);
  thief_check->SetCheck((RInfo->thief)?BF_CHECKED:BF_UNCHECKED);
  smart_blobs_check->SetCheck((RInfo->smart_blobs)?BF_CHECKED:BF_UNCHECKED);
  pursuit_check->SetCheck((RInfo->pursuit)?BF_CHECKED:BF_UNCHECKED);
  attack_type_check->SetCheck((RInfo->attack_type)?BF_CHECKED:BF_UNCHECKED);
  energy_drain_check->SetCheck((RInfo->energy_drain)?BF_CHECKED:BF_UNCHECKED);
//  death_roll_check->SetCheck((RInfo->death_roll)?BF_CHECKED:BF_UNCHECKED);
//  badass_check->SetCheck((RInfo->badass)?BF_CHECKED:BF_UNCHECKED);
  boss_flag_check->SetCheck((RInfo->boss_flag)?BF_CHECKED:BF_UNCHECKED);
  lighting_check->SetCheck((RInfo->lighting)?BF_CHECKED:BF_UNCHECKED);
  cloak_type_check->SetCheck((RInfo->cloak_type)?BF_CHECKED:BF_UNCHECKED);

  // update scroll bars
  score_value_bar->SetPosition((int)(RInfo->score_value/score_value_factor));
  strength_bar->SetPosition(fix_log(RInfo->strength));
  mass_bar->SetPosition(fix_log(RInfo->mass));
  drag_bar->SetPosition((int)(RInfo->drag/drag_factor));
  energy_blobs_bar->SetPosition((int)(RInfo->energy_blobs/energy_blobs_factor));
  lightcast_bar->SetPosition((int)(RInfo->lightcast/lightcast_factor));
  glow_bar->SetPosition((int)(RInfo->glow/glow_factor));
  aim_bar->SetPosition((int)((RInfo->aim+1)/aim_factor));
  field_of_view_bar->SetPosition((int)(RInfo->field_of_view[j]/field_of_view_factor));
  firing_wait_bar->SetPosition((int)(RInfo->firing_wait[j]/firing_wait_factor));
  firing_wait2_bar->SetPosition((int)(RInfo->firing_wait2[j]/firing_wait2_factor));
  badass_bar->SetPosition((int)(RInfo->badass/badass_factor));
  death_roll_bar->SetPosition((int)(RInfo->death_roll/death_roll_factor));
  turn_time_bar->SetPosition((int)(RInfo->turn_time[j]/turn_time_factor));
  max_speed_bar->SetPosition((int)(RInfo->max_speed[j]/max_speed_factor));
  circle_distance_bar->SetPosition((int)(RInfo->circle_distance[j]/circle_distance_factor));
  rapidfire_count_bar->SetPosition((int)(RInfo->rapidfire_count[j]/rapidfire_count_factor));
  evade_speed_bar->SetPosition((int)(RInfo->evade_speed[j]/evade_speed_factor));
  contains_prob_bar->SetPosition((int)(RInfo->contains_prob/contains_prob_factor));
  contains_count_bar->SetPosition((int)(RInfo->contains_count/contains_prob_factor));

  UpdateScrollText();
}


//------------------------------------------------------------------------
//------------------------------------------------------------------------
void TRobotDialog::UpdateScrollText() {
  score_value_text->SetText(itoa((int)score_value_factor * score_value_bar->GetPosition(),message,10));
  strength_text->SetText(itoa(strength_bar->GetPosition(),message,10));
  mass_text->SetText(itoa(mass_bar->GetPosition(),message,10));
  drag_text->SetText(itoa(drag_bar->GetPosition(),message,10));
  energy_blobs_text->SetText(itoa(energy_blobs_bar->GetPosition(),message,10));
  lightcast_text->SetText(itoa(lightcast_bar->GetPosition(),message,10));
  glow_text->SetText(itoa(glow_bar->GetPosition(),message,10));
  aim_text->SetText(itoa(aim_bar->GetPosition(),message,10));
  field_of_view_text->SetText(itoa(field_of_view_bar->GetPosition(),message,10));
  firing_wait_text->SetText(itoa(firing_wait_bar->GetPosition(),message,10));
  firing_wait2_text->SetText(itoa(firing_wait2_bar->GetPosition(),message,10));
  badass_text->SetText(itoa(badass_bar->GetPosition(),message,10));
  death_roll_text->SetText(itoa(death_roll_bar->GetPosition(),message,10));
  turn_time_text->SetText(itoa(turn_time_bar->GetPosition(),message,10));
  max_speed_text->SetText(itoa(max_speed_bar->GetPosition(),message,10));
  circle_distance_text->SetText(itoa(circle_distance_bar->GetPosition(),message,10));
  rapidfire_count_text->SetText(itoa(rapidfire_count_bar->GetPosition(),message,10));
  evade_speed_text->SetText(itoa(evade_speed_bar->GetPosition(),message,10));
  contains_prob_text->SetText(itoa(contains_prob_bar->GetPosition(),message,10));
  contains_count_text->SetText(itoa(contains_count_bar->GetPosition(),message,10));
}

//------------------------------------------------------------------------
// fix_log()
//------------------------------------------------------------------------
int fix_log(FIX x) {
  int retval;
  double value;
  if (x>=1) {
    value = log((double)x);
    retval = (int)(value + 0.5); // round (assume value is positive)
  } else {
    retval = 0;
  }
  return retval;
}

//------------------------------------------------------------------------
// fix_exp()
//------------------------------------------------------------------------
FIX fix_exp(int x) {
  FIX retval;
  double value;
  if (x>=0 && x<=21) {
    value = exp((double)x);
    retval = (FIX)(value + 0.5); // round (assume value is positive)
  } else {
    retval = 1;
  }
  return retval;
}

//------------------------------------------------------------------------
// SetSelItemData() - Finds data item in list box and selects item
//------------------------------------------------------------------------
void SetSelItemData(TComboBox *box, int data) {
  int count = box->GetCount();
  for (int i=0;i<count;i++) {
    if (box->GetItemData(i) == data) {
      box->SetSelIndex(i);
      break;
    }
  }
  if (i==count) {
    box->SetSelIndex(-1); // select none
  }
}



//------------------------------------------------------------------------
// TRobotDialog - robot_number_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::robot_type_box_msg() {
  RefreshData();
}

//------------------------------------------------------------------------
// TRobotDialog - restore robot message
//------------------------------------------------------------------------
void TRobotDialog::restore_robot_msg() {
  if (QueryMsg("Are you sure you want to change this robot's\n"
	       "attributes back to their defaults?") == IDOK) {
    int robot_number = robot_type_box->GetSelIndex();
	if (robot_number<0 || robot_number>= ROBOT_IDS2) {
      return;
    }
    ReadRobotResource(robot_number);
    Robot_info[robot_number].pad[0] = 0;
    RefreshData();
  }
}

//------------------------------------------------------------------------
// TRobotDialog - weapon_type_box_msg()
//------------------------------------------------------------------------
void TRobotDialog::weapon_type_box_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - weapon_type2_box_msg()
//------------------------------------------------------------------------
void TRobotDialog::weapon_type2_box_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - lighting_check_msg()
//------------------------------------------------------------------------
void TRobotDialog::lighting_check_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - cloak_type_check_msg()
//------------------------------------------------------------------------
void TRobotDialog::cloak_type_check_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - score_value_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::score_value_bar_msg(UINT) {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - strength_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::strength_bar_msg(UINT) {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - mass_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::mass_bar_msg(UINT) {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - drag_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::drag_bar_msg(UINT) {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - energy_blobs_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::energy_blobs_bar_msg(UINT) {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - lightcast_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::lightcast_bar_msg(UINT) {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - glow_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::glow_bar_msg(UINT) {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - aim_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::aim_bar_msg(UINT) {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - player_skill_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::player_skill_bar_msg(UINT) {
  RefreshData();
}

//------------------------------------------------------------------------
// TRobotDialog - field_of_view_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::field_of_view_bar_msg(UINT) {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - firing_wait_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::firing_wait_bar_msg(UINT) {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - firing_wait2_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::firing_wait2_bar_msg(UINT) {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - badass_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::badass_bar_msg(UINT) {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - death_roll_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::death_roll_bar_msg(UINT) {
  GetInput();
}



//------------------------------------------------------------------------
// TRobotDialog - turn_time_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::turn_time_bar_msg(UINT) {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - max_speed_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::max_speed_bar_msg(UINT) {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - rapidfire_count_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::rapidfire_count_bar_msg(UINT) {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - evade_speed_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::evade_speed_bar_msg(UINT) {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - circle_distance_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::circle_distance_bar_msg(UINT) {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - exp2_sound_box_msg()
//------------------------------------------------------------------------
void TRobotDialog::exp2_sound_box_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - see_sound_box_msg()
//------------------------------------------------------------------------
void TRobotDialog::see_sound_box_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - attack_sound_box_msg()
//------------------------------------------------------------------------
void TRobotDialog::attack_sound_box_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - claw_sound_box_msg()
//------------------------------------------------------------------------
void TRobotDialog::claw_sound_box_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - deathroll_sound_box_msg()
//------------------------------------------------------------------------
void TRobotDialog::deathroll_sound_box_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - behavior_box_msg()
//------------------------------------------------------------------------
void TRobotDialog::behavior_box_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - kamikaze_check_msg()
//------------------------------------------------------------------------
void TRobotDialog::kamikaze_check_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - companion_check_msg()
//------------------------------------------------------------------------
void TRobotDialog::companion_check_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - thief_check_msg()
//------------------------------------------------------------------------
void TRobotDialog::thief_check_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - smart_blobs_check_msg()
//------------------------------------------------------------------------
void TRobotDialog::smart_blobs_check_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - pursuit_check_msg()
//------------------------------------------------------------------------
void TRobotDialog::pursuit_check_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - attack_type_check_msg()
//------------------------------------------------------------------------
void TRobotDialog::attack_type_check_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - energy_drain_check_msg()
//------------------------------------------------------------------------
void TRobotDialog::energy_drain_check_msg() {
  GetInput();
}

/*
//------------------------------------------------------------------------
// TRobotDialog - death_roll_check_msg()
//------------------------------------------------------------------------
void TRobotDialog::death_roll_check_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - badass_check_msg()
//------------------------------------------------------------------------
void TRobotDialog::badass_check_msg() {
  GetInput();
}
*/

//------------------------------------------------------------------------
// TRobotDialog - boss_flag_check_msg()
//------------------------------------------------------------------------
void TRobotDialog::boss_flag_check_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - exp2_vclip_num_box_msg()
//------------------------------------------------------------------------
void TRobotDialog::exp2_vclip_num_box_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - contains_count_edit_msg()
//------------------------------------------------------------------------
void TRobotDialog::contains_count_bar_msg(UINT) {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - contains_type_box_msg()
//------------------------------------------------------------------------
void TRobotDialog::contains_type_box_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - contains_id_box_msg()
//------------------------------------------------------------------------
void TRobotDialog::contains_id_box_msg() {
  GetInput();
}

//------------------------------------------------------------------------
// TRobotDialog - contains_prob_bar_msg()
//------------------------------------------------------------------------
void TRobotDialog::contains_prob_bar_msg(UINT) {
  GetInput();
}


//------------------------------------------------------------------------
// ReadHamFile()
//
// Actions
//   1)	Reads all robot data from a HAM file
//   2) Memory will be allocated for polymodel data.  It is the
//	responsibility of the caller to free all non-null polymodel
//	data pointer.
//   3) If polymodel data is allocated aready, it will be freed and
//	reallocated.
//
// Parameters
//	fname - handle of file to read
//      type - type of file (0=DESCENT2.HAM, 1=extended robot HAM file)
//
// Globals used
//
//  N_robot_types
//  Robot_info[MAX_ROBOT_TYPES]
//
//  N_robot_joints
//  Robot_joints[MAX_ROBOT_JOINTS]
//
//  N_polygon_models
//  Polygon_models[MAX_POLYGON_MODELS]
//  Dying_modelnums[N_D2_POLYGON_MODELS]
//  Dead_modelnums[N_D2_POLYGON_MODELS]
//
//  N_object_bitmaps
//  ObjBitmaps[MAX_OBJ_BITMAPS]
//  ObjBitmapPtrs[MAX_OBJ_BITMAPS]
//
//
// Assumptions
//   1) Memory was allocated for globals (except polymodel data)
//------------------------------------------------------------------------
int ReadHamFile(char *fname,int type) {
  FILE *fp;
  INT16 t,t0;
  UINT32 id;
  POLYMODEL pm;

  pm.n_models = 0;
  fp = fopen(fname,"rb");
  if (!fp) {
    sprintf(message,"Unable to open ham file (%d)",fname);
    ErrorMsg(message);
    goto abort;
  }

  // The extended HAM only contains part of the normal HAM file.
  // Therefore, we have to skipp some items if we are reading
  // a normal HAM cause we are only interested in reading
  // the information which is found in the extended ham
  // (the robot information)
  if (type == NORMAL_HAM) {
    id = read_INT32(fp); // "HAM!"
    if (id != 0x214d4148L) {
      sprintf(message,"Not a ham file (%d)",fname);
      ErrorMsg(message);
      goto abort;
    }
    read_INT32(fp); // version (0x00000007)
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof(BITMAP_INDEX)*t,SEEK_CUR);
    fseek(fp,sizeof(TMAP_INFO)*t,SEEK_CUR);
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof(UINT8)*t,SEEK_CUR);
    fseek(fp,sizeof(UINT8)*t,SEEK_CUR);
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof(VCLIP)*t,SEEK_CUR);
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof(ECLIP)*t,SEEK_CUR);
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof(WCLIP)*t,SEEK_CUR);
  }

  // read robot information
  //------------------------
  t = (INT16) read_INT32(fp);
  t0 = (type == NORMAL_HAM) ? 0: N_D2_ROBOT_TYPES;
  N_robot_types = t0 + t;
  if (N_robot_types > MAX_ROBOT_TYPES) {
    sprintf(message,"Too many robots (%d) in <%s>.  Max is %d.",t,fname,MAX_ROBOT_TYPES-N_D2_ROBOT_TYPES);
    ErrorMsg(message);
    goto abort;
  }
  fread(&Robot_info[t0], sizeof(ROBOT_INFO), t, fp );

  // skip joints weapons, and powerups
  //----------------------------------
  t = (INT16) read_INT32(fp);
  fseek(fp,sizeof(JOINTPOS)*t,SEEK_CUR);
  if (type == NORMAL_HAM) {
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof(WEAPON_INFO)*t,SEEK_CUR);
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof(POWERUP_TYPE_INFO)*t,SEEK_CUR);
  }

  // read poly model data and write it to a file
  //---------------------------------------------
  t = (INT16) read_INT32(fp);
  if (t > MAX_POLYGON_MODELS) {
    sprintf(message,"Too many polygon models (%d) in <%s>.  Max is %d.",t,fname,MAX_POLYGON_MODELS-N_D2_POLYGON_MODELS);
    ErrorMsg(message);
    goto abort;
  }

  INT16 i;
  FILE *file;
  file = fopen("d:\\bc\\dlc2data\\poly.dat","wt");
  if (file) {
    for (i=0; i<t; i++ ) {
      fread(&pm, sizeof(POLYMODEL), 1, fp );
      fprintf(file,"n_models        = %ld\n",pm.n_models);
      fprintf(file,"model_data_size = %ld\n",pm.model_data_size);
      for (int j=0;j<pm.n_models;j++) {
	fprintf(file,"submodel_ptrs[%d]    = %#08lx\n",j,pm.submodel_ptrs[j]);
	fprintf(file,"submodel_offsets[%d] = %#08lx %#08lx %#08lx\n",j,
		pm.submodel_offsets[j].x,pm.submodel_offsets[j].y,pm.submodel_offsets[j].z);
	fprintf(file,"submodel_norms[%d]   = %#08lx %#08lx %#08lx\n",j,
		pm.submodel_norms[j].x,pm.submodel_norms[j].y,pm.submodel_norms[j].z);
	fprintf(file,"submodel_pnts[%d]    = %#08lx %#08lx %#08lx\n",j,
		pm.submodel_pnts[j].x,pm.submodel_pnts[j].y,pm.submodel_pnts[j].z);
	fprintf(file,"submodel_rads[%d]    = %#08lx\n",j,pm.submodel_rads[j]);
	fprintf(file,"submodel_parents[%d] = %d\n",j,pm.submodel_parents[j]);
	fprintf(file,"submodel_mins[%d]    = %#08lx %#08lx %#08lx\n",j,
		pm.submodel_mins[j].x,pm.submodel_mins[j].y,pm.submodel_mins[j].z);
	fprintf(file,"submodel_maxs[%d]    = %#08lx %#08lx %#08lx\n",j,
		pm.submodel_maxs[j].x,pm.submodel_maxs[j].y,pm.submodel_maxs[j].z);
      }
      fprintf(file,"mins            = %#08lx %#08lx %#08lx\n",pm.mins.x,pm.mins.y,pm.mins.z);
      fprintf(file,"maxs            = %#08lx %#08lx %#08lx\n",pm.maxs.x,pm.maxs.y,pm.maxs.z);
      fprintf(file,"rad             = %ld\n",pm.rad);
      fprintf(file,"n_textures      = %d\n",pm.n_textures);
      fprintf(file,"first_texture   = %d\n",pm.first_texture);
      fprintf(file,"simpler_model   = %d\n\n",pm.simpler_model);
    }
    fclose(file);
  }

#if ALLOCATE_POLYMODELS
  // read joint information
  //-----------------------
  t = (INT16) read_INT32(fp);
  t0 = (type == NORMAL_HAM) ? 0: N_D2_ROBOT_JOINTS;
  N_robot_joints = t0 + t;
  if (N_robot_joints > MAX_ROBOT_JOINTS) {
    sprintf(message,"Too many robot joints (%d) in <%s>.  Max is %d.",t,fname,MAX_ROBOT_JOINTS-N_D2_ROBOT_JOINTS);
    ErrorMsg(message);
    goto abort;
  }
  fread( &Robot_joints[t0], sizeof(JOINTPOS), t, fp );

  // skip weapon and powerup data
  //-----------------------------
  if (type == NORMAL_HAM) {
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof(WEAPON_INFO)*t,SEEK_CUR);
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof(POWERUP_TYPE_INFO)*t,SEEK_CUR);
  }

  // read poly model data
  //---------------------
  t = (INT16) read_INT32(fp);
  t0 = (type == NORMAL_HAM) ? 0: N_D2_POLYGON_MODELS;
  N_polygon_models = t0 + t;
  if (N_polygon_models > MAX_POLYGON_MODELS) {
    sprintf(message,"Too many polygon models (%d) in <%s>.  Max is %d.",t,fname,MAX_POLYGON_MODELS-N_D2_POLYGON_MODELS);
    ErrorMsg(message);
    goto abort;
  }

  INT16 i;

  for (i=t0; i<t0+t; i++ ) {
    // free poly data memory if already allocated
    if (Polygon_models[i]->model_data != NULL ) {
      free((void *)Polygon_models[i]->model_data);
    }
    fread(Polygon_models[i], sizeof(POLYMODEL), 1, fp );
    fread(&Polygon_model, sizeof(POLYMODEL), 1, fp );
  }
  for (i=t0; i<t0+t; i++ ) {
    Polygon_models[i]->model_data = (UINT8 *) malloc((int)Polygon_models[i]->model_data_size);

    if (Polygon_models[i]->model_data == NULL ) {
      ErrorMsg("Could not allocate memory for polymodel data");
      goto abort;
    }
    fread( Polygon_models[i]->model_data, sizeof(UINT8), (INT16)Polygon_models[i]->model_data_size, fp );
//    g3_init_polygon_model(Polygon_models[i].model_data);
  }

  // extended hog writes over normal hogs dying models instead of adding new ones
  fread( &Dying_modelnums[t0], sizeof(INT32), t, fp );
  fread( &Dead_modelnums[t0], sizeof(INT32), t, fp );

  // skip gague data
  //----------------
  if (type == NORMAL_HAM) {
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof(BITMAP_INDEX)*t,SEEK_CUR); // lores gague
    fseek(fp,sizeof(BITMAP_INDEX)*t,SEEK_CUR); // hires gague
  }

  // read object bitmap data
  //------------------------
  // NOTE: this overwrites D2 object bitmap indices instead of
  // adding more bitmap texture indicies.  I believe that D2
  // writes all 600 indicies even though it doesn't use all
  // of them.
  //----------------------------------------------------------
  t = (INT16) read_INT32(fp);
  t0 = (type == NORMAL_HAM) ? 0: N_D2_OBJBITMAPS;
  if (type == NORMAL_HAM) {
    N_object_bitmaps  = t0 + t;  // only update this if we are reading Descent2.ham file
  }
  if (t+t0 > MAX_OBJ_BITMAPS) {
    sprintf(message,"Too many object bitmaps (%d) in <%s>.  Max is %d.",t,fname,MAX_OBJ_BITMAPS-N_D2_OBJBITMAPS);
    ErrorMsg(message);
    goto abort;
  }
  fread( &ObjBitmaps[t0], sizeof(BITMAP_INDEX), t, fp );

  if (type == EXTENDED_HAM) {
    t = (INT16) read_INT32(fp);
    t0 = (type == NORMAL_HAM) ? 0: N_D2_OBJBITMAPPTRS;
    if (t+t0 > MAX_OBJ_BITMAPS) {
      sprintf(message,"Too many object bitmaps pointer (%d) in <%s>.  Max is %d.",t,fname,MAX_OBJ_BITMAPS-N_D2_OBJBITMAPPTRS);
      ErrorMsg(message);
      goto abort;
    }
  }
  fread(&ObjBitmapPtrs[t0], sizeof(BITMAP_INDEX), t, fp );
#endif

  fclose(fp);
  return 0;
abort:
  if (fp) fclose(fp);
  return 1;
}


//------------------------------------------------------------------------
// ReadHxmFile()
//
// Actions
//   1)	Reads all robot data from a HMX file
//   2) Memory will be allocated for polymodel data.  It is the
//	responsibility of the caller to free all non-null polymodel
//	data pointer.
//   3) If polymodel data is allocated aready, it will be freed and
//	reallocated.
//
// Parameters
//	fp - pointer to file (already offset to the correct position)
//
// Globals used
//
//  N_robot_types
//  Robot_info[MAX_ROBOT_TYPES]
//
//  N_robot_joints
//  Robot_joints[MAX_ROBOT_JOINTS]
//
//  N_polygon_models
//  Polygon_models[MAX_POLYGON_MODELS]
//  Dying_modelnums[N_D2_POLYGON_MODELS]
//  Dead_modelnums[N_D2_POLYGON_MODELS]
//
//  N_object_bitmaps
//  ObjBitmaps[MAX_OBJ_BITMAPS]
//  ObjBitmapPtrs[MAX_OBJ_BITMAPS]
//
//
// Assumptions
//   1) Memory was allocated for globals (except polymodel data)
//------------------------------------------------------------------------
int ReadHxmFile(FILE *fp) {
  UINT16 t,i,j;
  ROBOT_INFO RobotInfo;

  if (!fp) {
    ErrorMsg("Invalid file handle for reading HXM data.");
    goto abort;
  }

  UINT32 id;
  id = read_INT32(fp); // "HMX!"
  if (id != 0x21584d48L) {
    ErrorMsg("Not a HMX file");
    goto abort;
  }
  read_INT32(fp); // version (0x00000001)

  // read robot information
  //------------------------
  t = (UINT16) read_INT32(fp);
  for (j=0;j<t;j++) {
    i = (UINT16)read_INT32(fp);
    if (i>=N_robot_types) {
      sprintf(message,"Robots number (%d) out of range.  Range = [0..%d].",i,N_robot_types-1);
      ErrorMsg(message);
      goto abort;
    }
    fread(&RobotInfo, sizeof(ROBOT_INFO), 1, fp );
    // compare this to existing data
    if (_fmemcmp(&RobotInfo,&Robot_info[i],sizeof(ROBOT_INFO)) != 0) {
      _fmemcpy(&Robot_info[i],&RobotInfo,sizeof(ROBOT_INFO));
      Robot_info[i].pad[0] = 1; // mark as custom
    }
  }

  // skip the rest of the file
  //---------------------------

  fclose(fp);
  return 0;
abort:
  if (fp) fclose(fp);
  return 1;
}

//------------------------------------------------------------------------
// WriteHxmFile()
//
// Actions
//   1)	Writes robot info in HMX file format
//
// Parameters
//	fp - pointer to file (already offset to the correct position)
//
// Globals used
//
//  N_robot_types
//  Robot_info[MAX_ROBOT_TYPES]
//------------------------------------------------------------------------
int WriteHxmFile(FILE *fp) {
  UINT16 t,i;

  if (!fp) {
    ErrorMsg("Invalid file handle for writing HXM data.");
    goto abort;
  }

#if 1
  UINT32 id;
  id = 0x21584d48L;    // "HMX!"
  write_INT32(id,fp);
  write_INT32(1,fp);   // version 1
#endif

  // write robot information
  //------------------------
  t = 0;
  for (i=0;i<N_robot_types;i++) {
    if (Robot_info[i].pad[0]) t++;
  }
  write_INT32(t,fp); // number of robot info structs stored
  for (i=0;i<N_robot_types;i++) {
    if (Robot_info[i].pad[0]) {
      write_INT32((UINT32)i,fp);
      fwrite(&Robot_info[i], sizeof(ROBOT_INFO), 1, fp );
    }
  }

  // write zeros for the rest of the data
  //-------------------------------------
  write_INT32(0,fp);  //number of joints
  write_INT32(0,fp);  //number of polygon models
  write_INT32(0,fp);  //number of objbitmaps
  write_INT32(0,fp);  //number of objbitmaps
  write_INT32(0,fp);  //number of objbitmapptrs

  fclose(fp);
  return 0;
abort:
  if (fp) fclose(fp);
  return 1;
}

//------------------------------------------------------------------------
// InitRobotData()
//------------------------------------------------------------------------
void InitRobotData() {
  ReadRobotResource(-1);
}


//------------------------------------------------------------------------
// ReadRobotResource() - reads robot.hxm from resource data into Robot_info[]
//
// if robot_number == -1, then it reads all robots
//------------------------------------------------------------------------
void ReadRobotResource(int robot_number) {
  UINT16 i,j,t;
  UINT8 *ptr;
  HRSRC hFind = FindResource( hInst,"ROBOT_HXM", RT_RCDATA);
  HGLOBAL hResource = LoadResource( hInst, hFind);
  if (!hResource) {
    ErrorMsg("Could not find robot resource data");
    return;
  }
  ptr = (UINT8 *)LockResource(hResource);
  if (!ptr) {
    ErrorMsg("Could not lock robot resource data");
    return;
  }
  t = (UINT16)(*((UINT32 *)ptr));
  N_robot_types = min(t,MAX_ROBOT_TYPES);
  ptr += sizeof(UINT32);
  for (j=0;j<t;j++) {
    i = (UINT16)(*((UINT32 *)ptr));
    if (i>MAX_ROBOT_TYPES) break;
    ptr += sizeof(UINT32);
    // copy the robot info for one robot, or all robots
    if (j==robot_number || robot_number == -1) {
      _fmemcpy(&Robot_info[i], ptr, sizeof(ROBOT_INFO));
    }
    ptr += sizeof(ROBOT_INFO);
  }
  FreeResource(hResource);
}


DEFINE_RESPONSE_TABLE1( TRobotDialog, TDialog )
  EV_WM_DRAWITEM,
  EV_WM_TIMER,
  EV_COMMAND (IDOK, CmOk),
  EV_LBN_SELCHANGE(12,robot_type_box_msg),
  EV_COMMAND(13,restore_robot_msg),
  EV_LBN_SELCHANGE(110, weapon_type_box_msg),
  EV_LBN_SELCHANGE(111, weapon_type2_box_msg),
  EV_COMMAND(112,lighting_check_msg),
  EV_COMMAND(113,cloak_type_check_msg),
  EV_CHILD_NOTIFY_ALL_CODES(114,score_value_bar_msg),
  EV_CHILD_NOTIFY_ALL_CODES(116,strength_bar_msg),
  EV_CHILD_NOTIFY_ALL_CODES(118,mass_bar_msg),
  EV_CHILD_NOTIFY_ALL_CODES(120,drag_bar_msg),
  EV_CHILD_NOTIFY_ALL_CODES(122,energy_blobs_bar_msg),
  EV_CHILD_NOTIFY_ALL_CODES(124,lightcast_bar_msg),
  EV_CHILD_NOTIFY_ALL_CODES(126,glow_bar_msg),
  EV_CHILD_NOTIFY_ALL_CODES(128,aim_bar_msg),
  EV_CHILD_NOTIFY_ALL_CODES(130,player_skill_bar_msg),
  EV_CHILD_NOTIFY_ALL_CODES(132,field_of_view_bar_msg),
  EV_CHILD_NOTIFY_ALL_CODES(134,firing_wait_bar_msg),
  EV_CHILD_NOTIFY_ALL_CODES(136,turn_time_bar_msg),
  EV_CHILD_NOTIFY_ALL_CODES(138,max_speed_bar_msg),
  EV_CHILD_NOTIFY_ALL_CODES(140,rapidfire_count_bar_msg),
  EV_CHILD_NOTIFY_ALL_CODES(142,evade_speed_bar_msg),
  EV_CHILD_NOTIFY_ALL_CODES(144,circle_distance_bar_msg),
  EV_LBN_SELCHANGE(150, exp2_sound_box_msg),
  EV_LBN_SELCHANGE(151, see_sound_box_msg),
  EV_LBN_SELCHANGE(152, attack_sound_box_msg),
  EV_LBN_SELCHANGE(153, claw_sound_box_msg),
  EV_LBN_SELCHANGE(154, deathroll_sound_box_msg),
  EV_LBN_SELCHANGE(160, behavior_box_msg),
  EV_COMMAND(161,kamikaze_check_msg),
  EV_COMMAND(162,companion_check_msg),
  EV_COMMAND(163,thief_check_msg),
  EV_COMMAND(164,smart_blobs_check_msg),
  EV_COMMAND(165,pursuit_check_msg),
  EV_COMMAND(166,attack_type_check_msg),
  EV_COMMAND(167,energy_drain_check_msg),
//  EV_COMMAND(170,death_roll_check_msg),
//  EV_COMMAND(171,badass_check_msg),
  EV_COMMAND(172,boss_flag_check_msg),
  EV_LBN_SELCHANGE(173, exp2_vclip_num_box_msg),
  EV_CHILD_NOTIFY_ALL_CODES(10,contains_count_bar_msg),
  EV_LBN_SELCHANGE(181, contains_type_box_msg),
  EV_LBN_SELCHANGE(182, contains_id_box_msg),
  EV_CHILD_NOTIFY_ALL_CODES(183, contains_prob_bar_msg),

  EV_CHILD_NOTIFY_ALL_CODES(185,badass_bar_msg),
  EV_CHILD_NOTIFY_ALL_CODES(187,death_roll_bar_msg),
  EV_CHILD_NOTIFY_ALL_CODES(189,firing_wait2_bar_msg),


END_RESPONSE_TABLE;
