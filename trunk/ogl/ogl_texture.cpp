/*
 *
 * Graphics support functions for OpenGL.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef _WIN32
#	include <windows.h>
#	include <stddef.h>
#	include <io.h>
#endif
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#ifdef __macosx__
# include <stdlib.h>
# include <SDL/SDL.h>
#else
# include <malloc.h>
# include <SDL.h>
#endif

#include "descent.h"
#include "error.h"
#include "u_mem.h"
#include "rle.h"
#include "ogl_defs.h"
#include "ogl_lib.h"
#include "ogl_texture.h"
#include "texmerge.h"
#include "fbuffer.h"

//------------------------------------------------------------------------------

#define TEXTURE_LIST_SIZE 5000

CTextureManager textureManager;

//------------------------------------------------------------------------------
//little hack to find the largest or equal multiple of 2 for a given number
int Pow2ize (int x)
{
	int i;

for (i = 2; i <= 4096; i *= 2)
	if (x <= i) 
		return i;
return i;
}

//------------------------------------------------------------------------------

int Luminance (int r, int g, int b)
{
	int minColor, maxColor;

if (r < g) {
	minColor = (r < b) ? r : b;
	maxColor = (g > b) ? g : b;
	}
else {
	minColor = (g < b) ? g : b;
	maxColor = (r > b) ? r : b;
	}
return (minColor + maxColor) / 2;
}

//------------------------------------------------------------------------------

void CTextureManager::Init (void)
{
m_textures.Create (TEXTURE_LIST_SIZE);
for (int i = 0; i < TEXTURE_LIST_SIZE; i++)
	m_textures [i].SetIndex (i);
}

//------------------------------------------------------------------------------

void CTextureManager::Destroy (void)
{
	CTexture*	texP;
	int		i;

OglDestroyDrawBuffer ();
cameraManager.Destroy ();
OglDeleteLists (&hBigSphere, 1);
OglDeleteLists (&hSmallSphere, 1);
OglDeleteLists (&circleh5, 1);
OglDeleteLists (&circleh10, 1);
OglDeleteLists (cross_lh, sizeof (cross_lh) / sizeof (GLuint));
OglDeleteLists (primary_lh, sizeof (primary_lh) / sizeof (GLuint));
OglDeleteLists (secondary_lh, sizeof (secondary_lh) / sizeof (GLuint));
OglDeleteLists (g3InitTMU [0], sizeof (g3InitTMU) / sizeof (GLuint));
OglDeleteLists (g3ExitTMU, sizeof (g3ExitTMU) / sizeof (GLuint));
OglDeleteLists (&mouseIndList, 1);

if (!(texP = GetFirst ()))
	return;
do {
	Push (texP);
	texP->SetHandle (0); //(GLuint) -1);
	} while ((texP = GetNext ()));

for (i = 0; i < MAX_ADDON_BITMAP_FILES; i++)
	gameData.pig.tex.addonBitmaps [i].Unlink (1);

#if DBG
// Make sure all textures (bitmaps) from the game texture lists that had an OpenGL handle get the handle reset
CBitmap* bmP;
for (i = 0; i < 2; i++) {
	bmP = gameData.pig.tex.bitmaps [i].Buffer ();
	for (int j = ((i || !gameData.hoard.nBitmaps) ? gameData.pig.tex.nBitmaps [i] : gameData.hoard.nBitmaps); j; j--, bmP++)
		bmP->Unlink (0);
	}
#endif
}

//------------------------------------------------------------------------------

CTexture *CTextureManager::Get (CBitmap *bmP)
{
CTexture *texP = Pop ();

if (texP) 
	texP->SetBitmap (bmP);
else {
#if DBG
	Warning ("OGL: texture list full!\n");
#endif
	// try to recover: flush all textures, reload fonts and this level's textures
	RebuildRenderContext (gameStates.app.bGameRunning);
	if ((texP = Pop ()))
		texP->SetBitmap (bmP);
	}
return texP;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CTexture::Init (void)
{
m_info.handle = 0;
m_info.internalFormat = 4;
m_info.format = gameStates.ogl.nRGBAFormat;
m_info.w =
m_info.h = 0;
m_info.tw =
m_info.th = 0;
m_info.lw = 0;
m_info.bMipMaps = 0;
m_info.bSmoothe = 0;
m_info.u = 
m_info.v = 0;
m_info.bRenderBuffer = 0;
m_info.prio = 0.3f;
m_info.bmP = NULL;
}

//------------------------------------------------------------------------------

void CTexture::Setup (int w, int h, int lw, int bpp, int bMask, int bMipMap, int bSmoothe, CBitmap *bmP)
{
m_info.w = w;
m_info.h = h;
m_info.lw = lw;
m_info.tw = Pow2ize (w);
m_info.th = Pow2ize (h);
if (bMask) {
	m_info.format = GL_RED;
	m_info.internalFormat = 1;
	}
else if (bpp == 3) {
	m_info.format = GL_RGB;
#if TEXTURE_COMPRESSION
	if ((w > 64) && (w == h))
		m_info.internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
	else
#endif
	m_info.internalFormat = 3;
	}
else {
	m_info.format = GL_RGBA;
#if TEXTURE_COMPRESSION
	if ((w > 64) && (w == h))
		m_info.internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	else
#endif
	m_info.internalFormat = 4;
	}
m_info.bMipMaps = bMipMap && !bMask;
m_info.bSmoothe = bSmoothe || m_info.bMipMaps;
m_info.bmP = bmP;
}

//------------------------------------------------------------------------------

#if RENDER2TEXTURE == 1

void CTexture::SetRenderBuffer (CPBO* pbo)
{
if (!pbo)
	m_info.bRenderBuffer = 0;
else {
	m_info.pbo = *pbo;
	m_info.handle = pbo->TexId ();
	m_info.bRenderBuffer = 1;
	}
}

#elif RENDER2TEXTURE == 2

void CTexture::SetRenderBuffer (CFBO* fbo)
{
if (!fbo)
	m_info.bRenderBuffer = 0;
else {
	m_info.fbo = *fbo;
	m_info.handle = fbo->RenderBuffer ();
	m_info.bRenderBuffer = 1;
	}
}

#endif

//------------------------------------------------------------------------------

int CTexture::BindRenderBuffer (void)
{
#if RENDER2TEXTURE == 1
#	if 1
OGL_BINDTEX (m_info.pbuffer.texId);
#		if 1
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR /*GL_LINEAR_MIPMAP_LINEAR*/);
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#		else
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#		endif
OGL_BINDTEX (m_info.pbuffer.texId);
#	endif
#	ifdef _WIN32
#		if DBG
if (!m_info.pbo.Bind ()) {
	char *psz = reinterpret_cast<char*> (gluErrorString (glGetError ())));
	return 1;
	}
