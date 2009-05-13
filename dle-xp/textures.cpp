#include "stdafx.h"
#include "textures.h"
#include "io.h"
#include "dlcres.h"
#include "global.h"
#include "palette.h"
#include "dlc.h"

CDTexture pTextures [2][MAX_D2_TEXTURES];

struct tExtraTexture;

typedef struct tExtraTexture	*pExtraTexture;

struct tExtraTexture {
	pExtraTexture	pNext;
	CDTexture		texture;
	UINT16			texture_index;
} tExtraTexture;

pExtraTexture	extraTextures = NULL;

UINT8 bmBuf [2048 * 2048 * 4];

//------------------------------------------------------------------------

inline int Sqr (int i)
{
return i * i;
}


inline int ColorDelta (unsigned char r, unsigned char g, unsigned char b, PALETTEENTRY *sysPal, int j)
{
sysPal += j;
return 
	Sqr (r - sysPal->peRed) + 
	Sqr (g - sysPal->peGreen) + 
	Sqr (b - sysPal->peBlue);
}

//------------------------------------------------------------------------

inline unsigned int ClosestColor (unsigned char r, unsigned char g, unsigned char b, PALETTEENTRY *sysPal)
{
	unsigned int	k, delta, closestDelta = 0x7fffffff, closestIndex = 0;

for (k = 0; (k < 256) && closestDelta; k++) {
	delta = ColorDelta (r, g, b, sysPal, k);
	if (delta < closestDelta) {
		closestIndex = k;
		closestDelta = delta;
		}
	}
return closestIndex;
}

//------------------------------------------------------------------------

bool TGA2Bitmap (tRGBA *pTGA, UINT8 *pBM, int nWidth, int nHeight)
{
	tRGBA			rgba = {0,0,0,0};
	UINT8			*bConverted = NULL;

PALETTEENTRY *sysPal = (PALETTEENTRY *) malloc (256 * sizeof (PALETTEENTRY));
if (!sysPal) {
	ErrorMsg ("Not enough memory for palette.");
	return false;
	}
thePalette->GetPaletteEntries (0, 256, sysPal);

int nSize = nWidth * nHeight;	//only convert the 1st frame of animated TGAs
int h = nSize, i = 0, k, x, y;

#if 1
for (i = 0, k = nSize + nWidth, y = nHeight; y; y--)
	for (x = nWidth, k -= 2 * nWidth; x; i++, k++, x--) {
		rgba = pTGA [i];
		pBM [k] = ClosestColor (rgba.r, rgba.g, rgba.b, sysPal);
		}
#else
if (bConverted = (UINT8 *) malloc ((nSize + 7) / 8))
	memset (bConverted, 0, (nSize + 7) / 8);

nextPixel:

if (bConverted) {
	for (k = nSize; (i < h) && nSize; i++) {
		rgba = pTGA [i];
#	if 0
		if (bConverted [i >> 3] & (1 << (i & 7)))
			continue;
		closestIndex = ClosestColor (rgba.r, rgba.g, rgba.b, sysPal);
#	else
//		CBRK ((rgba.r == 252) && (rgba.b == 252) && (rgba.b == 252));
#		if 0
		pBM [--k] = ClosestColor (rgba.r, rgba.g, rgba.b, sysPal);
#		else
		k = (nHeight - (i / nHeight) - 1);
		k *= nWidth;
		k += i % nWidth;
		pBM [k] = ClosestColor (rgba.r, rgba.g, rgba.b, sysPal);
//		pBM [(nHeight - (i / nHeight) - 1) * nWidth + i % nWidth] = ClosestColor (rgba.r, rgba.g, rgba.b, sysPal);
#		endif
		continue;
#	endif
		for (j = i, pRGBA = pTGA + j; (j < h) && nSize; j++, pRGBA++) {
			if (bConverted [j >> 3] & (1 << (j & 7)))
				continue;
			if ((j == i) || ((pRGBA->r == rgba.r) && (pRGBA->g == rgba.g) && (pRGBA->b == rgba.b))) {
				pBM [j] = closestIndex;
				bConverted [j >> 3] |= (1 << (j & 7));
				nSize--;
				}
			}
		}
	}
else {
	for (; (i < h) && nSize; i++) {
		hRgba = rgba;
		rgba = pTGA [i];
		// look if this color already converted
		for (j = i - 1, pRGBA = pTGA + j; j >= 0; j--, pRGBA--)
			if ((pRGBA->r == rgba.r) && (pRGBA->g == rgba.g) && (pRGBA->b == rgba.b)) {
				i++;
				goto nextPixel;
				}
		closestIndex = ClosestColor (rgba.r, rgba.g, rgba.b, sysPal);
		for (j = i, pRGBA = pTGA + j; (j < h) && nSize; j++, pRGBA++) {
			if ((pRGBA->r == rgba.r) && (pRGBA->g == rgba.g) && (pRGBA->b == rgba.b)) {
				pBM [j] = closestIndex;
				nSize--;
				}
			}
		}
	}
if (bConverted)
	free (bConverted);
#endif
free (sysPal);
return true;
}

//------------------------------------------------------------------------
// DefineTexture ()
//
// ACTION - Defines data with texture.  If bitmap_handle != 0,
//          then data is copied from the global data.  Otherwise,
//          the pig file defines bmBuf and then global memory
//          is allocated and the data is copied to the global memory.
//          The next time that texture is used, the handle will be defined.
//------------------------------------------------------------------------

