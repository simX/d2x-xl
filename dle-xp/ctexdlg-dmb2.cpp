                        /*--------------------------*/
		
BEGIN_MESSAGE_MAP (CTextureDlg, CTexToolDlg)
	ON_WM_PAINT ()
	ON_WM_TIMER ()
	ON_WM_HSCROLL ()
	ON_WM_VSCROLL ()
	ON_WM_LBUTTONDOWN ()
	ON_BN_CLICKED (IDC_TEXTURE_SHOWFRAMES, LoadTextureListBoxes)
	ON_BN_CLICKED (IDC_TEXTURE_EDIT, OnEditTexture)
	ON_BN_CLICKED (IDC_TEXTURE_COPY, OnSaveTexture)
	ON_BN_CLICKED (IDC_TEXTURE_PASTESIDE, OnPasteSide)
	ON_BN_CLICKED (IDC_TEXTURE_PASTETOUCHING, OnPasteTouching)
	ON_BN_CLICKED (IDC_TEXTURE_PASTEMARKED, OnPasteMarked)
	ON_BN_CLICKED (IDC_TEXTURE_REPLACE, OnReplace)
	ON_BN_CLICKED (IDC_TEXTURE_PASTE1ST, OnPaste1st)
	ON_BN_CLICKED (IDC_TEXTURE_PASTE2ND, OnPaste2nd)
	ON_EN_KILLFOCUS (IDC_TEXTURE_BRIGHTNESS, OnBrightnessEdit)
	ON_EN_UPDATE (IDC_TEXTURE_BRIGHTNESS, OnBrightnessEdit)
END_MESSAGE_MAP()

                        /*--------------------------*/

CTextureDlg::CTextureDlg (CWnd *pParent)
	: CTexToolDlg (IDD_TEXTURES_DMB2, pParent, 1, IDC_TEXTURE_SHOW, RGB (0,0,0))
{
last_texture1 = -1;
last_texture2 = 0;
save_texture1 = -1;
save_texture2 = 0;
#if TEXTOODLG == 0
m_frame [0] = 0;
m_frame [1] = 0;
#endif
m_bUse1st = TRUE;
m_bUse2nd = FALSE;
*m_szTextureBuf = '\0';
}

                        /*--------------------------*/

CTextureDlg::~CTextureDlg ()
{
if (m_bInited) {
	if (IsWindow (m_textureWnd))
		m_textureWnd.DestroyWindow ();
	}
}

                        /*--------------------------*/

BOOL CTextureDlg::OnInitDialog ()
{
	CRect	rc;

if (!CToolDlg::OnInitDialog ())
   return FALSE;
InitSlider (IDC_TEXTURE_BRIGHTSLIDER, 0, 100);
LoadTextureListBoxes ();
CreateImgWnd (&m_textureWnd, IDC_TEXTURE_SHOW);
UpdateData (FALSE);
m_bInited = TRUE;
return TRUE;
}

                        /*--------------------------*/

void CTextureDlg::DoDataExchange (CDataExchange *pDX)
{
	int		i, nBrightness;
	char		szBrightness [20];

for (i = 0; i < 4; i++)
	DDX_Double (pDX, IDC_TEXTURE_LIGHT1 + i, m_lights [i], 0, 200, "%1.1f");
DDX_Check (pDX, IDC_TEXTURE_PASTE1ST, m_bUse1st);
DDX_Check (pDX, IDC_TEXTURE_PASTE2ND, m_bUse2nd);
DDX_Text (pDX, IDC_TEXTURE_PASTEBUF, m_szTextureBuf, sizeof (m_szTextureBuf));
sprintf (szBrightness, "%d", m_nBrightness);
DDX_Text (pDX, IDC_TEXTURE_BRIGHTNESS, szBrightness, sizeof (szBrightness));
if (pDX->m_bSaveAndValidate && *szBrightness) {
	m_nBrightness = atoi (szBrightness);
	nBrightness = (m_nBrightness < 0) ? 0 : (m_nBrightness > 100) ? 100 : m_nBrightness;
	((CSliderCtrl *) BrightnessSlider ())->SetPos (nBrightness);
	SetBrightness (nBrightness);
	}
}

                        /*--------------------------*/