#		else
if (!(m_info.pbo.Bind ())
	return 1;
#		endif
#	endif
#elif RENDER2TEXTURE == 2
OGL_BINDTEX (m_info.fbo.RenderBuffer ());
#endif
return 0;
}

//------------------------------------------------------------------------------
// Create an image buffer for the renderer from a palettized bitmap.
// The image buffer has power of two dimensions; the image will be stored
// in its upper left area and u and v coordinates of lower right image corner
// will be computed so that only the part of the buffer containing the image
// is rendered.

ubyte* CTexture::Convert (
//	GLubyte		*buffer,
//	int			width,
//	int			height,
	int			dxo,
	int			dyo,
//	int			tWidth,
//	int			tHeight,
//	int			nFormat,
	CBitmap		*bmP,
	int			nTransp,
	int			bSuperTransp)
{
	tRgbColorb	*colorP;
	int			x, y, c;
	ushort		r, g, b, a;
	int			bTransp, bpp;

#if DBG
if (strstr (bmP->Name (), "rock313"))
	bmP = bmP;
#endif
paletteManager.SetTexture (bmP->Parent () ? bmP->Parent ()->Palette () : bmP->Palette ());
if (!paletteManager.Texture ())
	return NULL;
bTransp = (nTransp || bSuperTransp) && bmP->HasTransparency ();
if (!bTransp)
	m_info.format = GL_RGB;
#if DBG
if (!nTransp)
	nTransp = 0;
#endif
colorP = paletteManager.Texture ()->Color ();

restart:

switch (m_info.format) {
	case GL_LUMINANCE:
		bpp = 1;
		break;

	case GL_LUMINANCE_ALPHA:
	case GL_RGB5:
	case GL_RGBA4:
		bpp = 2; 
		break;

	case GL_RGB:
		bpp = 3; 
		break;

	default:
		bpp = 4;
		break;
	}

if (m_info.tw * m_info.th * bpp > (int) sizeof (gameData.render.ogl.buffer))//shouldn'texP happen, descent never uses textures that big.
	Error ("texture too big %i %i", m_info.tw, m_info.th);

ubyte* rawData = bmP->Buffer ();
GLubyte* bufP = gameData.render.ogl.buffer;
//bmP->Flags () &= ~(BM_FLAG_TRANSPARENT | BM_FLAG_SUPER_TRANSPARENT);
for (y = 0; y < m_info.h; y++) {
	for (x = 0; x < m_info.w; x++) {
#if DBG
		if (!bmP->IsElement (rawData))
			break;
#endif
		c = *rawData++;
		if (nTransp && (c == TRANSPARENCY_COLOR)) {
			//bmP->Flags () |= BM_FLAG_TRANSPARENT;
			switch (m_info.format) {
				case GL_LUMINANCE:
					(*(bufP++)) = 0;
					break;

				case GL_LUMINANCE_ALPHA:
					*reinterpret_cast<GLushort*> (bufP) = 0;
					bufP += 2; 
					break;

				case GL_RGB:
				case GL_RGB5:
					m_info.format = gameStates.ogl.nRGBAFormat;
					goto restart;
					break;

				case GL_RGBA:
					*reinterpret_cast<GLuint*> (bufP) = (nTransp ? 0 : 0xffffffff);
					bufP += 4;
					break;
				
				case GL_RGBA4:
					*reinterpret_cast<GLushort*> ( bufP) = (nTransp ? 0 : 0xffff);
					bufP += 2;
					break;
				}
//				 (*(texP++)) = 0;
			}
		else {
			switch (m_info.format) {
				case GL_LUMINANCE://these could prolly be done to make the intensity based upon the intensity of the resulting color, but its not needed for anything (yet?) so no point. :)
						(*(bufP++)) = 255;
					break;

				case GL_LUMINANCE_ALPHA:
						(*(bufP++)) = 255;
						(*(bufP++)) = 255;
					break;

				case GL_RGB:
				case GL_RGB5:
					if (bSuperTransp && (c == SUPER_TRANSP_COLOR)) {
						//bmP->Flags () |= BM_FLAG_SUPER_TRANSPARENT;
						m_info.format = gameStates.ogl.nRGBAFormat;
						goto restart;
						}
					else {
						r = colorP [c].red * 4;
						g = colorP [c].green * 4;
						b = colorP [c].blue * 4;
						if ((r < gameStates.ogl.nTransparencyLimit) && 
							 (g < gameStates.ogl.nTransparencyLimit) && 
							 (b < gameStates.ogl.nTransparencyLimit)) {
							m_info.format = gameStates.ogl.nRGBAFormat;
							goto restart;
							}
						if (m_info.format == GL_RGB) {
							(*(bufP++)) = (GLubyte) r;
							(*(bufP++)) = (GLubyte) g;
							(*(bufP++)) = (GLubyte) b;
							}
						else {
							r >>= 3;
							g >>= 2;
							b >>= 3;
							*reinterpret_cast<GLushort*> ( bufP) = r + (g << 5) + (b << 11);
							bufP += 2;
							}
						}
					break;

				case GL_RGBA:
				case GL_RGBA4: {
					if (bSuperTransp && (c == SUPER_TRANSP_COLOR)) {
						//bmP->Flags () |= BM_FLAG_SUPER_TRANSPARENT;
#if 0
						if (bShaderMerge) {
							r = g = b = 0;
							a = 1;
							}
						else
#endif
						 {
							r = 120;
							g = 88;
							b = 128;
							a = 0;
							}
						}
					else {
						r = colorP [c].red * 4;
						g = colorP [c].green * 4;
						b = colorP [c].blue * 4;
						if ((r < gameStates.ogl.nTransparencyLimit) && 
							 (g < gameStates.ogl.nTransparencyLimit) && 
							 (b < gameStates.ogl.nTransparencyLimit))
							a = 0;
						else if (nTransp == 1) {
#if 0 //non-linear formula
							double da = (double) (r * 3 + g * 5 + b * 2) / (10.0 * 255.0);
							da *= da;
							a = (ubyte) (da * 255.0);
#else
							a = (r * 3 + g * 5 + b * 2) / 10;	//transparency based on color intensity
#endif
							}
						else if (nTransp == 2)	// black is transparent
							a = c ? 255 : 0;
						else
							a = 255;	//not transparent
						}
					if (m_info.format == GL_RGBA) {
						(*(bufP++)) = (GLubyte) r;
						(*(bufP++)) = (GLubyte) g;
						(*(bufP++)) = (GLubyte) b;
						(*(bufP++)) = (GLubyte) a;
						}
					else {
						*reinterpret_cast<GLushort*> (bufP) = (r >> 4) + ((g >> 4) << 4) + ((b >> 4) << 8) + ((a >> 4) << 12);
						bufP += 2;
						}
					break;
					}
				}
			}
		}
#if DBG
memset (bufP, 0, (m_info.tw - m_info.w) * bpp);	//erase the entire unused rightmost part of the line
#else
memset (bufP, 0, bpp); //erase one pixel to compensate for rounding problems with the u coordinate
#endif
	bufP += (m_info.tw - m_info.w) * bpp;
	}
#if DBG
memset (bufP, 0, (m_info.th - m_info.h) * m_info.tw * bpp);
#else
memset (bufP, 0, m_info.tw * bpp);
#endif
if (m_info.format == GL_RGB)
	m_info.internalFormat = 3;
else if (m_info.format == GL_RGBA)
	m_info.internalFormat = 4;
else if ((m_info.format == GL_RGB5) || (m_info.format == GL_RGBA4))
	m_info.internalFormat = 2;
return gameData.render.ogl.buffer;
}