int DefineTexture (INT16 nBaseTex,INT16 nOvlTex, CDTexture *pDestTx, int x0, int y0) 
{
	typedef struct tFrac {
		int	c, d;
	} tFrac;

	UINT8			*ptr;
	INT16			textures [2], mode, w, h, s;
	int			i, x, y, y1, offs;
	tFrac			scale, scale2;
	int			rc; // return code
	CDTexture	*pTx [2];
	UINT8			*bmBuf = pDestTx->m_pDataBM;
	UINT8			c;
	
	textures [0] = nBaseTex;
	textures [1] = nOvlTex & 0x3fff;
	mode     = nOvlTex & 0xC000;
	for (i = 0; i < 2; i++) {
#if 0	
	ASSERT (textures [i] < MAX_TEXTURES);
#endif
if ((textures [i] < 0) || (textures [i] >= MAX_TEXTURES))
	textures [i] = 0;
	// buffer textures if not already buffered
	pTx [i] = pTextures [file_type] + textures [i];
	if (!(pTx [i]->m_pDataBM && pTx [i]->m_bValid))
		if (rc = pTx [i]->Read (textures [i]))
			return (rc);
	}
	
	// Define bmBuf based on texture numbers and rotation
pDestTx->m_width = pTx [0]->m_width;
pDestTx->m_height = pTx [0]->m_height;
pDestTx->m_size = pTx [0]->m_size;
pDestTx->m_bValid = 1;
ptr = pTx [0]->m_pDataBM;
//CBRK (textures [0] == 220);
if (ptr) {
	// if not rotated, then copy directly
	if (x0 == 0 && y0 == 0) 
		memcpy (bmBuf, ptr, pTx [0]->m_size);
	else {
		// otherwise, copy bit by bit
		w = pTx [0]->m_width;
#if 1
		int	l1 = y0 * w + x0;
		int	l2 = pTx [0]->m_size - l1;
		memcpy (bmBuf, ptr + l1, l2);
		memcpy (bmBuf + l2, ptr, l1);
#else
		UINT8		*dest = bmBuf;
		h = w;//pTx [0]->m_height;
		for (y = 0; y < h; y++)
			for (x = 0; x < w; x++)
				*dest++ = ptr [(((y - y0 + h) % h) * w) + ((x - x0 + w) % w)];
#endif
		}
	}

// Overlay texture 2 if present

if (textures [1] == 0)
	return 0;
if (!(ptr = pTx [1]->m_pDataBM))
	return 0;
if (pTx [0]->m_width == pTx [1]->m_width)
	scale.c = scale.d = 1;
else if (pTx [0]->m_width < pTx [1]->m_width) {
	scale.c = pTx [1]->m_width / pTx [0]->m_width;
	scale.d = 1;
	}
else if (pTx [0]->m_width > pTx [1]->m_width) {
	scale.d = pTx [0]->m_width / pTx [1]->m_width;
	scale.c = 1;
	}
scale2.c = scale.c * scale.c;
scale2.d = scale.d * scale.d;
offs = 0;
w = pTx [1]->m_width / scale.c * scale.d;
h = w;//pTx [1]->m_height / scale.c * scale.d;
s = (pTx [1]->m_width * pTx [1]->m_width)/*pTx [1]->m_size*/ / scale2.c * scale2.d;
if (!(x0 || y0)) {
	UINT8 *dest, *dest2;
	if (mode == 0x0000) {
		dest = bmBuf;
		for (y = 0; y < h; y++)
			for (x = 0; x < w; x++, dest++) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					*dest = c;
				}
		}
	else if (mode == (INT16) 0x4000) {
		dest = bmBuf + h - 1;
		for (y = 0; y < h; y++, dest--)
			for (x = 0, dest2 = dest; x < w; x++, dest2 += w) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					*dest2 = c;
				}
		}
	else if (mode == (INT16) 0x8000) {
		dest = bmBuf + s - 1;
		for (y = 0; y < h; y++)
			for (x = 0; x < w; x++, dest--) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					*dest = c;
				}
		}
	else if (mode == (INT16) 0xC000) {
		dest = bmBuf + (h - 1) * w;
		for (y = 0; y < h; y++, dest++)
			for (x = 0, dest2 = dest; x < w; x++, dest2 -= w) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					*dest2 = c;
				}
		}
	} 
else {
	if (mode == 0x0000) {
		for (y = 0; y < h; y++) {
			y1 = ((y + y0) % h) * w;
			for (x = 0; x < w; x++) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					bmBuf [y1 + (x + x0) % w] = c;
				}
			}
		}
	else if (mode == (INT16) 0x4000) {
		for (y = h - 1; y >= 0; y--)
			for (x = 0; x < w; x++) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					bmBuf [((x + y0) % h) * w + (y + x0) % w] = c;
				}
		}
	else if (mode == (INT16) 0x8000) {
		for (y = h - 1; y >= 0; y--) {
			y1 = ((y + y0) % h) * w;
			for (x = w - 1; x >= 0; x--) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					bmBuf [y1 + (x + x0) % w] = c;
				}
			}
		}
	else if (mode == (INT16) 0xC000) {
		for (y = 0; y < h; y++)
			for (x = w - 1; x >= 0; x--) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					bmBuf [((x + y0) % h) * w + (y + x0) % w] = c;
				}
			}
	}
return 0;
}

//------------------------------------------------------------------------
// ReadTextureFromFile ()
//
// ACTION - defines a bitmap from a descent PIG fTextures.
//
// INPUTS - index:	no. of texture within pig file
//          fTextures:	pointer to pig file stream
//			mode:	bit16/15=position of orgin
//                  00=upper left   01=upper right
//                  10=lower right  11=lower left
//
// Changes - Y axis flipped to since DIBs have zero in lower left corner
//------------------------------------------------------------------------