void CTextureDlg::LoadTextureListBoxes () 
{
	HINSTANCE	hInst = AfxGetApp()->m_hInstance;
	char			name [80];
	int			bShowFrames;
	int			nTextures, iTexture, index;
	CComboBox	*cbTexture1 = CBTexture1 ();
	CComboBox	*cbTexture2 = CBTexture2 ();

GetMine ();
bShowFrames = GetCheck (IDC_TEXTURE_SHOWFRAMES);

INT16 texture1 = m_mine ? m_mine->CurrSide ()->tmap_num : 0;
INT16 texture2 = m_mine ? m_mine->CurrSide ()->tmap_num2 & 0x3fff : 0;

if ((texture1 < 0) || (texture1 >= MAX_TEXTURES))
	texture1 = 0;
if ((texture2 < 0) || (texture2 >= MAX_TEXTURES))
	texture2 = 0;

cbTexture1->ResetContent ();
cbTexture2->ResetContent ();
index = cbTexture1->AddString ("(none)");
texture_resource = (file_type == RDL_FILE) ? D1_TEXTURE_STRING_TABLE : D2_TEXTURE_STRING_TABLE;
nTextures = (file_type == RDL_FILE) ? MAX_D1_TEXTURES : MAX_D2_TEXTURES;
for (iTexture = 0; iTexture < nTextures; iTexture++) {
	LoadString (hInst, texture_resource + iTexture, name, sizeof (name));
	if (bShowFrames || !strstr ((char *) name, "frame")) {
		index = cbTexture1->AddString (name);
		cbTexture1->SetItemData (index, iTexture);
		if (texture1 == iTexture)
			cbTexture1->SetCurSel (index);
		index = cbTexture2->AddString (iTexture ? name : "(none)");
		if (texture2 == iTexture)
			cbTexture2->SetCurSel (index);
		cbTexture2->SetItemData (index, iTexture);
		}
	}
}

                        /*--------------------------*/

bool CTextureDlg::SideHasLight (void)
{
if	((m_mine->IsLight (m_mine->CurrSide ()->tmap_num & 0x3fff) != -1) ||
	 (m_mine->IsLight (m_mine->CurrSide ()->tmap_num2 & 0x3fff) != -1))
	return true;
CDWall *pWall = m_mine->CurrWall ();
return pWall && (pWall->type == WALL_TRANSPARENT);

}

                        /*--------------------------*/

void CTextureDlg::OnPaint ()
{
CTexToolDlg::OnPaint ();
#if TEXTOOLDLG == 0
UpdateTextureWnd ();
#endif
}

                        /*--------------------------*/

void CTextureDlg::UpdateTextureWnd (void)
{
#if TEXTOOLDLG == 0
RefreshTextureWnd ();
m_textureWnd.InvalidateRect (NULL);
m_textureWnd.UpdateWindow ();
#endif
}

                        /*--------------------------*/

