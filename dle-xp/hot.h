// Copyright (C) 1997 Bryan Aamot
//------------------------------------------------------------------------
// DIALOG - THotDialog
//------------------------------------------------------------------------
class THotDialog : public TDialog {
 TButton *OpenButton,*SaveButton,*CubeButton,*ObjectButton,
	 *ToolsButton,*TextureButton,*LightButton,
	 *ZoomInButton,*ZoomOutButton,
	 *RHRightButton,*RHLeftButton,*RVRightButton,*RVLeftButton,
	 *MLeftButton,*MRightButton,*MUpButton,*MDownButton;
 public:
  THotDialog(PTWindowsObject AParent, LPSTR name);
  virtual void SetupWindow();
  virtual void WMMouseActivate(RTMessage Msg) = [WM_FIRST + WM_MOUSEACTIVATE];
  virtual void WMSetFocus(RTMessage Msg) = [WM_FIRST + WM_SETFOCUS];
  virtual void OpenMsg   (RTMessage Msg) = [ID_FIRST+9017];
  virtual void SaveMsg   (RTMessage Msg) = [ID_FIRST+9018];
  virtual void CubeMsg   (RTMessage Msg) = [ID_FIRST+9014];
  virtual void ObjectMsg (RTMessage Msg) = [ID_FIRST+9015];
  virtual void ToolsMsg  (RTMessage Msg) = [ID_FIRST+9012];
  virtual void TextureMsg(RTMessage Msg) = [ID_FIRST+9013];
  virtual void LightMsg  (RTMessage Msg) = [ID_FIRST+9016];
  virtual void ZoomInMsg (RTMessage Msg) = [ID_FIRST+ 9019];
  virtual void ZoomOutMsg(RTMessage Msg) = [ID_FIRST+ 9020];
  virtual void MLeftMsg  (RTMessage Msg) = [ID_FIRST+ 9008];
  virtual void MRightMsg (RTMessage Msg) = [ID_FIRST+ 9010];
  virtual void MUpMsg    (RTMessage Msg) = [ID_FIRST+ 9009];
  virtual void MDownMsg  (RTMessage Msg) = [ID_FIRST+ 9011];
  virtual void RHRightMsg(RTMessage Msg) = [ID_FIRST+ 9021];
  virtual void RHLeftMsg (RTMessage Msg) = [ID_FIRST+ 9022];
  virtual void RVRightMsg(RTMessage Msg) = [ID_FIRST+ 9023];
  virtual void RVLeftMsg (RTMessage Msg) = [ID_FIRST+ 9024];
};

