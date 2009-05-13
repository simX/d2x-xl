// Copyright (C) 1997 Bryan Aamot
//---------------------------------------------------------------------------
// CLASS - MainWindow
//---------------------------------------------------------------------------
class CMainWindow : public CFrameWindow {

private:
	RECT rc;				   // window dimenstions
	HMENU hMainMenu;
	int app_just_started;
	int main_window_focus;
	HWND hButton[NUM_HOT_BUTTONS];
	HBITMAP hButtonBitmapUp[NUM_HOT_BUTTONS];
	HBITMAP hButtonBitmapDown[NUM_HOT_BUTTONS];
	INT16 last_xPoint,last_yPoint;
	INT16 highlight_xPoint,highlight_yPoint;
public:
	TMainWindow(TWindow * AParent, LPSTR ATitle);
	virtual bool CanClose();
	virtual ~TMainWindow();
	virtual void SetupWindow();
	virtual void GetWindowClass(WNDCLASS &AWndClass);
	virtual bool PreProcessMsg(MSG& msg);
	// virtual void Paint(HDC PaintDC, PAINTSTRUCT _FAR & PaintInfo);
	void EvSize( UINT, TSize& );
	void EvKillFocus( HWND );
	void EvSetFocus( HWND );
	void EvPaint();
	void EvTimer( UINT );
	void EvLButtonDown( UINT, TPoint& );
	void EvRButtonDown( UINT, TPoint& );
	void EvMouseMove( UINT, TPoint& );
	void EvLButtonUp( UINT, TPoint& );
	//  virtual void WM_KeyDown(RTMessage Msg) = [WM_FIRST + WM_KEYDOWN];
	LRESULT WMUser( WPARAM, LPARAM );
	// OWLCVT: Unrecognized DDVT value 1281 for 'WMUser', generic signature used
	LRESULT EvCommand(uint id, HWND hWndCtl, uint notifyCode);
	//  LRESULT EvCommand( UINT, HWND, UINT );
	void EvMeasureItem( UINT, MEASUREITEMSTRUCT FAR & );
	void EvDrawItem( UINT, DRAWITEMSTRUCT FAR & );
	void EvInitMenu(HMENU menu);

	// menu items
	void CM_New();
	void CM_New2();
	void CM_New_Vertigo();
	void CM_Open();
	void CM_Save();
	void CM_Save_As();
	void CM_Edit_Mission_File( );
	void CM_Information();
	void CM_Check();
	void CM_Preferences();
	void CM_Exit();
	void CM_Convert();

	void CM_Undo();
	void UpdateUndoBuffer(int enable);
	void CM_Cut();
	void CM_Copy();
	void CM_Paste();
	void CM_Quick_Paste();
	void CM_Delete_Block();
	void CM_Copy_Other_Cube();
	void CM_Mark();
	void CM_MarkAll();
	void CM_UnmarkAll();

	void CM_Zoom_In();
	void CM_Zoom_Out();
	void CM_Pan_Left();
	void CM_Pan_Right();
	void CM_Pan_Up();
	void CM_Pan_Down();
	void CM_Rotate_Horiz_Left();
	void CM_Rotate_Horiz_Right();
	void CM_Rotate_Vert_Up();
	void CM_Rotate_Vert_Down();
	void CM_Rotate_Clockwise();
	void CM_Rotate_Counterclockwise();
	void CM_Center_Mine();
	void CM_Center_Object();
	void CM_Center_Cube();
	void CM_Center_Rotation();
	void CM_Show_Points();
	void CM_Show_Partial_Lines();
	void CM_Show_All_Lines();
	void CM_Show_Nearby_Lines();
	void CM_Show_Filled_Polygons();
	void CM_Show_Doors();
	void CM_Show_Robots();
	void CM_Show_Hostages();
	void CM_Show_Players();
	void CM_Show_Power_Ups();
	void CM_Show_Keys();
	void CM_Show_Weapons();
	void CM_Show_Control_Center();
	void CM_Show_Special();
	void CM_Show_Lights();
	void CM_Show_All_Objects();
	void CM_Show_No_Objects();
	void CM_Show_Partial_Filled();