void CTextureDlg::Refresh ()
{
if (!m_bInited)
	return;
if (!GetMine ())
	return;
//Beep (1000,100);
	HINSTANCE	hInst = AfxGetApp()->m_hInstance;
	CComboBox	*cbTexture1 = CBTexture1 ();
	CComboBox	*cbTexture2 = CBTexture2 ();
	INT16			texture1, texture2, mode;
	bool			bShowTexture;
	int			i, j;
	CDSegment	*seg;
	CDSide		*side;
	CDWall		*pWall;
	CDColor	*color;
// enable buttons as required
/*
EditButton->EnableWindow((file_type == RDL_FILE || path [0] == NULL) ? FALSE: TRUE);
LightButton->EnableWindow((file_type==RDL_FILE) ? FALSE:TRUE);
PickButton->EnableWindow(path [0] ? TRUE:FALSE);
*/
// set animation frames to zero
#if TEXTOOLDLG == 0
m_frame [0] = 0;
m_frame [1] = 0;
#endif

seg = m_mine->CurrSeg ();
side = m_mine->CurrSide ();
color = m_mine->CurrLightColor ();
int nSide = m_mine->Current ()->side;
texture1 = side->tmap_num;
texture2 = side->tmap_num2 & 0x3fff;
pWall = m_mine->CurrWall ();
m_nColorIndex = (pWall && (pWall->type == WALL_TRANSPARENT)) ? pWall->cloak_value : color->index;
m_rgbColor.peRed = (char) (255.0 * color->color.r);
m_rgbColor.peGreen = (char) (255.0 * color->color.g);
m_rgbColor.peBlue = (char) (255.0 * color->color.b);
if ((texture1 < 0) || (texture1 >= MAX_TEXTURES))
	texture1 = 0;
if ((texture2 < 0) || (texture2 >= MAX_TEXTURES))
	texture2 = 0;
GetBrightness ((m_bUse2nd && !m_bUse1st) ? texture2 : texture1);
mode = side->tmap_num2 & 0xc000;
// set edit fields to % of light and enable them
for (i = 0; i < 4; i++) {
/*
	switch(mode) {
		case (INT16) 0x0000: 
			j = i;       
			break;
		case (INT16) 0x4000: 
			j = (i + 3) % 4; 
			break;
		case (INT16) 0x8000: 
			j = (i + 2) % 4;
			break;
		case (INT16) 0xC000: 
			j = (i + 1) % 4;
			break;
		default: 
			j = i;
		}
*/
	j = (i + lightIdxFromMode [mode / 0x4000]) % 4;
	m_lights [j] = (double) ((UINT16) side->uvls [i].l) / 327.68;
	}

if (seg->children [nSide]==-1)
	bShowTexture = TRUE;
else {
	INT16 wall_num = side->wall_num;
	bShowTexture = (wall_num < m_mine->GameInfo ().walls.count);
	}
if (bShowTexture) {
	if ((texture1!=last_texture1) || (texture2!=last_texture2) || (mode!=last_mode)) {
		last_texture1 = texture1;
		last_texture2 = texture2;
		last_mode = mode;
		LoadString (hInst, texture_resource + texture1, message, sizeof (message));
		cbTexture1->SetCurSel (i = cbTexture1->SelectString (-1, message));  // unselect if string not found
		if (side->tmap_num2) {
			LoadString (hInst, texture_resource + texture2, message, sizeof (message));
			cbTexture2->SetCurSel (cbTexture2->SelectString (-1, message));  // unselect if string not found
			}
		else
			cbTexture2->SelectString (-1, "(none)");
		}
	}
else {
	last_texture1 = -1;
	last_texture2 = -1;
	cbTexture1->SelectString (-1, "(none)");
	cbTexture2->SelectString (-1, "(none)");
	}
RefreshAlignment ();
UpdateData (FALSE);
#if TEXTOOLDLG
CTexToolDlg::Refresh ();
#else
UpdateTextureWnd ();
#endif
UpdateAlignWnd ();
UpdateLightWnd ();
UpdatePaletteWnd ();
}

                        /*--------------------------*/

void CTextureDlg::RefreshTextureWnd ()
{
#if TEXTOOLDLG
if (!CTexToolDlg::Refresh ())
#else
if (!PaintTexture (&m_textureWnd))
#endif
	last_texture1 =
	last_texture2 = -1;
}

                        /*--------------------------*/
#if TEXTOOLDLG == 0
void CTextureDlg::DrawTexture (INT16 texture1, INT16 texture2, int x0, int y0) 
{
	UINT8 bitmap_array [64*64];
	int x_offset;
	int y_offset;
	x_offset = 0;
	y_offset = 0;
	CDC	*pDC = m_textureWnd.GetDC ();
	CDTexture tx (bmBuf);

memset (bitmap_array,0,sizeof (bitmap_array));
if (DefineTexture(texture1,texture2,&tx,x0,y0))
	return;
BITMAPINFO *bmi;
bmi = MakeBitmap ();
CPalette	*oldPalette = pDC->SelectPalette (thePalette, FALSE);
pDC->RealizePalette ();
//SetDIBitsToDevice (pDC->m_hDC, x_offset, y_offset, 64, 64, 0, 0, 0, 64, bitmap_array, bmi, DIB_RGB_COLORS);
CRect	rc;
m_textureWnd.GetClientRect (&rc);
StretchDIBits (pDC->m_hDC, x_offset, y_offset, rc.Width (), rc.Height (), 0, 0, tx.width, tx.height,
		        	(void *) bitmap_array, bmi, DIB_RGB_COLORS, SRCCOPY);
pDC->SelectPalette(oldPalette, FALSE);
m_textureWnd.ReleaseDC (pDC);
}
#endif
                        /*--------------------------*/

BOOL CTextureDlg::OnSetActive ()
{
#if TEXTOOLDLG
CTexToolDlg::OnSetActive ();
#else
m_nTimer = SetTimer (1, 100U, NULL);
#endif
if (m_iLight >= 0)
	m_nLightTimer = SetTimer (2, m_nLightDelay, NULL);
Refresh ();
return CToolDlg::OnSetActive ();
}

                        /*--------------------------*/

