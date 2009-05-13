//==========================================================================
// CLASS -- THogDialog
//==========================================================================
class THogDialog : public TDialog {
	TListBox *LevelsListBox;
	TListBox *SizeListBox;
	TListBox *OffsetListBox;
	TButton  *ImportButton,*ExportButton,*DeleteButton;
	TStatic  *SizeStatic;
	LPSTR HogName;
public:
	THogDialog(TWindow * AParent, LPSTR name, LPSTR filename);
	virtual void SetupWindow();
	void Ok();
	void ImportMsg();
	void ExportMsg();
	void DeleteMsg();
	void RenameMsg();
	void read_hog_data();
	void RefreshData();

	DECLARE_RESPONSE_TABLE( THogDialog );
};