int CDTexture::Read (INT16 index) 
{
	UINT8				xsize[200];
	UINT8				line[320],*line_ptr;
	UINT8				j;
	PIG_TEXTURE		ptexture;
	D2_PIG_TEXTURE d2_ptexture;
	PIG_HEADER		file_header;
	D2_PIG_HEADER	d2_file_header;
	INT32				offset,data_offset;
	INT16				x,y,w,h,s;
	UINT8				byte,runcount,runvalue;
	INT32				nSize;
	INT16				linenum;
	HRSRC				hFind = 0;
	HGLOBAL			hGlobal = 0;
	INT16				*texture_table;
	int				rc;
	INT16				*texture_ptr;
	FILE				*fTextures = NULL;
	char				path [256];
	
if (m_bModified)
	return 0;
strcpy (path, (file_type == RDL_FILE) ? descent_path : descent2_path);
if (!strstr (path, ".pig"))
	strcat (path, "groupa.pig");

HINSTANCE hInst = AfxGetInstanceHandle ();

// do a range check on the texture number
if ((index > ((file_type == RDL_FILE) ? MAX_D1_TEXTURES:MAX_D2_TEXTURES)) || (index < 0)) {
DEBUGMSG (" Reading texture: Texture # out of range.");
rc = 1;
goto abort;
}

// get pointer to texture table from resource fTextures
hFind = (file_type == RDL_FILE) ?
	FindResource (hInst,MAKEINTRESOURCE (IDR_TEXTURE_DAT), "RC_DATA") : 
	FindResource (hInst,MAKEINTRESOURCE (IDR_TEXTURE2_DAT), "RC_DATA");
if (!hFind) {
	DEBUGMSG (" Reading texture: Texture resource not found.");
	rc = 1;
	goto abort;
	}
hGlobal = LoadResource (hInst, hFind);
if (!hGlobal) {
	DEBUGMSG (" Reading texture: Could not load texture resource.");
	rc = 2;
	goto abort;
	}
// first long is number of textures
texture_ptr = (INT16 *)LockResource (hGlobal);
texture_table = texture_ptr + 2;

fTextures = fopen (path,"rb");
if (!fTextures) {
	DEBUGMSG (" Reading texture: Texture file not found.");
	rc = 1;
	goto abort;
	}

// read fTextures header
fseek (fTextures,0,SEEK_SET);
data_offset = read_INT32 (fTextures);
if (data_offset == 0x47495050L) /* 'PPIG' Descent 2 type */
	data_offset = 0;
else if (data_offset < 0x10000L)
	data_offset = 0;
fseek (fTextures,data_offset,SEEK_SET);
if (file_type == RL2_FILE)
	fread (&d2_file_header, sizeof (d2_file_header), 1, fTextures);
else
	fread (&file_header, sizeof (file_header), 1, fTextures);

// read texture header
if (file_type == RL2_FILE) {
	offset = sizeof (D2_PIG_HEADER) + data_offset +
				(long) (texture_table[index]-1) * sizeof (D2_PIG_TEXTURE);
	fseek (fTextures,offset,SEEK_SET);
	fread (&d2_ptexture, sizeof (D2_PIG_TEXTURE), 1, fTextures);
	w = d2_ptexture.xsize + ((d2_ptexture.wh_extra & 0xF) << 8);
	h = d2_ptexture.ysize + ((d2_ptexture.wh_extra & 0xF0) << 4);
	}
else {
	offset = sizeof (PIG_HEADER) + data_offset + 
				(long) (texture_table[index]-1) * sizeof (ptexture);
	fseek (fTextures,offset,SEEK_SET);
	fread (&ptexture, sizeof (PIG_TEXTURE), 1, fTextures);
	// copy d1 texture into d2 texture struct
	strncpy (d2_ptexture.name,ptexture.name,sizeof (ptexture.name));
	d2_ptexture.dflags = ptexture.dflags;
	d2_ptexture.xsize = ptexture.xsize;
	d2_ptexture.ysize = ptexture.ysize;
	d2_ptexture.flags = ptexture.flags;
	d2_ptexture.avg_color = ptexture.avg_color;
	d2_ptexture.offset = ptexture.offset;
	d2_ptexture.wh_extra = (ptexture.dflags == 128) ? 1 : 0;
	w = h = 64;
	}
s = w * h;

// seek to data
if (file_type == RL2_FILE) {
	offset = sizeof (D2_PIG_HEADER) + data_offset
	+ d2_file_header.num_textures * sizeof (D2_PIG_TEXTURE)
	+ d2_ptexture.offset;
	}
else {
	offset =  sizeof (PIG_HEADER) + data_offset
	+ file_header.number_of_textures * sizeof (PIG_TEXTURE)
	+ file_header.number_of_sounds   * sizeof (PIG_SOUND)
	+ d2_ptexture.offset;
	}

// allocate data if necessary
if (m_pDataBM && ((m_width != w) || (m_height != h))) {
	delete m_pDataBM;
	m_pDataBM = NULL;
	}
if (m_pDataTGA) {
	delete m_pDataTGA;
	m_pDataTGA = NULL;
	}
if (m_pDataBM == NULL)
	m_pDataBM = new UINT8 [s];
if (m_pDataBM == NULL) {
	rc = 1;
	goto abort;
	}
m_nFormat = 0;
fseek (fTextures,offset,SEEK_SET);
if (d2_ptexture.flags & 0x08) {
	fread (&nSize,1,sizeof (INT32),fTextures);
	fread (xsize,d2_ptexture.ysize,1,fTextures);
	linenum = 0;
	for (y=h-1;y>=0;y--) {
		fread (line,xsize[linenum++],1,fTextures);
		line_ptr = line;
			for (x=0;x<w;) {
			byte = *line_ptr++;
			if ((byte & 0xe0) == 0xe0) {
				runcount = byte & 0x1f;
				runvalue = *line_ptr++;
				for (j=0;j<runcount;j++) {
					if (x<w) {
						m_pDataBM[y*w+x] = runvalue;
						x++;
						}
					}
				}
			else {
				m_pDataBM[y*w+x] = byte;
				x++;
				}
			}
		}
	}
else {
	for (y=h-1;y>=0;y--) {
#if 1
		fread (m_pDataBM + y * w, w, 1, fTextures);
#else
		fread (line,w,1,fTextures);
		line_ptr = line;
		for (x=0;x<w;x++) {
			byte = *line_ptr++;
			m_pDataBM[y*w+x] = byte;
			}
#endif
		}
	}
fclose (fTextures);
m_width = w;
m_height = h;
m_size = s;
m_bValid = 1;
return (0);

abort:
// free handle
if (hGlobal) FreeResource (hGlobal);
return (rc);
}