BOOL CTextureDlg::OnKillActive ()
{
#if TEXTOOLDLG
CTexToolDlg::OnKillActive ();
#else
if (m_nTimer >= 0) {
	KillTimer (m_nTimer);
	m_nTimer = -1;
	}
#endif
if (m_nLightTimer >= 0) {
	KillTimer (m_nLightTimer);
	m_nLightTimer = -1;
	}
return CToolDlg::OnKillActive ();
}

                        /*--------------------------*/

#if TEXTOOLDLG == 0

void CTextureDlg::AnimateTexture (void)
{
if (!GetMine ())
	return;

	CDSegment *seg = m_mine->CurrSeg ();

	UINT16 texture [2];
	static int scroll_offset_x = 0;
	static int scroll_offset_y = 0;
	int bScroll;
	int x,y;
	static int old_x,old_y;

	CDSide	*side = m_mine->CurrSide ();

texture [0] = side->tmap_num & 0x3fff;
texture [1] = side->tmap_num2;

// if texture1 is a scrolling texture, then offset the textures and
// redraw them, then return
bScroll = m_mine->ScrollSpeed (texture [0], &x, &y);
if (bScroll) {
	DrawTexture (texture [0], texture [1], scroll_offset_x, scroll_offset_y);
	if (old_x != x || old_y != y) {
		scroll_offset_x = 0;
		scroll_offset_y = 0;
		}
	old_x = x;
	old_y = y;
	scroll_offset_x += x;
	scroll_offset_y += y;
	scroll_offset_x &= 63;
	scroll_offset_y &= 63;
	return;
	}

scroll_offset_x = 0;
scroll_offset_y = 0;

// abort if this is not a wall
#ifndef _DEBUG
INT16 wall_num = side->wall_num;
if (wall_num >= m_mine->GameInfo ().walls.count)
	return;

// abort if this wall is not a door
if (m_mine->Walls () [wall_num].type != WALL_DOOR)
	return;
#endif
	int i;
	static int hold_time [2] = {0,0};
	static int inc [2]= {1,1}; // 1=forward or -1=backwards
	int index [2];
	static UINT16 d1_anims [] = {
		371, 376, 387, 399, 413, 419, 424, 436, 444, 459,
		472, 486, 492, 500, 508, 515, 521, 529, 536, 543,
		550, 563, 570, 577, 584, 0
		};
// note: 584 is not an anim texture, but it is used to calculate
//       the number of frames in 577
// The 0 is used to end the search

	static UINT16 d2_anims [] = {
		435, 440, 451, 463, 477, 483, 488, 500, 508, 523,
		536, 550, 556, 564, 572, 579, 585, 593, 600, 608,
		615, 628, 635, 642, 649, 664, 672, 687, 702, 717,
		725, 731, 738, 745, 754, 763, 772, 780, 790, 806,
		817 ,827 ,838 ,849 ,858 ,863 ,871 ,886, 901 ,910,
		0
		};
// note: 910 is not an anim texture, but it is used to calculate
//       the number of frames in 901
// The 0 is used to end the search
	UINT16 *anim; // points to d1_anim or d2_anim depending on file_type

// first find out if one of the textures is animated
anim = (file_type == RDL_FILE) ? d1_anims : d2_anims;

for (i=0; i<2;i++)
	for (index [i] = 0; anim [index [i]]; index [i]++)
		if (texture [i] == anim [index [i]])
			break;

if (anim [index [0]] || anim [index [1]]) {
	// calculate new texture numbers
	for (i = 0; i < 2; i++) {
		if (anim [index [i]]) {
		// if hold time has not expired, then return
			if (hold_time [i] < 5)
				hold_time [i]++;
			else
				m_frame [i] += inc [i];
			if (m_frame [i] < 0) {
				m_frame [i] = 0;
				hold_time [i] = 0;
				inc [i] = 1;
				}
			if (anim [index [i]] + m_frame [i] >= anim [index [i] + 1]) {
				m_frame [i] = (anim [index [i] + 1] - anim [index [i]]) - 1;
				hold_time [i] = 0;
				inc [i] = -1;
				}
			texture [i] = anim [index [i]] + m_frame [i];
			}
		}
	DrawTexture (texture [0], texture [1], 0, 0);
	}
}
#endif
                        /*--------------------------*/

