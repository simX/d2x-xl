#ifndef __textures_h
#define __textures_h

extern UINT8 bmBuf [2048 * 2048 * 4];

typedef struct {
    byte  identSize;          // size of ID field that follows 18 byte header (0 usually)
    byte  colorMapType;      // type of colour map 0=none, 1=has palette
    byte  imageType;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

    short colorMapStart;     // first colour map entry in palette
    short colorMapLength;    // number of colours in palette
    byte  colorMapBits;      // number of bits per palette entry 15,16,24,32

    short xStart;             // image x origin
    short yStart;             // image y origin
    short width;              // image width in pixels
    short height;             // image height in pixels
    byte  bits;               // image bits per pixel 8,16,24,32
    byte  descriptor;         // image descriptor bits (vh flip bits)
} tTgaHeader;


typedef struct tRGBA {
	unsigned char	r, g, b, a;
} tRGBA;


typedef struct tBGRA {
	unsigned char	b, g, r, a;
} tBGRA;

typedef struct tABGR {
	unsigned char	a, b, g, r;
} tABGR;

typedef struct tBGR {
	unsigned char	r, g, b;
} tBGR;

class CDTexture
{
public:
	UINT8		*m_pDataBM;
	tRGBA		*m_pDataTGA;
	UINT32	m_width, m_height, m_size;
	BOOLEAN	m_bModified, m_bExtData, m_bValid;
	UINT8		m_nFormat;	// 0: Bitmap, 1: TGA (RGB)

	CDTexture(UINT8 *pData = NULL) 
		: m_pDataBM (pData) 
		{ m_pDataTGA = NULL, m_nFormat = 0, m_bValid = m_bModified = FALSE, m_bExtData = (pData != NULL); }
	~CDTexture() {
		if (!m_bExtData) {
			delete m_pDataBM;
			if (m_pDataTGA)
				delete m_pDataTGA;
			}
		}

	int Read (INT16 index);
	double Scale (INT16 index = -1);
};


int DefineTexture(INT16 nBaseTex,INT16 nOvlTex, CDTexture *pDestTx, int x0, int y0);
void RgbFromIndex (int nIndex, PALETTEENTRY *pRGB);
BITMAPINFO *MakeBitmap(void);
BOOL HasCustomTextures ();
int CountCustomTextures ();
void FreeTextureHandles(bool bDeleteModified = true);
int ReadPog(FILE *file, UINT32 nFileSize = 0xFFFFFFFF);
int CreatePog (FILE *file);
bool PaintTexture (CWnd *pWnd, int bkColor = -1, 
						 int segnum = -1, int sidenum = -1, int texture1 = -1, int texture2 = 0,
						 int xOffset = 0, int yOffset = 0);
bool TGA2Bitmap (tRGBA *pTGA, UINT8 *pBM, int nWidth, int nHeight);

extern CDTexture pTextures [2][MAX_D2_TEXTURES];

#endif //__textures_h