//------------------------------------------------------------------------------
//create texture buffer from data already in RGBA format

ubyte *CTexture::Copy (int dxo, int dyo, ubyte *data)
{
if (!dxo && !dyo && (m_info.w == m_info.tw) && (m_info.h == m_info.th))
	return data;	//can use data 1:1
else {	//need to reformat
	int		h, w, tw;
	GLubyte	*bufP;

	h = m_info.lw / m_info.w;
	w = (m_info.w - dxo) * h;
	data += m_info.lw * dyo + h * dxo;
	bufP = gameData.render.ogl.buffer;
	tw = m_info.tw * h;
	h = tw - w;
	for (; dyo < m_info.h; dyo++, data += m_info.lw) {
		memcpy (bufP, data, w);
		bufP += w;
		memset (bufP, 0, h);
		bufP += h;
		}
	memset (bufP, 0, m_info.th * tw - (bufP - gameData.render.ogl.buffer));
	return gameData.render.ogl.buffer;
	}
}

//------------------------------------------------------------------------------

int CTexture::FormatSupported (void)
{
	GLint nFormat = 0;

if (!gameStates.ogl.bGetTexLevelParam)
	return 1;

switch (m_info.format) {
	case GL_RGB:
		if (m_info.internalFormat == 3)
			return 1;
		break;

	case GL_RGBA:
		if (m_info.internalFormat == 4)
			return 1;
		break;

	case GL_RGB5:
	case GL_RGBA4:
		if (m_info.internalFormat == 2)
			return 1;
		break;
	
	case GL_INTENSITY4:
		if (gameStates.ogl.bIntensity4 == -1) 
			return 1; 
		if (!gameStates.ogl.bIntensity4) 
			return 0; 
		break;

	case GL_LUMINANCE4_ALPHA4:
		if (gameStates.ogl.bLuminance4Alpha4 == -1) 
			return 1; 
		if (!gameStates.ogl.bLuminance4Alpha4) 
			return 0; 
		break;
	}

glTexImage2D (GL_PROXY_TEXTURE_2D, 0, m_info.internalFormat, m_info.tw, m_info.th, 0,
				  m_info.format, GL_UNSIGNED_BYTE, gameData.render.ogl.buffer);//NULL?
glGetTexLevelParameteriv (GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &nFormat);
switch (m_info.format) {
	case GL_RGBA:
#if TEXTURE_COMPRESSION
		if ((nFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ||
			 (nFormat == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT) ||
			 (nFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT))
			nFormat = m_info.internalFormat;
		else
#endif
			m_info.internalFormat = 4;
		break;

	case GL_RGB:
			m_info.internalFormat = 3;
		break;

	case GL_RGB5:
	case GL_RGBA4:
		m_info.internalFormat = 2;
		break;
	
	case GL_INTENSITY4:
		gameStates.ogl.bIntensity4 = (nFormat == m_info.internalFormat) ? -1 : 0;
		break;

	case GL_LUMINANCE4_ALPHA4:
		gameStates.ogl.bLuminance4Alpha4 = (nFormat == m_info.internalFormat) ? -1 : 0;
		break;

	default:
		break;
	}
return nFormat == m_info.internalFormat;
}

//------------------------------------------------------------------------------

int CTexture::Verify (void)
{
while (!FormatSupported ()) {
	switch (m_info.format) {
		case GL_INTENSITY4:
			if (gameStates.ogl.bLuminance4Alpha4) {
				m_info.internalFormat = 2;
				m_info.format = GL_LUMINANCE_ALPHA;
				break;
				}

		case GL_LUMINANCE4_ALPHA4:
			m_info.internalFormat = 4;
			m_info.format = GL_RGBA;
			break;

		case GL_RGB:
			m_info.internalFormat = 3;
			m_info.format = GL_RGBA;
			break;

		case GL_RGB5:
			m_info.internalFormat = 3;
			m_info.format = GL_RGB;
			break;
		
		case GL_RGBA4:
			m_info.internalFormat = 4;
			m_info.format = GL_RGBA;
			break;

		default:
#if TRACE
			console.printf (CON_DBG,"...no texP format to fall back on\n");
#endif
			return 1;
		}
	}
return 0;
}

//------------------------------------------------------------------------------

void CTexture::SetBufSize (int dbits, int bits, int w, int h)
{
if (bits <= 0) //the beta nvidia GLX server. doesn'texP ever return any bit sizes, so just use some assumptions.
	bits = dbits;
}

//------------------------------------------------------------------------------

void CTexture::SetSize (void)
{
	GLint	w, h;
	int	nBits = 16, a = 0;

if (gameStates.ogl.bGetTexLevelParam) {
		GLint texP;

	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_LUMINANCE_SIZE, &texP);
	a += texP;
	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_INTENSITY_SIZE, &texP);
	a += texP;
	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, &texP);
	a += texP;
	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_GREEN_SIZE, &texP);
	a += texP;
	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE, &texP);
	a += texP;
	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE, &texP);
	a += texP;
	}