void CTextureDlg::OnTimer (UINT nIdEvent)
{
#if TEXTOOLDLG
if (nIdEvent == 2)
	AnimateLight ();
else 
	CTexToolDlg::OnTimer (nIdEvent);
#else
if (nIdEvent == 1)
	AnimateTexture ();
else if (nIdEvent == 2)
	AnimateLight ();
else 
	CToolDlg::OnTimer (nIdEvent);
#endif
}

                        /*--------------------------*/

void CTextureDlg::SelectTexture (int nIdC, bool bFirst)
{
if (!GetMine ())
	return;

	CDSide		*side = m_mine->CurrSide ();
	CComboBox	*pcb = bFirst ? CBTexture1 () : CBTexture2 ();
	int			index = pcb->GetCurSel ();
	
if (index <= 0)
	side->tmap_num2 = 0;
else {
	INT16 texture = (INT16) pcb->GetItemData (index);
	if (bFirst)
		m_mine->SetTexture (-1, -1, texture, -1);
	else
		m_mine->SetTexture (-1, -1, -1, texture);
	}
Refresh ();
theApp.MineView ()->Refresh ();
}

                        /*--------------------------*/

void CTextureDlg::OnSetLight () 
{
if (!GetMine ())
	return;
UpdateData (TRUE);
CDSide *side = m_mine->CurrSide ();
INT16 mode = side->tmap_num2 & 0xc000;
for (int i = 0; i < 4; i++) {
	int j = (i + lightIdxFromMode [mode / 0x4000]) % 4;
	side->uvls [i].l = (UINT16) (m_lights [j] * 327.68);
	}
}

                        /*--------------------------*/

void CTextureDlg::OnSelect1st () 
{
SelectTexture (IDC_TEXTURE_1ST, true);
}

                        /*--------------------------*/

void CTextureDlg::OnSelect2nd () 
{
SelectTexture (IDC_TEXTURE_2ND, false);
}

                        /*--------------------------*/

void CTextureDlg::OnEditTexture () 
{
	CTextureEdit	e (NULL);

int i = e.DoModal ();
theApp.MineView ()->Refresh (false);
Refresh ();
}

                        /*--------------------------*/

void CTextureDlg::OnSaveTexture () 
{
if (!GetMine ())
	return;

	HINSTANCE	hInst = AfxGetApp()->m_hInstance;
	char			t1Name [20],
					t2Name [20];
	CDSide		*side = m_mine->CurrSide ();
	CComboBox	*pcb;

save_texture1 = side->tmap_num & 0x3fff;
save_texture2 = side->tmap_num2 & 0x3fff;
for (int i = 0; i < 4; i++)
	save_uvls [i].l = side->uvls [i].l;

//CBTexture1 ()->SelectString (-1, texture_name1);
//CBTexture2 ()->SelectString (-1, texture_name2);
pcb = CBTexture1 ();
LoadString (hInst, texture_resource + pcb->GetItemData (pcb->GetCurSel ()), t1Name, sizeof (t1Name));
pcb = CBTexture2 ();
if (i = pcb->GetItemData (pcb->GetCurSel ()))
	LoadString (hInst, texture_resource + i, t2Name, sizeof (t2Name));
else
	strcpy (t2Name, "(none)");
sprintf(m_szTextureBuf ,"%s,%s", t1Name, t2Name);
UpdateData (FALSE);
//SaveTextureStatic->SetText(message);
}

                        /*--------------------------*/

void CTextureDlg::OnPaste1st () 
{
m_bUse1st = !m_bUse1st;
Refresh ();
}

                        /*--------------------------*/

void CTextureDlg::OnPaste2nd () 
{
m_bUse2nd = !m_bUse2nd;
Refresh ();
}

                        /*--------------------------*/

void CTextureDlg::OnPasteSide () 
{
UpdateData (TRUE);
if (!(m_bUse1st || m_bUse2nd))
	return;
if (!GetMine ())
	return;

	CDSide *side = m_mine->CurrSide ();

//CheckForDoor ();
theApp.SetModified (TRUE);
m_mine->SetTexture (m_mine->Current ()->segment, m_mine->Current ()->side, 
						  m_bUse1st ? save_texture1 : -1, m_bUse2nd ? save_texture2 : -1);
for (int i = 0; i < 4; i++)
	side->uvls [i].l = save_uvls [i].l;
Refresh ();
theApp.MineView ()->Refresh ();
}


                        /*--------------------------*/