//------------------------------------------------------------------------

double CDTexture::Scale (INT16 index)
{
if (!m_width)
	if (index < 0)
		return 1.0;
	else
		Read (index);
return m_width ? (double) m_width / 64.0 : 1.0;
}

//------------------------------------------------------------------------------

int rle_expand (D2_PIG_TEXTURE *bmP, UINT8 *texBuf)
{
#	define RLE_CODE			0xE0
#	define NOT_RLE_CODE		31
#	define IS_RLE_CODE(x)	(((x) & RLE_CODE) == RLE_CODE)


	UINT8				*expandBuf, *pSrc, *pDest;
	UINT8				c, h;
	int				i, j, l, bufSize, bBigRLE;
	unsigned short nLineSize;

bufSize = bmP->xsize * bmP->ysize;
if (!(bmP->flags & BM_FLAG_RLE))
	return bufSize;
if (!(expandBuf = (UINT8 *) malloc (bufSize)))
	return -1;

bBigRLE = (bmP->flags & BM_FLAG_RLE_BIG) != 0;
if (bBigRLE)
	pSrc = texBuf + 4 + 2 * bmP->ysize;
else
	pSrc = texBuf + 4 + bmP->ysize;
pDest = expandBuf;
for (i = 0; i < bmP->ysize; i++, pSrc += nLineSize) {
	if (bBigRLE)
		nLineSize = *((unsigned short *) (texBuf + 4 + 2 * i));
	else
		nLineSize = texBuf [4 + i];
	for (j = 0; j < nLineSize; j++) {
		h = pSrc [j];
		if (!IS_RLE_CODE (h)) {
			c = h; // translate
			if (pDest - expandBuf >= bufSize) {
				free (expandBuf);
				return -1;
				}	
			*pDest++ = c;
			}
		else if (l = (h & NOT_RLE_CODE)) {
			c = pSrc [++j];
			if ((pDest - expandBuf) + l > bufSize) {
				free (expandBuf);
				return -1;
				}	
			memset (pDest, c, l);
			pDest += l;
			}
		}
	}
l = (int) (pDest - expandBuf);
memcpy (texBuf, expandBuf, l);
bmP->flags &= ~(BM_FLAG_RLE | BM_FLAG_RLE_BIG);
free (expandBuf);
return l;
}

//-----------------------------------------------------------------------------
// ReadPog ()
//-----------------------------------------------------------------------------