else {
	w = m_info.tw;
	h = m_info.th;
	}
switch (m_info.format) {
	case GL_LUMINANCE:
		nBits = 8;
		break;

	case GL_LUMINANCE_ALPHA:
		nBits = 8;
		break;

	case GL_RED:
		nBits = 8;
		break;

	case GL_RGB:
		nBits = 24;

	case GL_RGBA:
#if TEXTURE_COMPRESSION
	case GL_COMPRESSED_RGBA:
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
#endif
		nBits = 32;
		break;

	case GL_RGB5:
	case GL_RGBA4:
		nBits = 16;
		break;

	case GL_ALPHA:
		nBits = 8;
		break;

	default:
		Error ("TexSetSize unknown texformat\n");
		break;
	}
SetBufSize (nBits, a, w, h);
}

//------------------------------------------------------------------------------

void CTexture::Unlink (void)
{
if (m_info.bmP && (m_info.bmP->Texture () == this))
	m_info.bmP->SetTexture (NULL);
#if DBG
else
	m_info.bmP = m_info.bmP;
#endif
m_info.bmP = NULL;
}

//------------------------------------------------------------------------------

void CTexture::Destroy (void)
{
if (m_info.handle && (m_info.handle != GLuint (-1))) {
	OglDeleteTextures (1, reinterpret_cast<GLuint*> (&m_info.handle));
	m_info.handle = 0;
	Unlink ();
	Init ();
	}
}