void CTextureDlg::OnPasteTouching ()
{
UpdateData (TRUE);
if (!(m_bUse1st || m_bUse2nd))
	return;
if (!GetMine ())
	return;
if (save_texture1 == -1 || save_texture2 == -1)
	return;
//CheckForDoor ();
// set all segment sides as not "pasted" yet
	CDSegment *seg = m_mine->Segments ();
for (int segnum = m_mine->SegCount (); segnum; segnum--, seg++)
    seg->seg_number = 0;
theApp.SetModified (TRUE);
theApp.LockUndo ();
PasteTexture (m_mine->Current ()->segment, m_mine->Current ()->side, 100);
theApp.UnlockUndo ();
Refresh ();
theApp.MineView ()->Refresh ();
}

                        /*--------------------------*/

void CTextureDlg::OnPasteMarked () 
{
UpdateData (TRUE);
if (!(m_bUse1st || m_bUse2nd))
	return;
if (!GetMine ())
	return;

	INT16			segnum,
					sidenum;
	CDSegment	*seg = m_mine->Segments ();
	CDSide		*side;
	bool			bChange = false,
					bAll = !m_mine->GotMarkedSides ();

if (bAll && (QueryMsg ("Paste texture to entire mine?") != IDYES))
	return;
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bAll)
	INFOMSG (" Pasting texture in entire mine.");
for (segnum = 0; segnum < m_mine->SegCount (); segnum++, seg++) {
	for (sidenum = 0, side = seg->sides; sidenum < 6; sidenum++, side++) {
		if (bAll || m_mine->SideIsMarked (segnum, sidenum)) {
			if (seg->children [sidenum] == -1) {
				bChange = true;
				m_mine->SetTexture (segnum, sidenum, m_bUse1st ? save_texture1 : -1, m_bUse2nd ? save_texture2 : -1);
				for (int i = 0; i < 4; i++)
					side->uvls [i].l = save_uvls [i].l;
				}
			}
		}
	}
if (bChange)
	theApp.UnlockUndo ();
else
	theApp.ResetModified (bUndo);
Refresh ();
theApp.MineView ()->Refresh ();
}

                        /*--------------------------*/

void CTextureDlg::OnReplace () 
{
UpdateData (TRUE);
if (!(m_bUse1st || m_bUse2nd))
	return;
if (!GetMine ())
	return;

	INT16			segnum,
					sidenum;
	CDSegment	*seg = m_mine->Segments ();
	CDSide		*side;
	bool			bChange = false,
					bAll = !m_mine->GotMarkedSides ();

if (bAll && (QueryMsg ("Replace textures in entire mine?") != IDYES))
	return;
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bAll)
	INFOMSG (" Replacing textures in entire mine.");
for (segnum = 0; segnum < m_mine->SegCount (); segnum++, seg++)
	for (sidenum = 0, side = seg->sides; sidenum < 6; sidenum++, side++)
		if (bAll || m_mine->SideIsMarked (segnum, sidenum)) {
			if (m_bUse1st && (side->tmap_num != last_texture1))
				continue;
			if (m_bUse2nd && ((side->tmap_num2 & 0x3FFF) != last_texture2))
				continue;
			if ((seg->children [sidenum] >= 0) && (side->wall_num == NO_WALL))
				 continue;
			if (m_mine->SetTexture (segnum, sidenum, m_bUse1st ? save_texture1 : -1, m_bUse2nd ? save_texture2 : -1))
				bChange = true;
			for (int i = 0; i < 4; i++)
				side->uvls [i].l = save_uvls [i].l;
			}
if (bChange)
	theApp.UnlockUndo ();
else
	theApp.ResetModified (bUndo);
Refresh ();
theApp.MineView ()->Refresh ();
}

                        /*--------------------------*/