int ReadPog (FILE *fTextures, UINT32 nFileSize) 
{
	D2_PIG_HEADER	d2_file_header;
	D2_PIG_TEXTURE d2texture;

	HRSRC			hFind = 0;
	HGLOBAL		hGlobal = 0;
	UINT32		*n_textures = 0;
	UINT16		*texture_table;
	UINT16		texture1;
	UINT16		*xlatTbl = NULL;
	int			tWidth, tHeight, tSize;
	UINT32		offset, hdrOffset, bmpOffset, hdrSize, xlatTblSize;
	int			rc; // return code;
	int			tnum;
	UINT8			*ptr;
	int			row;
	UINT16		nUnknownTextures, nMissingTextures;
	bool			bExtraTexture;
	CDTexture	*pTx;

// make sure this is descent 2 fTextures
if (file_type == RDL_FILE) {
	INFOMSG (" Descent 1 does not support custom textures.");
	rc = 1;
	goto abort;
	}

//--------------------------------------------------------------------
// first open the texture table
//--------------------------------------------------------------------
// get pointer to texture table from resource fTextures
//	hFind	 = FindResource (hInst,"TEXTURE_DATA2", "RC_DATA");
hFind = FindResource (hInst,MAKEINTRESOURCE (IDR_TEXTURE2_DAT), "RC_DATA");
if (!hFind) {
	DEBUGMSG (" POG manager: Texture resource not found.");
	rc = 2;
	goto abort;
	}
hGlobal = LoadResource (hInst, hFind);
if (!hGlobal) {
	DEBUGMSG (" POG manager: Could not load texture resource from pog file.");
	rc = 3;
	goto abort;
	}
n_textures = (UINT32 *)LockResource (hGlobal);
texture_table = (UINT16 *) (n_textures + 1);     // first long is number of textures

// read file header
fread (&d2_file_header, sizeof (D2_PIG_HEADER), 1, fTextures);
if (d2_file_header.signature != 0x474f5044L) {  // 'DPOG'
	ErrorMsg ("Invalid pog file - reading from hog file");
	rc = 4;
	goto abort;
	}
FreeTextureHandles ();
sprintf (message," Pog manager: Reading %d custom textures",d2_file_header.num_textures);
DEBUGMSG (message);
xlatTbl = new UINT16 [d2_file_header.num_textures];
if (!xlatTbl) {
	rc = 5;
	goto abort;
	}
xlatTblSize = d2_file_header.num_textures * sizeof (UINT16);
offset = ftell (fTextures);
fread (xlatTbl, xlatTblSize, 1, fTextures);
// loop for each custom texture
nUnknownTextures = 0;
nMissingTextures = 0;
hdrOffset = offset + xlatTblSize;
hdrSize = xlatTblSize + d2_file_header.num_textures * sizeof (D2_PIG_TEXTURE);
bmpOffset = offset + hdrSize;
for (tnum = 0; tnum < d2_file_header.num_textures; tnum++) {
	// read texture index
	UINT16 texture_index = xlatTbl [tnum];
	// look it up in the list of textures
	for (texture1 = 0; texture1 < MAX_D2_TEXTURES; texture1++)
		if (texture_table [texture1] == texture_index)
			break;
	bExtraTexture = (texture1 >= MAX_D2_TEXTURES);
	// get texture data offset from texture header
#if 1
	fseek (fTextures, hdrOffset + tnum * sizeof (D2_PIG_TEXTURE), SEEK_SET);
#endif
	if (fread (&d2texture, sizeof (D2_PIG_TEXTURE), 1, fTextures) != 1)
		break;
	tWidth = d2texture.xsize + ((d2texture.wh_extra & 0xF) << 8);
	if ((d2texture.flags & 0x80) && (tWidth > 256))
		tHeight = (int) d2texture.ysize * tWidth;
	else
		tHeight = d2texture.ysize + ((d2texture.wh_extra & 0xF0) << 4);
//	if (tWidth != 512 || tHeight != 512)
//		continue;
	tSize = tWidth * tHeight;
	if (hdrSize + d2texture.offset + tSize >= nFileSize) {
		nMissingTextures++;
		continue;
		}
	if (bExtraTexture) {
		pExtraTexture	pxTx = (pExtraTexture) malloc (sizeof (*pxTx));
		if (!pxTx) {
			nUnknownTextures++;
			continue;
			}
		pxTx->pNext = extraTextures;
		extraTextures = pxTx;
		pxTx->texture_index = texture_index;
		pxTx->texture.m_pDataBM = NULL;
		pTx = &(pxTx->texture);
		texture1 = 0;
		}
	else
		pTx = pTextures [file_type];
// allocate memory for texture if not already
	ptr = (UINT8*) malloc (tWidth * tHeight);
	if (ptr) {
		if (pTx [texture1].m_pDataBM)
			delete pTx [texture1].m_pDataBM;
		pTx [texture1].m_pDataBM = ptr;
		if (d2texture.flags & 0x80) {
			ptr = (UINT8*) malloc (tSize * sizeof (tRGBA));
			if (ptr) {
				pTx [texture1].m_pDataTGA = (tRGBA *) ptr;
				pTx [texture1].m_nFormat = 1;
				}
			else {
				delete pTx [texture1].m_pDataBM;
				continue;
				}
			}
		else
			pTx [texture1].m_nFormat = 0;
		pTx [texture1].m_width = tWidth;
		pTx [texture1].m_height = tHeight;
		pTx [texture1].m_size = tSize;
		pTx [texture1].m_bValid = 1;
		// read texture into memory (assume non-compressed)
#if 1
		fseek (fTextures, bmpOffset + d2texture.offset, SEEK_SET);
#endif
		if (pTx [texture1].m_nFormat) {
			fread (ptr, pTx [texture1].m_size * sizeof (tRGBA), 1, fTextures);
			pTx [texture1].m_bValid = 
				TGA2Bitmap (pTx [texture1].m_pDataTGA, pTx [texture1].m_pDataBM, (int) tWidth, (int) tHeight);
			}
		else {
			if (d2texture.flags & BM_FLAG_RLE) {
				fread (ptr, tSize, 1, fTextures);
				rle_expand (&d2texture, ptr);
				}
			else {
				UINT8 *p = ptr + tWidth * (tHeight - 1); // point to last row of bitmap
				for (row = 0; row < tHeight; row++) {
					fread (p, tWidth, 1, fTextures);
					p -= tWidth;
					}
				}
			}
		if (!bExtraTexture)
			pTx [texture1].m_bModified = TRUE;
		}
	}
if (nUnknownTextures) {
	sprintf (message, " Pog manager: %d unknown textures found.", nUnknownTextures);
	DEBUGMSG (message);
	}
if (nMissingTextures) {
	sprintf (message, " Pog manager: %d textures missing (Pog file probably damaged).", nMissingTextures);
	DEBUGMSG (message);
	}

rc = 0;

abort:

if (xlatTbl)
	delete xlatTbl;
if (n_textures) 
	GlobalUnlock (hGlobal);  // no need to unlock it but what the heck
if (hGlobal) 
	FreeResource (hGlobal);
return rc;
}

//-----------------------------------------------------------------------------