//------------------------------------------------------------------------------

void CTexture::Release (void)
{
textureManager.Push (this);
}

//------------------------------------------------------------------------------
//Texture () MUST be bound first
void CTexture::Wrap (int state)
{
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, state);
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, state);
}

//------------------------------------------------------------------------------

GLuint CTexture::Create (int w, int h)			
{
	int		nSize = w * h * sizeof (uint); 
	ubyte		*data = new ubyte [nSize]; 

if (!data)
	return 0;
memset (data, 0, nSize); 	
OglGenTextures (1, &m_info.handle); 
OGL_BINDTEX  (m_info.handle); 		
glTexImage2D (GL_TEXTURE_2D, 0, 4, w, h, 0, gameStates.ogl.nRGBAFormat, GL_UNSIGNED_BYTE, data); 			// Build Texture Using Information In data
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
delete[] data; 							
return m_info.handle; 						
}

//------------------------------------------------------------------------------

int CTexture::Prepare (bool bCompressed)
{
#if TEXTURE_COMPRESSION
if (bCompressed) {
	m_info.SetWidth (w);
	m_info.SetHeight (h);
	}
#endif
//calculate u/v values that would make the resulting texture correctly sized
m_info.u = (float) m_info.w / (float) m_info.tw;
m_info.v = (float) m_info.h / (float) m_info.th;
return 0;
}

//------------------------------------------------------------------------------

#if TEXTURE_COMPRESSION

int CTexture::Compress (void)
{
if (m_info.internalformat != GL_COMPRESSED_RGBA)
	return 0;

	GLint		nFormat, nParam;
	ubyte		*data;
	CBitmap	*bmP = m_info.bmP;

glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB, &nParam);
if (nParam) {
	glGetTexLevelParameteriv (GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &nFormat);
	if ((nFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) ||
		 (nFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ||
		 (nFormat == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT) ||
		 (nFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)) {
		glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &nParam);
		if (nParam && (data = new ubyte [nParam])) {
			bmP->DestroyBuffer ();
			glGetCompressedTexImage (GL_TEXTURE_2D, 0, reinterpret_cast<GLvoid*> (data));
			bmP->SetBuffer (data, 0, nParam);
			bmP->SetFormat (nFormat);
			bmP->SetCompressed (1);
			}
		}
	}
if (bmP->Compressed ())
	return 1;
if (bmP->BPP () == 3) {
	m_info.format = GL_RGB;
	m_info.internalFormat = 3;
	}
else {
	m_info.format = GL_RGBA;
	m_info.internalFormat = 4;
	}
return 0;
}

#endif

//------------------------------------------------------------------------------

#if TEXTURE_COMPRESSION
int CTexture::Load (ubyte *buffer, int nBufSize, int nFormat, bool bCompressed)
#else
int CTexture::Load (ubyte *buffer)
#endif
{
if (!buffer)
	return 1;
// Generate OpenGL texture IDs.
OglGenTextures (1, reinterpret_cast<GLuint*> (&m_info.handle));
if (!m_info.handle) {
#if DBG
	int nError = glGetError ();
#endif
	return 1;
	}
m_info.prio = m_info.bMipMaps ? (m_info.th > m_info.tw) ? 1.0f : 0.5f : 0.1f;
glPrioritizeTextures (1, (GLuint *) &m_info.handle, &m_info.prio);
Bind ();
glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
if (m_info.bSmoothe) {
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gameStates.ogl.texMagFilter);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gameStates.ogl.texMinFilter);
	}