void CTextureDlg::PasteTexture (INT16 segnum, INT16 sidenum, INT16 nDepth) 
{
if (nDepth <= 0) 
	return;

if (!GetMine ())
	return;

	CDSegment	*seg = m_mine->Segments (segnum);
	CDSide		*side = seg->sides + sidenum;
	INT16			old_texture1, 
					old_texture2;
	int			i;

// remember these texture for a comparison below
old_texture1 = side->tmap_num;
old_texture2 = side->tmap_num2;
if ((old_texture1 < 0) || (old_texture1 >= MAX_TEXTURES))
	old_texture1 = 0;
if ((old_texture2 < 0) || (old_texture2 >= MAX_TEXTURES))
	old_texture2 = 0;
// mark segment as "pasted"
seg->seg_number = 1;
// paste texture
m_mine->SetTexture (segnum, sidenum, m_bUse1st ? save_texture1 : -1, m_bUse2nd ? save_texture2 : -1);
for (i = 0; i < 4; i++)
	side->uvls [i].l = save_uvls [i].l;

// now check each adjing side to see it has the same texture
for (i = 0; i < 4; i++) {
	INT16 adj_segnum, adj_sidenum;
	if (GetAdjacentSide (segnum, sidenum, i, &adj_segnum, &adj_sidenum)) {
		// if adj matches and its not "pasted" yet
		seg = m_mine->Segments (adj_segnum);
		side = seg->sides + adj_sidenum;
#if 0
		if (seg->seg_number)
			continue;
		if (m_bUse1st && (side->tmap_num != old_texture1))
			continue;
		if (m_bUse2nd && (side->tmap_num2 != old_texture2))
			continue;
		PasteTexture (adj_segnum, adj_sidenum, --nDepth);
#else
		if ((seg->seg_number == 0) &&
			 (!m_bUse1st || (side->tmap_num == old_texture1)) &&
			 (!m_bUse2nd || (side->tmap_num2 == old_texture2))) {
			PasteTexture (adj_segnum, adj_sidenum, --nDepth);
			}
#endif
		}
	}
}

                        /*--------------------------*/

bool CTextureDlg::GetAdjacentSide (INT16 start_segment, INT16 start_side, INT16 linenum,
												INT16 *adj_segnum, INT16 *adj_sidenum) 
{
if (!GetMine ())
	return false;

	CDSegment *seg;
	INT16 sidenum,childnum;
	INT16 point0,point1,vert0,vert1;
	INT16 childs_side,childs_line;
	INT16 childs_point0,childs_point1,childs_vert0,childs_vert1;
	int side_child[6][4] = {
		{4,3,5,1},
		{2,4,0,5},
		{5,3,4,1},
		{0,4,2,5},
		{2,3,0,1},
		{0,3,2,1}
		};

  // figure out which side of child shares two points w/ start_side
  // find vert numbers for the line's two end points
point0 = line_vert[side_line[start_side][linenum]][0];
point1 = line_vert[side_line[start_side][linenum]][1];
seg = m_mine->Segments () + start_segment;
vert0  = seg->verts[point0];
vert1  = seg->verts[point1];

sidenum = side_child[start_side][linenum];
childnum = seg->children[sidenum];
if (childnum < 0 || childnum >= m_mine->SegCount ())
	return false;
for (childs_side=0;childs_side<6;childs_side++) {
	seg = m_mine->Segments () + childnum;
	if ((seg->children[childs_side] == -1) ||
	    (seg->sides[childs_side].wall_num < m_mine->GameInfo ().walls.count)) {
		for (childs_line=0;childs_line<4;childs_line++) {
			// find vert numbers for the line's two end points
			childs_point0 = line_vert[side_line[childs_side][childs_line]][0];
			childs_point1 = line_vert[side_line[childs_side][childs_line]][1];
			childs_vert0  = seg->verts[childs_point0];
			childs_vert1  = seg->verts[childs_point1];
			// if points of child's line == corresponding points of parent
			if (childs_vert0 == vert1 && childs_vert1 == vert0 ||
				childs_vert0 == vert0 && childs_vert1 == vert1) {
				// now we know the child's side & line which touches the parent
				// child:  childnum, childs_side, childs_line, childs_point0, childs_point1
				// parent: start_segment, start_side, linenum, point0, point1
				*adj_segnum = childnum;
				*adj_sidenum = childs_side;
				return true;
				}
			}
		}
	}
return false;
}
                        /*--------------------------*/

void CTextureDlg::GetBrightness (int nTexture)
{
	int nBrightness;

if (nTexture < 0)
	nBrightness = 0;
else {
	nBrightness = lightMap [nTexture];
	if (nBrightness == MAX_BRIGHTNESS)
		nBrightness = 100;
	else
		nBrightness = (100 * nBrightness + MAX_BRIGHTNESS / 2) / MAX_BRIGHTNESS;
	((CSliderCtrl *) BrightnessSlider ())->SetPos (nBrightness);
	}
m_nBrightness = nBrightness;
}

                        /*--------------------------*/

