#if 0
//==========================================================================
// MENU - Add_Object
//==========================================================================
void TMainWindow::CM_Add_Object()
{
 if (current->object == GameInfo ().objects.count) {
 ErrorMsg("Cannot add another secret return.");
 return;
 }
 if (GameInfo ().objects.count < MAX_OBJECTS) {
	copy_object(OBJ_NONE);
	::PostMessage(HWindow,WM_FIRST + WM_PAINT,0,0L);
 } else {
	ErrorMsg("Maximum numbers of Objects () reached");
 }
}

//==========================================================================
// MENU - Delete_Object
//==========================================================================
void TMainWindow::CM_Delete_Object() {

// HDC hdc;
 if (current->object == GameInfo ().objects.count) {
	ErrorMsg("Cannot delete the secret return.");
	return;
 }
 if (GameInfo ().objects.count > 1) {
	if (QueryMsg("Are you sure you want to delete this object?") == IDYES) {
	 delete_object();
	 ::PostMessage(HWindow,WM_FIRST + WM_PAINT,0,0L);
	}
 } else {
	ErrorMsg("Cannot delete the last object");
 }
}

//==========================================================================
// MENU - Add_Player
//
// Action - Adds a player. Id set to first unused player number.
// Warns user if there are already 8 players
//==========================================================================
void TMainWindow::CM_Add_Player()
{

 if (GameInfo ().objects.count < MAX_OBJECTS) {
	copy_object(OBJ_PLAYER);
	::PostMessage(HWindow,WM_FIRST + WM_PAINT,0,0L);
 } else {
	ErrorMsg("Maximum numbers of Objects () reached");
 }
}

//==========================================================================
// MENU - Add_CoopPlayer
//==========================================================================
void TMainWindow::CM_Add_CoopPlayer()
{

 if (GameInfo ().objects.count < MAX_OBJECTS) {
	copy_object(OBJ_COOP);
	::PostMessage(HWindow,WM_FIRST + WM_PAINT,0,0L);
 } else {
	ErrorMsg("Maximum numbers of Objects () reached");
 }
}

//==========================================================================
// MENU - Add_Robot
//==========================================================================
void TMainWindow::CM_Add_Robot()
{

 if (GameInfo ().objects.count < MAX_OBJECTS) {
	if (copy_object(OBJ_ROBOT)) {
	 Objects ()[current->object].id = 3; // class 1 drone
	 set_object_data(Objects ()[current->object].type);
	}
	::PostMessage(HWindow,WM_FIRST + WM_PAINT,0,0L);
 } else {
	ErrorMsg("Maximum numbers of Objects () reached");
 }
}

//==========================================================================
// MENU - Add Guide Bot
//==========================================================================
void TMainWindow::CM_Add_GuideBot() {
 if (file_type == RDL_FILE) {
	ErrorMsg("Guide bots are not supported in Descent 1");
	return;
 }

 if (GameInfo ().objects.count < MAX_OBJECTS) {
	if (copy_object(OBJ_ROBOT)) {
	 Objects ()[current->object].id = 33; // guide bot
	 set_object_data(Objects ()[current->object].type);
	}
	::PostMessage(HWindow,WM_FIRST + WM_PAINT,0,0L);
 } else {
	ErrorMsg("Maximum numbers of Objects () reached");
 }
}

//==========================================================================
// MENU - Add_Weapon
//==========================================================================
void TMainWindow::CM_Add_Weapon() {
 if (GameInfo ().objects.count < MAX_OBJECTS) {
	if (copy_object(OBJ_POWERUP)) {
	 Objects ()[current->object].id = 3; // laser
	 set_object_data(Objects ()[current->object].type);
	}
	::PostMessage(HWindow,WM_FIRST + WM_PAINT,0,0L);
 } else {
	ErrorMsg("Maximum numbers of Objects () reached");
 }
}

//==========================================================================
// MENU - Add_Powerup
//==========================================================================
void TMainWindow::CM_Add_Powerup()
{

 if (GameInfo ().objects.count < MAX_OBJECTS) {
	if (copy_object(OBJ_POWERUP)) {
	 Objects ()[current->object].id = 1; // energy boost
	 set_object_data(Objects ()[current->object].type);
	}
	::PostMessage(HWindow,WM_FIRST + WM_PAINT,0,0L);
 } else {
	ErrorMsg("Maximum numbers of Objects () reached");
 }
}

#endif