void WritePogTextureHeader (FILE *pDestPigFile, CDTexture *pTexture, int nTexture, UINT32& nOffset)
{
	D2_PIG_TEXTURE d2texture;
	UINT8 *pSrc;

sprintf (d2texture.name, "new%04d", nTexture);
d2texture.dflags = 0;
d2texture.flags = pTexture->m_nFormat ? 0x80 : 0;
d2texture.xsize = pTexture->m_width % 256;
if ((d2texture.flags & 0x80) && (pTexture->m_width > 256)) {
	d2texture.wh_extra = (pTexture->m_width >> 8);
	d2texture.ysize = pTexture->m_height / pTexture->m_width;
	}
else {
	d2texture.ysize = pTexture->m_height % 256;
	d2texture.wh_extra = (pTexture->m_width >> 8) | ((pTexture->m_height >> 4) & 0xF0);
	}
d2texture.avg_color = 0;
d2texture.offset = nOffset;
nOffset += (d2texture.flags & 0x80) ? pTexture->m_size * 4: pTexture->m_size;

// check for transparency and super transparency
if (!pTexture->m_nFormat)
	if (pSrc = (UINT8 *) pTexture->m_pDataBM) {
		int j;
		for (j = 0; j < pTexture->m_size; j++, pSrc++) {
			if (*pSrc == 255) 
				d2texture.flags |= BM_FLAG_TRANSPARENT;
			if (*pSrc == 254) 
				d2texture.flags |= BM_FLAG_SUPER_TRANSPARENT;
			}
	}
fwrite (&d2texture, sizeof (D2_PIG_TEXTURE), 1, pDestPigFile);
}

//-----------------------------------------------------------------------------

bool WritePogTexture (FILE *pDestPigFile, CDTexture *pTexture)
{
	UINT8		*pSrc = pTexture->m_nFormat ? (UINT8*) pTexture->m_pDataTGA : pTexture->m_pDataBM;

if (!pSrc) {
	DEBUGMSG (" POG manager: Couldn't lock texture data.");
	return false;
	}
if (pTexture->m_nFormat) {
#if 0
	int w = pTexture->m_width;
	int h = pTexture->m_height;
	tBGRA	bgra;

	pSrc += w * (h - 1) * 4;
	int i, j;
	for (i = h; i; i--) {
		for (j = w; j; j--) {
			bgra.b = *pSrc++;
			bgra.g = *pSrc++;
			bgra.r = *pSrc++;
			bgra.a = *pSrc++;
			fwrite (&bgra, sizeof (bgra), 1, pDestPigFile);
			}
		pSrc -= 2 * w * 4;
		}
#else
	fwrite (pSrc, pTexture->m_width * pTexture->m_height * sizeof (tRGBA), 1, pDestPigFile);
#endif
	}
else {
	UINT16 w = pTexture->m_width;
	UINT16 h = pTexture->m_height;
	pSrc += w * (h - 1); // point to last row of bitmap
	int row;
	for (row = 0; row < h; row++) {
		fwrite (pSrc, w, 1, pDestPigFile);
		pSrc -= w;
		}
	}
return true;
}

//-----------------------------------------------------------------------------
// CreatePog ()
//
// Action - Creates a POG fTextures from all the changed textures
//
// Format:
//   Pig Header
//   Texture Indicies (N total * UINT16)
//   Texture Header 0
//     ...
//   Texture Header N
//   Texture 0
//     ...
//   Texture N
//
// where N is the number of textures.
//
// Uncommpressed Texture data:
//   UINT8  data[data_size];    // raw data
//
// Commpressed Texture data:
//   UINT32 total_size;      // including this long word and everything below
//   UINT8  line_sizes[64];  // size of each line (in bytes)
//   UINT8  data[data_size]; // run length encoded (RLE) data
//
// RLE definition:
//
//   If upper 3 bits of byte are set, lower 5 bytes represents how many times
//   to repeat the following byte.  The last byte of the data must be set
//   to 0xE0, which means 0 bytes to follow.  Note: The last byte may be
//   omitted since the xsize array defines the length of each line???
//
//-----------------------------------------------------------------------------
int CreatePog (FILE *outPigFile) 
{
#if DEMO == 0
  int rc; // return code;
  D2_PIG_HEADER d2_file_header;
  HRSRC hFind = 0;
  HGLOBAL hGlobal = 0;
  UINT32 *n_textures = 0, nOffset = 0;
  UINT16 *texture_table;
  int i;
  int num;
  pExtraTexture	pxTx;

if (file_type == RDL_FILE) {
	ErrorMsg ("Descent 1 does not support custom textures.");
	rc = 4;
	goto abort;
	}

//--------------------------------------------------------------------
// first open the texture table
//--------------------------------------------------------------------
// get pointer to texture table from resource fTextures
hFind   = FindResource (hInst,MAKEINTRESOURCE (IDR_TEXTURE2_DAT), "RC_DATA");
if (!hFind) {
	DEBUGMSG (" Reading texture: Texture resource not found.");
	rc = 5;
	goto abort;
	}
hGlobal = LoadResource (hInst, hFind);
if (!hGlobal) {
	DEBUGMSG (" Pog manager: Could not load texture resource.");
	rc = 6;
	goto abort;
	}
n_textures = (UINT32 *)LockResource (hGlobal);
texture_table = (UINT16 *) (n_textures + 1);     // first long is number of textures

sprintf (message,"%s\\dle_temp.pog",starting_directory);

// write file  header
d2_file_header.signature    = 0x474f5044L; /* 'DPOG' */
d2_file_header.version      = 0x00000001L;
d2_file_header.num_textures = 0;
for (i=0;i<MAX_D2_TEXTURES;i++)
	if (pTextures [file_type][i].m_bModified) {
		d2_file_header.num_textures++;
		}
for (pxTx = extraTextures; pxTx; pxTx = pxTx->pNext)
	d2_file_header.num_textures++;
fwrite (&d2_file_header, sizeof (D2_PIG_HEADER), 1, outPigFile);

// write list of textures
for (i=0;i<MAX_D2_TEXTURES;i++)
	if (pTextures [file_type][i].m_bModified)
		fwrite (texture_table + i, sizeof (UINT16), 1, outPigFile);

for (pxTx = extraTextures; pxTx; pxTx = pxTx->pNext)
	fwrite (&pxTx->texture_index, sizeof (UINT16), 1, outPigFile);

// write texture headers
num = 0;
for (i = 0; i < MAX_D2_TEXTURES; i++)
	if (pTextures [file_type][i].m_bModified)
		WritePogTextureHeader (outPigFile, pTextures [file_type] + i, num++, nOffset);
for (pxTx = extraTextures; pxTx; pxTx = pxTx->pNext, num++)
	WritePogTextureHeader (outPigFile, &pxTx->texture, num, nOffset);

sprintf (message," Pog manager: Saving %d custom textures",d2_file_header.num_textures);
DEBUGMSG (message);

//-----------------------------------------
// write textures (non-compressed)
//-----------------------------------------
rc = 8;
for (i=0;i<MAX_D2_TEXTURES;i++)
	if (pTextures [file_type][i].m_bModified && 
		 !WritePogTexture (outPigFile, pTextures [file_type] + i))
		goto abort;
for (pxTx = extraTextures; pxTx; pxTx = pxTx->pNext)
	if (!WritePogTexture (outPigFile, &pxTx->texture))
		goto abort;

rc = 0; // return success

abort:
if (outPigFile) 
	fclose (outPigFile);
if (n_textures) 
	GlobalUnlock (hGlobal);  // no need to unlock it but what the heck
if (hGlobal) 
	FreeResource (hGlobal);
return rc;
#else //DEMO
return 0;
#endif //DEMO
}