void CTextureDlg::SetBrightness (int nBrightness)
{
	static	BOOL	bSemaphore = FALSE;

if (!bSemaphore) {
	bSemaphore = TRUE;

	CComboBox	*pcb = (m_bUse2nd && !m_bUse1st) ? CBTexture2 () : CBTexture1 ();
	int			index = pcb->GetCurSel ();
	INT16			texture = (INT16) pcb->GetItemData (index);

	if (texture >= 0) {
		m_nBrightness = nBrightness;
		lightMap [texture] = ((nBrightness == 100) ? MAX_BRIGHTNESS : nBrightness * (MAX_BRIGHTNESS / 100));
		}
	bSemaphore = FALSE;
	}
}

                        /*--------------------------*/

void CTextureDlg::OnBrightnessEdit ()
{
UpdateData (TRUE);
}

                        /*--------------------------*/

void CTextureDlg::OnHScroll(UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
	int	nPos = pScrollBar->GetScrollPos ();
	CRect	rc;

if (pScrollBar == BrightnessSlider ()) {
	switch (scrollCode) {
		case SB_LINEUP:
			nPos--;
			break;
		case SB_LINEDOWN:
			nPos++;
			break;
		case SB_PAGEUP:
			nPos -= 10;
			break;
		case SB_PAGEDOWN:
			nPos += 10;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nPos = thumbPos;
			break;
		case SB_ENDSCROLL:
			return;
		}
	if (nPos < 0)
		nPos = 0;
	else if (nPos > 100)
		nPos = 100;
	SetBrightness (nPos);
	UpdateData (FALSE);
//	pScrollBar->SetScrollPos (nPos, TRUE);
	}
else {
	m_alignWnd.GetClientRect (rc);
	switch (scrollCode) {
	case SB_LINEUP:
		nPos--;
		break;
	case SB_LINEDOWN:
		nPos++;
		break;
	case SB_PAGEUP:
		nPos -= rc.Width () / 4;
		break;
	case SB_PAGEDOWN:
		nPos += rc.Width () / 4;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nPos = thumbPos;
		break;
	case SB_ENDSCROLL:
		return;
	}
  }
pScrollBar->SetScrollPos (nPos, TRUE);
UpdateAlignWnd ();
}

                        /*--------------------------*/

void CTextureDlg::OnVScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
	int	nPos = pScrollBar->GetScrollPos ();
	CRect	rc;

if (pScrollBar == TimerSlider ()) {
	if (m_iLight < 0)
		return;
	switch (scrollCode) {
		case SB_LINEUP:
			nPos++;
			break;
		case SB_LINEDOWN:
			nPos--;
			break;
		case SB_PAGEUP:
			nPos += 5;
			break;
		case SB_PAGEDOWN:
			nPos -= 5;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nPos = 20 - thumbPos;
			break;
		case SB_ENDSCROLL:
			return;
		}
	if (nPos < 0)
		nPos = 0;
	else if (nPos > 20)
		nPos = 20;
	SetLightDelay (nPos * 50);
//	pScrollBar->SetScrollPos (nPos, TRUE);
	}
else if (pScrollBar == BrightnessSlider ()) {
	switch (scrollCode) {
		case SB_LINEUP:
			nPos--;
			break;
		case SB_LINEDOWN:
			nPos++;
			break;
		case SB_PAGEUP:
			nPos -= 10;
			break;
		case SB_PAGEDOWN:
			nPos += 10;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nPos = thumbPos;
			break;
		case SB_ENDSCROLL:
			return;
		}
	if (nPos < 0)
		nPos = 0;
	else if (nPos > 100)
		nPos = 100;
	SetBrightness (nPos);
	UpdateData (FALSE);
//	pScrollBar->SetScrollPos (nPos, TRUE);
	}
else {
	m_alignWnd.GetClientRect (rc);
	switch (scrollCode) {
		case SB_LINEUP:
			nPos--;
			break;
		case SB_LINEDOWN:
			nPos++;
			break;
		case SB_PAGEUP:
			nPos -= rc.Height () / 4;
			break;
		case SB_PAGEDOWN:
			nPos += rc.Height () / 4;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nPos = thumbPos;
			break;
		case SB_ENDSCROLL:
			return;
		}
	pScrollBar->SetScrollPos (nPos, TRUE);
	UpdateAlignWnd ();
	}
}