else {
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
#if TEXTURE_COMPRESSION
if (bCompressed) {
	glCompressedTexImage2D (
		GL_TEXTURE_2D, 0, nFormat,
		texP->tw, texP->th, 0, nBufSize, buffer);
	}
else 
#endif
#if DBG
	if (gameStates.ogl.bNeedMipMaps >= 0) 
#endif
	{
#if 0
#if RENDER2TEXTURE == 2
	if (m_info.bMipMaps && gameStates.ogl.bNeedMipMaps && !glGenerateMipMapEXT)
		glTexParameteri (GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
#endif
	glTexImage2D (GL_TEXTURE_2D, 0, m_info.internalFormat, m_info.tw, m_info.th, 0, m_info.format, GL_UNSIGNED_BYTE, buffer);
#	if RENDER2TEXTURE == 2
	if (m_info.bMipMaps && gameStates.ogl.bNeedMipMaps && glGenerateMipMapEXT)
		glGenerateMipMapEXT (GL_TEXTURE_2D);
#	endif
#else
	if (m_info.bMipMaps && gameStates.ogl.bNeedMipMaps)
		gluBuild2DMipmaps (GL_TEXTURE_2D, m_info.internalFormat, m_info.tw, m_info.th, m_info.format, GL_UNSIGNED_BYTE, buffer);
	else
		glTexImage2D (GL_TEXTURE_2D, 0, m_info.internalFormat, m_info.tw, m_info.th, 0, m_info.format, GL_UNSIGNED_BYTE, buffer);
#endif
	OglClearError (1);
#if TEXTURE_COMPRESSION
	Compress ();
#endif
	//SetSize ();
	}
return 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CBitmap::UnlinkTexture (void)
{
if (Texture () && (Texture ()->Handle () == -1)) {
	Texture ()->SetHandle (0);
	SetTexture (NULL);
	}
}

//------------------------------------------------------------------------------

void CBitmap::Unlink (int bAddon)
{
	CBitmap	*altBmP, *bmfP;
	int			i, j;

if (bAddon || (Type () == BM_TYPE_STD)) {
	if (Mask ())
		Mask ()->UnlinkTexture ();
	if (!(bAddon || Override ()))
		UnlinkTexture ();
	else {
		altBmP = bAddon ? this : Override ();
		if (altBmP->Mask ())
			altBmP->Mask ()->UnlinkTexture ();
		altBmP->UnlinkTexture ();
		if ((altBmP->Type () == BM_TYPE_ALT) && altBmP->Frames ()) {
			i = altBmP->FrameCount ();
			if (i > 1) {
				for (j = i, bmfP = altBmP->Frames (); j; j--, bmfP++) {
					if (bmfP->Mask ())
						bmfP->Mask ()->UnlinkTexture ();
					bmfP->UnlinkTexture ();
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------------

CBitmap *LoadFaceBitmap (short nTexture, short nFrameIdx)
{
	CBitmap	*bmP;
	int		nFrames;

LoadBitmap (gameData.pig.tex.bmIndexP [nTexture].index, gameStates.app.bD1Mission);
bmP = gameData.pig.tex.bitmapP + gameData.pig.tex.bmIndexP [nTexture].index;
if (bmP->Override ()) {
	bmP = bmP->Override ();
	if (bmP->WallAnim ()) {
		nFrames = bmP->FrameCount ();
		if (nFrames > 1) {
			bmP->SetupTexture (1, 3, 1);
			if (bmP->Frames ()) {
				if ((nFrameIdx >= 0) || (-nFrames > nFrameIdx))
					bmP->SetCurFrame (bmP->Frames ());
				else
					bmP->SetCurFrame (bmP->Frames () - nFrameIdx - 1);
				bmP->CurFrame ()->SetupTexture (1, 3, 1);
				}
			}
		}
	}
return bmP;
}

//------------------------------------------------------------------------------
//loads a palettized bitmap into a ogl RGBA texture.
//Sizes and pads dimensions to multiples of 2 if necessary.
//In theory this could be a problem for repeating textures, but all real
//textures (not sprites, etc) in descent are 64x64, so we are ok.
//stores OpenGL textured id in *texid and u/v values required to get only the real data in *u/*v
int CBitmap::LoadTexture (int dxo, int dyo, int nTransp, int superTransp)
{
	ubyte*		data = Buffer ();

if (!data)
	return 1;

	GLubyte*		bufP = NULL;
	CTexture		texture, *texP;
	bool			bLocal;
	int			funcRes = 1;

texP = Texture ();
if ((bLocal = (texP == NULL))) {
	texture.Setup (m_info.props.w, m_info.props.h, m_info.props.rowSize, m_info.nBPP);
	texP = &texture;
	}
#if TEXTURE_COMPRESSION
texP->Prepare (m_info.bCompressed);
#	ifndef __macosx__
if (!(m_info.bCompressed || superTransp || Parent ())) {
	if (gameStates.ogl.bTextureCompression && gameStates.ogl.bHaveTexCompression &&
		 ((texP->Format () == GL_RGBA) || (texP->Format () == GL_RGB)) && 
		 (TextureP->TW () >= 64) && (texP->TH () >= m_info.tw))
		texP->SetInternalformat (GL_COMPRESSED_RGBA);
	if (texP->Verify ())
		return 1;
	}
#	endif
#else
texP->Prepare ();
#endif

//	if (width!=twidth || height!=theight)
#if RENDER2TEXTURE
if (!texP->IsRenderBuffer ()) 
#endif
 {
#if TEXTURE_COMPRESSION
	if (data && !m_info.bCompressed)
#else
	if (data) {
#endif
		if (nTransp < 0) 
			bufP = texP->Copy (dxo, dyo, data);
		else
			bufP = texP->Convert (dxo, dyo, this, nTransp, superTransp);
		}
#if TEXTURE_COMPRESSION
	texP->Load (Compressed () ? Buffer () : bufP, BufSize (), Format (), Compressed ());
#else
	funcRes = texP->Load (bufP);
#endif
	if (bLocal)
		texP->Destroy ();
	}
return 0;
}

//------------------------------------------------------------------------------

ubyte decodebuf [2048*2048];

#if RENDER2TEXTURE == 1
int CBitmap::PrepareTexture (int bMipMap, int nTransp, int bMask, CBO *renderBuffer)
#elif RENDER2TEXTURE == 2
int CBitmap::PrepareTexture (int bMipMap, int nTransp, int bMask, CFBO *renderBuffer)
#else
int CBitmap::PrepareTexture (int bMipMap, int nTransp, int bMask, tPixelBuffer *renderBuffer)
#endif
{
	CTexture	*texP;

if ((m_info.nType == BM_TYPE_STD) && Parent () && (Parent () != this))
	return Parent ()->PrepareTexture (bMipMap, nTransp, bMask, renderBuffer);

if (!(texP = Texture ())) {
	if (!(texP = textureManager.Get (this)))
		return 1;
	SetTexture (texP);
	texP->Setup (m_info.props.w, m_info.props.h, m_info.props.rowSize, m_info.nBPP, bMask, bMipMap, 0, this);
	texP->SetRenderBuffer (renderBuffer);
	}
else {
	if (texP->Handle () > 0)
		return 0;
	if (!texP->Width ())
		texP->Setup (m_info.props.w, m_info.props.h, m_info.props.rowSize, m_info.nBPP, bMask, bMipMap, 0, this);
	}
if (Flags () & BM_FLAG_RLE)
	RLEExpand (NULL, 0);
if (!bMask) {
	tRgbColorf color;
	if (0 <= (AvgColor (&color)))
		SetAvgColorIndex ((ubyte) Palette ()->ClosestColor (&color));
	}
LoadTexture (0, 0, (m_info.props.flags & BM_FLAG_TGA) ? -1 : nTransp, 
				 (m_info.props.flags & (BM_FLAG_TRANSPARENT | BM_FLAG_SUPER_TRANSPARENT)) != 0);
return 0;
}

//------------------------------------------------------------------------------

int CBitmap::SetupFrames (int bMipMaps, int nTransp, int bLoad)
{
	int	nFrames = (m_info.nType == BM_TYPE_ALT) ? m_info.frames.nCount : 0;
	ubyte	nFlags;

if (nFrames < 2)
	return 0;
else {
#if DBG
	if (!strcmp (m_info.szName, "sparks.tga"))
		nDbgSeg = nDbgSeg;
#endif
	m_info.frames.bmP = new CBitmap [nFrames];

	int		i, w = m_info.props.w;
	CBitmap* bmfP = m_info.frames.currentP = m_info.frames.bmP;

	for (i = 0; i < nFrames; i++, bmfP++) {
		bmfP->InitChild (this, 0, i * w, w, w);
		bmfP->SetType (BM_TYPE_FRAME);
		bmfP->SetTop (0);
		nFlags = bmfP->Flags ();
		if (m_info.transparentFrames [i / 32] & (1 << (i % 32)))
			nFlags |= BM_FLAG_TRANSPARENT;
		if (m_info.supertranspFrames [i / 32] & (1 << (i % 32)))
			nFlags |= BM_FLAG_SUPER_TRANSPARENT;
		if (nFlags & BM_FLAG_SEE_THRU)
			nFlags |= BM_FLAG_SEE_THRU;
		bmfP->SetFlags (nFlags);
		if (bLoad)
			bmfP->PrepareTexture (bMipMaps, nTransp, 0, NULL);
		}
	}
return 1;
}

//------------------------------------------------------------------------------

CBitmap *CBitmap::CreateMask (void)
{
	int		i = (int) Width () * (int) Height ();
	ubyte		*pi;
	ubyte		*pm;

if (!gameStates.render.textures.bHaveMaskShader)
	return NULL;
if (m_info.maskP)
	return m_info.maskP;
SetBPP (4);
if (!(m_info.maskP = CBitmap::Create (0, (Width ()  + 1) / 2, (Height () + 1) / 2, 4)))
	return NULL;
#if DBG
sprintf (m_info.szName, "{%s}", Name ());
#endif
m_info.maskP->SetWidth (m_info.props.w);
m_info.maskP->SetHeight (m_info.props.w);
m_info.maskP->SetBPP (1);
UseBitmapCache (m_info.maskP, (int) m_info.maskP->Width () * (int) m_info.maskP->RowSize ());
if (m_info.props.flags & BM_FLAG_TGA) {
	for (pi = Buffer (), pm = m_info.maskP->Buffer (); i; i--, pi += 4, pm++)
		if ((pi [0] == 120) && (pi [1] == 88) && (pi [2] == 128))
			*pm = 0;
		else
			*pm = 0xff;
	}
else {
	for (pi = Buffer (), pm = m_info.maskP->Buffer (); i; i--, pi++, pm++)
		if (*pi == SUPER_TRANSP_COLOR)
			*pm = 0;
		else
			*pm = 0xff;
	}
return m_info.maskP;
}

//------------------------------------------------------------------------------

int CBitmap::CreateMasks (void)
{
	int	nMasks, i, nFrames;

if (!gameStates.render.textures.bHaveMaskShader)
	return 0;
if ((m_info.nType != BM_TYPE_ALT) || !m_info.frames.bmP) {
	if (m_info.props.flags & BM_FLAG_SUPER_TRANSPARENT)
		return CreateMask () != NULL;
	return 0;
	}
nFrames = FrameCount ();
for (nMasks = i = 0; i < nFrames; i++)
	if (m_info.supertranspFrames [i / 32] & (1 << (i % 32)))
		if (m_info.frames.bmP [i].CreateMask ())
			nMasks++;
return nMasks;
}

//------------------------------------------------------------------------------
// returns 0:Success, 1:Error

int CBitmap::Bind (int bMipMaps, int nTransp)
{
	CBitmap		*bmP;

#if DBG
	static int nDepth = 0;

if (nDepth > 1)
	return -1;
nDepth++;

if (!strcmp (m_info.szName, "sparks.tga"))
	nDbgSeg = nDbgSeg;
#endif
if ((bmP = HasOverride ())) {
	int i = bmP->Bind (bMipMaps, nTransp);
#if DBG
	nDepth--;
#endif
	return i;
	}

	CTexture* texP = Texture ();

#if RENDER2TEXTURE
if (texP && texP->IsRenderBuffer ())
	texP->BindRenderBuffer ();
else
#endif
 {
	if (!Prepared ()) {
		if (!(bmP = SetupTexture (bMipMaps, nTransp, 1))) {
#if DBG
			SetupTexture (bMipMaps, nTransp, 1);
			nDepth--;
#endif
			return 1;
			}
		texP = bmP->Texture ();
#if 0
		if (!texP)
			bmP->SetupTexture (1, nTransp, 1);
#endif
		}

	CBitmap* mask = Mask ();

	if (mask && !mask->Prepared ())
		mask->SetupTexture (0, -1, 1);
	}
if (!texP)
	return -1;
texP->Bind ();

#if DBG
nDepth--;
#endif
return 0;
}

//------------------------------------------------------------------------------

CBitmap *CBitmap::SetupTexture (int bMipMaps, int nTransp, int bLoad)
{
	CBitmap *bmP;

if ((bmP = HasOverride ()))
	return bmP->SetupTexture (bMipMaps, nTransp, bLoad);

int	h, w, nFrames;

h = m_info.props.h;
w = m_info.props.w;
if (!(h * w))
	return NULL;
nFrames = (m_info.nType == BM_TYPE_ALT) ? FrameCount () : 0;
if (!(m_info.props.flags & BM_FLAG_TGA) || (nFrames < 2)) {
	if (bLoad && PrepareTexture (bMipMaps, nTransp, 0, NULL))
		return NULL;
	if (CreateMasks () && Mask ()->PrepareTexture (0, -1, 1, NULL))
		return NULL;
	}
else if (!Frames ()) {
	h = CreateMasks ();
	SetupFrames (bMipMaps, nTransp, bLoad);
	if (h) {
		if (bLoad) {
			CBitmap*	bmfP = Frames ();
			for (int i = nFrames; i; i--, bmfP++) {
				if (bmfP->PrepareTexture (bMipMaps, nTransp, 1))
					return NULL;
				if (bmfP->Mask () && (bmfP->Mask ()->PrepareTexture (0, -1, 1, NULL)))
					return NULL;
				}
			}
		}
	}
return Override (-1);
}

//------------------------------------------------------------------------------