	void CM_Add_Cube();
	void CM_Add_Reactor();
	void CM_Add_RobotMaker();
	void CM_Add_FuelCenter();
	void CM_Add_Object();
	void CM_Add_Player();
	void CM_Add_CoopPlayer();
	void CM_Add_Robot();
	void CM_Add_Weapon();
	void CM_Add_Powerup();
	void CM_Add_AutoDoor();
	void CM_Add_NormalExit();
	void CM_Add_SecretExit();
	void CM_Add_DoorTrigger();
	void CM_Add_RobotMakerTrigger();
	void CM_Add_ShieldTrigger();
	void CM_Add_EnergyTrigger();
	void CM_Add_UnlockTrigger();
	void CM_Add_GuideBot();
	void CM_Add_FuelCell();
	void CM_Add_PrisonDoor();
	void CM_Add_IllusionaryWall();
	void CM_Add_ForceField();
	void CM_Add_GuideBotDoor();
	void CM_Add_Fan();
	void CM_Add_WaterFall();
	void CM_Add_LavaFall();
	void CM_Add_Grate();

	void CM_Delete_Cube();
	void CM_Delete_Object();
	void CM_Delete_Wall();
	void CM_Delete_Trigger();

	void CM_Join_Cubes();
	void CM_Join_Sides();
	void CM_Join_Lines();
	void CM_Join_Points();
	void CM_Unjoin_Cubes();
	void CM_Unjoin_Sides();
	void CM_Unjoin_Lines();
	void CM_Unjoin_Points();

	void CM_Next_Object();
	void CM_Previous_Object();
	void CM_Next_Cube();
	void CM_Previous_Cube();
	void CM_Forward_Cube();
	void CM_Backwards_Cube();
	void CM_Select_Other_Cube();
	void CM_Next_Side();
	void CM_Previous_Side();
	void CM_Next_Side2();
	void CM_Previous_Side2();
	void CM_Next_Line();
	void CM_Previous_Line();
	void CM_Next_Point();
	void CM_Previous_Point();

	void CM_Edit();
	void CM_Show_Texture();
	void CM_Show_Cube_Data();
	void CM_Show_Object_Data();
	void CM_Show_Wall_Data();
	void CM_Show_Shading();
	void CM_Show_Hires();

	void CM_Point_Edit();
	void CM_Advanced_Object_Data();
	void CM_Trigger_Edit();
	void CM_Light_Adjust();
	void CM_Texture_Alignment();
	void CM_Texture_Picker();
	void CM_Robot_Editor();
	void CM_Blinking_Light();
	void CM_Control_Center_Triggers();
	void CM_Curve_Generator();

	void CM_Help_Index();
	void CM_Help_Keyboard();
	void CM_Help_Commands();
	void CM_Help_Procedures();
	void CM_Help_Using();
	void CM_Help_Register();
	void CM_About();

	void CM_Pan_Out();
	void CM_Pan_In();

	void CM_LoadPreviousFile1();
	void CM_LoadPreviousFile2();
	void CM_LoadPreviousFile3();
	void CM_LoadPreviousFile4();

	void CM_NumPad0();
	void CM_NumPad1();
	void CM_NumPad2();
	void CM_NumPad3();
	void CM_NumPad4();
	void CM_NumPad5();
	void CM_NumPad6();
	void CM_NumPad7();
	void CM_NumPad8();
	void CM_NumPad9();

	void CM_IncreaseSpline();
	void CM_DecreaseSpline();

	void CM_BlockMode();
	void CM_NextSelectMode();
	void CM_NextInsertMode();

	// class based functions
	void handle_timer_1();
	void handle_timer_2();
	void add_door(UINT8 type,UINT8 flags,UINT8 keys, INT8 clipnum, INT16 tmapnum);
	void load_new_level();
	void UpdateObjectChecks();
	void UpdateDialogs();
	void CloseAllDialogs();
	void DrawTopBar();
	void UpdatePreviousFileMenuItem();
	void MySetCaption(LPSTR ATitle);
	void LoadPreviousFile(int index);
	void set_show_option(int option);
	void make_mission_file(char *file_name,int custom_textures, int custom_robots);
	void make_hog(char *rdl_file_name,char *hog_file_name);
	DECLARE_RESPONSE_TABLE( TMainWindow );
};

