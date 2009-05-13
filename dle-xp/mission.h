//==========================================================================
// CLASS -- TMissionDialog
//==========================================================================
class TMissionDialog : public TDialog {
	// controls
	TComboBox *TypeComboBox;
	TListBox *LevelsListBox;
	TButton  *InsertButton;
	TButton  *DeleteButton;
	TButton  *MoveDownButton;
	TButton  *MoveUpButton;
	TButton  *RenameButton;
	TEdit    *NameEdit;
	TEdit    *EditorEdit;
	TEdit    *BuildTimeEdit;
	TEdit    *DateEdit;
	TEdit    *BriefingEdit;
	TEdit    *RevisionEdit;
	TEdit    *AuthorEdit;
	TEdit    *EmailEdit;
	TEdit    *WebSiteEdit;
	TEdit    *CommentEdit;
	TCheckBox  *CustomTexturesCheck;
	TCheckBox  *CustomRobotsCheck;
	TCheckBox  *CustomMusicCheck;
	TCheckBox  *NormalCheck;
	TCheckBox  *AnarchyCheck;
	TCheckBox  *RobotAnarchyCheck;
	TCheckBox  *CoopCheck;
	TCheckBox  *CaptureFlagCheck;
	TCheckBox  *HoardCheck;
	TCheckBox  *MultiAuthorCheck;
	TCheckBox  *WantFeedbackCheck;

	char MissionName[256];

public:
	TMissionDialog(TWindow * AParent, LPSTR name, LPSTR filename);
	virtual void SetupWindow();
	void Ok();
    void AutoMsg();
	void InsertMsg();
	void DeleteMsg();
	void MoveDownMsg();
	void MoveUpMsg();
	void RenameMsg();
	void ReadMissionFile();
	void SaveMissionFile();
	void FixLevelName(char *name);
	void ReadStringField(FILE *file, char *field, char *str, int max_length);
	BOOL ReadBoolField(FILE *file, char *field);
	void ReadNextString(FILE *file, char *str, int max_length);
    void StripNewLine(char *str);
	DECLARE_RESPONSE_TABLE( TMissionDialog );
};