//------------------------------------------------------------------------
// Free Texture Handles
//------------------------------------------------------------------------

void FreeTextureHandles (bool bDeleteModified) 
{
  // free any textures that have been buffered
int i, j;
for (j = 0; j < 2; j++)
	for (i = 0; i < MAX_D2_TEXTURES; i++) {
		if (!bDeleteModified && pTextures [j][i].m_bModified)
			continue;
		if (pTextures [j][i].m_pDataBM) {
			free (pTextures [j][i].m_pDataBM);
			pTextures [j][i].m_pDataBM = NULL;
			}
		if (pTextures [j][i].m_pDataTGA) {
			free (pTextures [j][i].m_pDataTGA);
			pTextures [j][i].m_pDataTGA = NULL;
			}
		pTextures [file_type][i].m_bModified = FALSE;
		pTextures [file_type][i].m_nFormat = 0;
		}
pExtraTexture	p;
while (extraTextures) {
	p = extraTextures;
	extraTextures = p->pNext;
	free (p);
	}
}



BOOL HasCustomTextures () 
{
int i;
for (i = 0; i < MAX_D2_TEXTURES; i++)
	if (pTextures [file_type][i].m_bModified)
		return TRUE;
return FALSE;
}



int CountCustomTextures () 
{
	int i, count = 0;

for (i = 0; i < MAX_D2_TEXTURES; i++)
	if (pTextures [file_type][i].m_bModified)
		count++;
return count;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void RgbFromIndex (int nIndex, PALETTEENTRY *pRGB)
{
if (pRGB) {
	HRSRC hFind = FindResource (hInst, PaletteResource (), "RC_DATA");
	if (hFind) {
		HGLOBAL hPalette = LoadResource (hInst, hFind);
		UINT8 *palette = ((UINT8 *) LockResource (hPalette)) + 3 * nIndex;
		pRGB->peRed = (*palette++)<<2;
		pRGB->peGreen = (*palette++)<<2;
		pRGB->peBlue = (*palette)<<2;
		pRGB->peFlags = 0;
		}
	}
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------


BITMAPINFO *MakeBitmap (void) 
{
	BITMAPINFO *bmi;
	BITMAPINFOHEADER *bi;
	INT16 i;
	HRSRC hFind;
	HGLOBAL hPalette;
	UINT8 *palette;

	typedef struct tagMyBMI {
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD bmiColors[256];
		} MyBMI;

static MyBMI my_bmi;

	// point bitmap info to structure
	bmi = (BITMAPINFO *)&my_bmi;

	// point the info to the bitmap header structure
	bi = &bmi->bmiHeader;

	// define bit map info header elements
	bi->biSize          = sizeof (BITMAPINFOHEADER);
	bi->biWidth         = 64;
	bi->biHeight        = 64;
	bi->biPlanes        = 1;
	bi->biBitCount      = 8;
	bi->biCompression   = BI_RGB;
	bi->biSizeImage     = 0;
	bi->biXPelsPerMeter = 0;
	bi->biYPelsPerMeter = 0;
	bi->biClrUsed       = 0;
	bi->biClrImportant  = 0;

	// define d1 palette from resource
	hFind = FindResource (hInst, PaletteResource (), "RC_DATA");
	if (!hFind) {
		DEBUGMSG (" Bitmap creation: Palette resource not found.");
		return NULL;
		}
	hPalette = LoadResource (hInst, hFind);
	palette = (UINT8 *) LockResource (hPalette);
#if 0
	for (i=0;i<256;i++) {
	  bmi->bmiColors[i].rgbRed      = *palette++;
	  bmi->bmiColors[i].rgbGreen    = *palette++;
	  bmi->bmiColors[i].rgbBlue     = *palette++;
	  bmi->bmiColors[i].rgbReserved = 0;palette++;
	}
#else
	for (i=0;i<256;i++) {
	  bmi->bmiColors[i].rgbRed    = (*palette++)<<2;
	  bmi->bmiColors[i].rgbGreen  = (*palette++)<<2;
	  bmi->bmiColors[i].rgbBlue   = (*palette++)<<2;
	  bmi->bmiColors[i].rgbReserved = 0;
	}
#endif
	FreeResource (hPalette);
  return bmi;
}
// -------------------------------------------------------------------------- 
// -------------------------------------------------------------------------- 

bool PaintTexture (CWnd *pWnd, int bkColor, int segnum, int sidenum, int texture1, int texture2, int xOffset, int yOffset)
{
	CMine			*mine;

if (!(mine = theApp.GetMine ()))
	return false;

	CDC			*pDC = pWnd->GetDC ();

if (!pDC)
	return false;

	HINSTANCE	hInst = AfxGetApp ()->m_hInstance;
	CBitmap		bmTexture;
	FILE			*fTextures = NULL;
	INT32			data_offset;
	char			szFile [256];
	BITMAP		bm;
	CDC			memDC;
	CDSegment	*seg;
	CDSide		*side;
	INT16			nWall;
	bool			bShowTexture = true;
	char			*path = (file_type == RDL_FILE) ? descent_path : descent2_path;

CRect	rc;
pWnd->GetClientRect (rc);

if (texture1 < 0) {
	seg = (segnum < 0) ? mine->CurrSeg () : mine->Segments (segnum);
	side = (sidenum < 0) ? mine->CurrSide () : seg->sides + sidenum;
	int nSide = mine->Current ()->side;
	texture1 = side->nBaseTex;
	texture2 = side->nOvlTex & 0x1fff;
	if (seg->children [nSide] == -1)
		bShowTexture = TRUE;
	else {
		nWall = side->nWall;
		bShowTexture = (nWall < mine->GameInfo ().walls.count);
		}
	}
if ((texture1 < 0) || (texture1 >= MAX_TEXTURES + 10))
	bShowTexture = false;
if ((texture2 < 0) || (texture2 >= MAX_TEXTURES))	// this allows to suppress bitmap display by 
	bShowTexture = false;									// passing 0 for texture 1 and -1 for texture2

if (bShowTexture) {
	// check pig file
	strcpy (szFile, (file_type == RDL_FILE) ? descent_path : descent2_path);
	if (fTextures = fopen (szFile,"rb")) {
		fseek (fTextures, 0, SEEK_SET);
		data_offset = read_INT32 (fTextures);  // determine type of pig file
		fclose (fTextures);
		}
	else
		data_offset = -1;  // pig file not found
	if (data_offset > 0x10000L) {  // pig file type is v1.4a or descent 2 type
		CDTexture	tx (bmBuf);
		if (DefineTexture (texture1, texture2, &tx, xOffset, yOffset))
			DEBUGMSG (" Texture renderer: Texture not found (DefineTexture failed)");
		CPalette *pOldPalette = pDC->SelectPalette (thePalette, FALSE);
		pDC->RealizePalette ();
		int caps = pDC->GetDeviceCaps (RASTERCAPS);
		if (caps & RC_DIBTODEV) {
			BITMAPINFO *bmi = MakeBitmap ();
			if (!bmi)
				return false;
			bmi->bmiHeader.biWidth = 
			bmi->bmiHeader.biHeight = tx.m_width;
			StretchDIBits (pDC->m_hDC, 0, 0, rc.Width (), rc.Height (), 0, 0, tx.m_width, tx.m_width,
					        	 (void *) bmBuf, bmi, DIB_RGB_COLORS, SRCCOPY);
			}
		else {
			double scale = tx.Scale ();
			int x, y;
			for (x = 0; x < tx.m_width; x = (int) (x + scale))
				for (y = 0; y < tx.m_width; y = (int) (y + scale))
					pDC->SetPixel ((INT16) (x / scale), (INT16) (y / scale), PALETTEINDEX (bmBuf [y*tx.m_width+x]));
			}
		pDC->SelectPalette (pOldPalette, FALSE);
		}
	else {
		HGDIOBJ hgdiobj1;
		bmTexture.LoadBitmap ((data_offset < 0) ? "NO_PIG_BITMAP" : "WRONG_PIG_BITMAP");
		bmTexture.GetObject (sizeof (BITMAP), &bm);
		memDC.CreateCompatibleDC (pDC);
		hgdiobj1 = memDC.SelectObject (bmTexture);
		pDC->StretchBlt (0, 0, rc.Width (), rc.Height (), &memDC, 0, 0, 64, 64, SRCCOPY);
		memDC.SelectObject (hgdiobj1);
		DeleteObject (bmTexture);
//		DeleteDC (memDC.m_hDC);
		}
	}
else if (bkColor < 0) {
	HGDIOBJ hgdiobj;
	// set bitmap
	bmTexture.LoadBitmap ("NO_TEXTURE_BITMAP");
	bmTexture.GetObject (sizeof (BITMAP), &bm);
	memDC.CreateCompatibleDC (pDC);
	hgdiobj = memDC.SelectObject (bmTexture);
	pDC->StretchBlt (0, 0, rc.Width (), rc.Height (), &memDC, 0, 0, 64, 64, SRCCOPY);
	memDC.SelectObject (hgdiobj);
	DeleteObject (bmTexture);
//	DeleteDC (memDC.m_hDC);
	}
else if (bkColor >= 0)
	pDC->FillSolidRect (&rc, (COLORREF) bkColor);
pWnd->ReleaseDC (pDC);
pWnd->InvalidateRect (NULL, TRUE);
pWnd->UpdateWindow ();
return bShowTexture;
}
//eof textures.cpp