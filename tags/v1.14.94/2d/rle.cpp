/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1998 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "descent.h"
#include "pstypes.h"
#include "u_mem.h"
#include "mono.h"
#include "gr.h"
#include "grdef.h"
#include "error.h"
#include "rle.h"
#include "byteswap.h"

#define RLE_CODE        0xE0
#define NOT_RLE_CODE    31

#define IS_RLE_CODE(x) (( (x) & RLE_CODE) == RLE_CODE)

#define rle_stosb(_dest, _len, _color)	memset(_dest,_color,_len)

//------------------------------------------------------------------------------

void gr_rle_decode (ubyte *src, ubyte *dest)
{
	ubyte data, count = 0;

while (1) {
	data = *src++;
	if (!IS_RLE_CODE (data))
		*dest++ = data;
	else {
		if (!(count = (data & NOT_RLE_CODE)))
			return;
		data = *src++;
		memset (dest, data, count);
		}
	}
}

//------------------------------------------------------------------------------

// Given pointer to start of one scanline of rle data, uncompress it to
// dest, from source pixels x1 to x2.
void gr_rle_expand_scanline_masked (ubyte *dest, ubyte *src, int x1, int x2 )
{
	int i = 0;
	ubyte count;
	ubyte color=0;

	if (x2 < x1) return;

	count = 0;
	while (i < x1) {
		color = *src++;
		if (color == RLE_CODE) return;
		if (IS_RLE_CODE (color)) {
			count = color & (~RLE_CODE);
			color = *src++;
		} else {
			// unique
			count = 1;
		}
		i += count;
	}
	count = i - x1;
	i = x1;
	// we know have '*count' pixels of 'color'.

	if (x1+count > x2) {
		count = x2-x1+1;
		if (color != TRANSPARENCY_COLOR)
			rle_stosb ((char* )dest, count, color);
		return;
	}

	if (color != TRANSPARENCY_COLOR)
		rle_stosb ((char* )dest, count, color);
	dest += count;
	i += count;

	while (i <= x2)	 {
		color = *src++;
		if (color == RLE_CODE) return;
		if (IS_RLE_CODE (color)) {
			count = color & (~RLE_CODE);
			color = *src++;
		} else {
			// unique
			count = 1;
		}
		// we know have '*count' pixels of 'color'.
		if (i+count <= x2) {
			if (color != TRANSPARENCY_COLOR)
				rle_stosb ((char* )dest, count, color);
			i += count;
			dest += count;
		} else {
			count = x2-i+1;
			if (color != TRANSPARENCY_COLOR)
				rle_stosb ((char* )dest, count, color);
			i += count;
			dest += count;
		}

	}
}

//------------------------------------------------------------------------------

void gr_rle_expand_scanline (ubyte *dest, ubyte *src, int x1, int x2 )
{
	int i = 0;
	ubyte count;
	ubyte color=0;

	if (x2 < x1) return;

	count = 0;
	while (i < x1) {
		color = *src++;
		if (color == RLE_CODE) return;
		if (IS_RLE_CODE (color)) {
			count = color & (~RLE_CODE);
			color = *src++;
		} else {
			// unique
			count = 1;
		}
		i += count;
	}
	count = i - x1;
	i = x1;
	// we know have '*count' pixels of 'color'.

	if (x1+count > x2) {
		count = x2-x1+1;
		rle_stosb ((char* )dest, count, color);
		return;
	}

	rle_stosb ((char* )dest, count, color);
	dest += count;
	i += count;

	while (i <= x2)	 {
		color = *src++;
		if (color == RLE_CODE) return;
		if (IS_RLE_CODE (color)) {
			count = color & (~RLE_CODE);
			color = *src++;
		} else {
			// unique
			count = 1;
		}
		// we know have '*count' pixels of 'color'.
		if (i+count <= x2) {
			rle_stosb ((char* )dest, count, color);
			i += count;
			dest += count;
		} else {
			count = x2-i+1;
			rle_stosb ((char* )dest, count, color);
			i += count;
			dest += count;
		}
	}
}

//------------------------------------------------------------------------------

int gr_rle_encode (int org_size, ubyte *src, ubyte *dest)
{
	int i;
	ubyte c, oc;
	ubyte count;
	ubyte *dest_start;

	dest_start = dest;
	oc = *src++;
	count = 1;

	for (i=1; i<org_size; i++) {
		c = *src++;
		if (c!=oc) {
			if (count) {
				if ((count==1) && (! IS_RLE_CODE (oc))) {
					*dest++ = oc;
					Assert (oc != RLE_CODE);
				} else {
					count |= RLE_CODE;
					*dest++ = count;
					*dest++ = oc;
				}
			}
			oc = c;
			count = 0;
		}
		count++;
		if (count == NOT_RLE_CODE) {
			count |= RLE_CODE;
			*dest++=count;
			*dest++=oc;
			count = 0;
		}
	}
	if (count) {
		if ((count==1) && (! IS_RLE_CODE (oc))) {
			*dest++ = oc;
			Assert (oc != RLE_CODE);
		} else {
			count |= RLE_CODE;
			*dest++ = count;
			*dest++ = oc;
		}
	}
	*dest++ = RLE_CODE;

	return (int) (dest - dest_start);
}

//------------------------------------------------------------------------------

int gr_rle_getsize (int org_size, ubyte *src)
{
	int i;
	ubyte c, oc;
	ubyte count;
	int dest_size=0;

	oc = *src++;
	count = 1;

	for (i=1; i<org_size; i++) {
		c = *src++;
		if (c!=oc) {
			if (count) {
				if ((count==1) && (! IS_RLE_CODE (oc))) {
					dest_size++;
				} else {
					dest_size++;
					dest_size++;
				}
			}
			oc = c;
			count = 0;
		}
		count++;
		if (count == NOT_RLE_CODE) {
			dest_size++;
			dest_size++;
			count = 0;
		}
	}
	if (count) {
		if ((count==1) && (! IS_RLE_CODE (oc))) {
			dest_size++;
		} else {
			dest_size++;
			dest_size++;
		}
	}
	dest_size++;

	return dest_size;
}

//------------------------------------------------------------------------------

int CBitmap::RLECompress (void)
{
	int y, d1, d;
	int doffset;
	ubyte *rle_data;
	int large_rle = 0;

	// first must check to see if this is large bitmap.

	for (y=0; y < m_info.props.h; y++) {
		d1= gr_rle_getsize (m_info.props.w, &Buffer ()[m_info.props.w*y]);
		if (d1 > 255) {
			large_rle = 1;
			break;
		}
	}

	if (!(rle_data = new ubyte [MAX_BMP_SIZE (m_info.props.w, m_info.props.h)]))
		return 0;
	if (!large_rle)
		doffset = 4 + m_info.props.h;
	else
		doffset = 4 + (2 * m_info.props.h);		// each row of rle'd bitmap has short instead of byte offset now

	for (y=0; y<m_info.props.h; y++) {
		d1 = gr_rle_getsize (m_info.props.w, &Buffer ()[m_info.props.w*y]);
		if (( (doffset+d1) > m_info.props.w*m_info.props.h) || (d1 > (large_rle?32767:255))) {
			delete[] rle_data;
			return 0;
		}
		d = gr_rle_encode (m_info.props.w, &Buffer ()[m_info.props.w*y], &rle_data[doffset]);
		Assert (d==d1);
		doffset	+= d;
		if (large_rle)
			*reinterpret_cast<short*> (rle_data + 2 * y + 4) = (short) d;
		else
			rle_data[y+4] = d;
	}
	memcpy (rle_data, &doffset, 4);
	memcpy (Buffer (), rle_data, doffset);
	delete[] rle_data;
	m_info.props.flags |= BM_FLAG_RLE;
	if (large_rle)
		m_info.props.flags |= BM_FLAG_RLE_BIG;
	return 1;
}

//------------------------------------------------------------------------------

#define MAX_CACHE_BITMAPS 32

typedef struct rle_cache_element {
	CBitmap * rle_bitmap;
	ubyte * rle_data;
	CBitmap * expanded_bitmap;
	int last_used;
} rle_cache_element;

int rle_cache_initialized = 0;
int rleCounter = 0;
int rle_next = 0;
rle_cache_element rle_cache [MAX_CACHE_BITMAPS];

int rle_hits = 0;
int rle_misses = 0;

void _CDECL_ RLECacheClose (void)
{
if (rle_cache_initialized) {
	int i;
	PrintLog ("deleting RLE cache\n");
	rle_cache_initialized = 0;
	for (i=0; i<MAX_CACHE_BITMAPS; i++) {
		delete rle_cache [i].expanded_bitmap;
		rle_cache [i].expanded_bitmap = NULL;
		}
	}
gameData.pig.tex.rleBuffer.Destroy ();
}

//------------------------------------------------------------------------------

void RLECacheInit ()
{
	int i;

for (i=0; i<MAX_CACHE_BITMAPS; i++) {
	rle_cache [i].rle_bitmap = NULL;
	rle_cache [i].expanded_bitmap = CBitmap::Create (0, 64, 64, 1);
	rle_cache [i].last_used = 0;
	Assert (rle_cache [i].expanded_bitmap != NULL);
	}
rle_cache_initialized = 1;
//atexit (RLECacheClose);
}

//------------------------------------------------------------------------------

void RLECacheFlush (void)
{
	int i;

for (i=0; i<MAX_CACHE_BITMAPS; i++) {
	rle_cache [i].rle_bitmap = NULL;
	rle_cache [i].last_used = 0;
	}
}

//------------------------------------------------------------------------------

CBitmap *rle_expand_texture (CBitmap *bmP)
{
	int i;
	int lowestCount, lc;
	int least_recently_used;

if (!rle_cache_initialized) 
	RLECacheInit ();

Assert (!(bmP->Flags () & BM_FLAG_PAGED_OUT));

lc = rleCounter;
rleCounter++;
if (rleCounter < lc) {
	for (i=0; i<MAX_CACHE_BITMAPS; i++) {
		rle_cache [i].rle_bitmap = NULL;
		rle_cache [i].last_used = 0;
		}
	}

lowestCount = rle_cache [rle_next].last_used;
least_recently_used = rle_next;
rle_next++;
if (rle_next >= MAX_CACHE_BITMAPS)
	rle_next = 0;
for (i = 0; i < MAX_CACHE_BITMAPS; i++) {
	if (rle_cache [i].rle_bitmap == bmP) {
		rle_hits++;
		rle_cache [i].last_used = rleCounter;
		return rle_cache [i].expanded_bitmap;
		}
	if (rle_cache [i].last_used < lowestCount) {
		lowestCount = rle_cache [i].last_used;
		least_recently_used = i;
		}
	}
Assert (bmP->Width () <=64 && bmP->Height () <= 64); //dest buffer is 64x64
rle_misses++;
bmP->ExpandTo (rle_cache [least_recently_used].expanded_bitmap);
rle_cache [least_recently_used].rle_bitmap = bmP;
rle_cache [least_recently_used].last_used = rleCounter;
return rle_cache [least_recently_used].expanded_bitmap;
}

//------------------------------------------------------------------------------

void gr_rle_expand_scanline_generic (CBitmap * dest, int dx, int dy, ubyte *src, int x1, int x2)
{
	int i = 0, j;
	int count;
	ubyte color=0;

	if (x2 < x1) return;

	count = 0;
	while (i < x1) {
		color = *src++;
		if (color == RLE_CODE) return;
		if (IS_RLE_CODE (color)) {
			count = color & NOT_RLE_CODE;
			color = *src++;
		} else {
			// unique
			count = 1;
		}
		i += count;
	}
	count = i - x1;
	i = x1;
	// we know have '*count' pixels of 'color'.

	if (x1+count > x2) {
		count = x2-x1+1;
		for (j=0; j<count; j++)
			dest->DrawPixel (dx++, dy, color);
		return;
	}

	for (j=0; j<count; j++)
		dest->DrawPixel (dx++, dy, color);
	i += count;

	while (i <= x2)	 {
		color = *src++;
		if (color == RLE_CODE) return;
		if (IS_RLE_CODE (color)) {
			count = color & NOT_RLE_CODE;
			color = *src++;
		} else {
			// unique
			count = 1;
		}
		// we know have '*count' pixels of 'color'.
		if (i+count <= x2) {
			for (j=0; j<count; j++)
				dest->DrawPixel (dx++, dy, color);
			i += count;
		} else {
			count = x2-i+1;
			for (j=0; j<count; j++)
				dest->DrawPixel (dx++, dy, color);
			i += count;
		}
	}
}

//------------------------------------------------------------------------------

void gr_rle_expand_scanline_generic_masked (CBitmap * dest, int dx, int dy, ubyte *src, int x1, int x2 )
{
	int i = 0, j;
	int count;
	ubyte color = 0;

	if (x2 < x1) return;

	count = 0;
	while (i < x1) {
		color = *src++;
		if (color == RLE_CODE) return;
		if (IS_RLE_CODE (color)) {
			count = color & NOT_RLE_CODE;
			color = *src++;
		} else {
			// unique
			count = 1;
		}
		i += count;
	}
	count = i - x1;
	i = x1;
	// we know have '*count' pixels of 'color'.

	if (x1+count > x2) {
		count = x2-x1+1;
		if (color != TRANSPARENCY_COLOR) {
			for (j=0; j<count; j++)
				dest->DrawPixel (dx++, dy, color);
		}
		return;
	}

	if (color != TRANSPARENCY_COLOR) {
		for (j=0; j<count; j++)
			dest->DrawPixel (dx++, dy, color);
	} else
		dx += count;
	i += count;

	while (i <= x2)	 {
		color = *src++;
		if (color == RLE_CODE) return;
		if (IS_RLE_CODE (color)) {
			count = color & NOT_RLE_CODE;
			color = *src++;
		} else {
			// unique
			count = 1;
		}
		// we know have '*count' pixels of 'color'.
		if (i+count <= x2) {
			if (color != TRANSPARENCY_COLOR) {
				for (j=0; j<count; j++)
					dest->DrawPixel (dx++, dy, color);
			} else
				dx += count;
			i += count;
		} else {
			count = x2-i+1;
			if (color != TRANSPARENCY_COLOR) {
				for (j=0; j<count; j++)
					dest->DrawPixel (dx++, dy, color);
			} else
				dx += count;
			i += count;
		}
	}
}

//------------------------------------------------------------------------------

void CBitmap::ExpandTo (CBitmap * destP)
{
	ubyte * dbits;
	ubyte * sbits;
	int i;
#ifdef RLE_DECODE_ASM
	ubyte * dbits1;
#endif

sbits = Buffer () + 4 + m_info.props.h;
dbits = destP->Buffer ();
destP->SetFlags (m_info.props.flags & ~BM_FLAG_RLE);
for (i=0; i < m_info.props.h; i++) {
	gr_rle_decode (sbits, dbits);
	sbits += (int) Buffer () [4+i];
	dbits += m_info.props.w;
	}
}

//------------------------------------------------------------------------------
// swaps entries 0 and 255 in an RLE bitmap without uncompressing it

void CBitmap::RLESwap_0_255 (void)
{
	int h, i, j, len, rle_big;
	ubyte *ptr, *ptr2, *temp, *start;
	ushort nLineSize;

rle_big = m_info.props.flags & BM_FLAG_RLE_BIG;
temp = new ubyte [MAX_BMP_SIZE (m_info.props.w, m_info.props.h)];
if (rle_big) {                  // set ptrs to first lines
	ptr = Buffer () + 4 + 2 * m_info.props.h;
	ptr2 = temp + 4 + 2 * m_info.props.h;
} else {
	ptr = Buffer () + 4 + m_info.props.h;
	ptr2 = temp + 4 + m_info.props.h;
}
for (i = 0; i < m_info.props.h; i++) {
	start = ptr2;
	if (rle_big)
		nLineSize = INTEL_SHORT (*reinterpret_cast<ushort*> (Buffer () + 4 + 2 * i));
	else
		nLineSize = Buffer () [4 + i];
	for (j = 0; j < nLineSize; j++) {
		h = ptr [j];
		if (!IS_RLE_CODE (h)) {
			if (h == 0) {
				*ptr2++ = RLE_CODE | 1;
				*ptr2++ = 255;
				}
			else
				*ptr2++ = ptr[j];
			}
		else {
			*ptr2++ = h;
			if ((h & NOT_RLE_CODE) == 0)
				break;
			h = ptr [++j];
			if (h == 0)
				*ptr2++ = 255;
			else if (h == 255)
				*ptr2++ = 0;
			else
				*ptr2++ = h;
			}
		}
	if (rle_big)                // set line size
		*reinterpret_cast<ushort*> (temp + 4 + 2 * i) = INTEL_SHORT ((short) (ptr2 - start));
	else
		temp[4 + i] = (ubyte) (ptr2 - start);
	ptr += nLineSize;           // go to next line
	}
len = (int) (ptr2 - temp);
*reinterpret_cast<int*> (temp) = len;           // set total size
memcpy (Buffer (), temp, len);
delete[] temp;
}

//------------------------------------------------------------------------------
// remaps all entries using colorMap in an RLE bitmap without uncompressing it

int CBitmap::RLERemap (ubyte *colorMap, int maxLen)
{
	int				h, i, j, len, bBigRLE;
	ubyte	*pSrc, *destP, *remapBuf, *start;
	ushort nLineSize;

bBigRLE = m_info.props.flags & BM_FLAG_RLE_BIG;
remapBuf = new ubyte [MAX_BMP_SIZE (m_info.props.w, m_info.props.h) + 30000];
if (bBigRLE) {                  // set ptrs to first lines
	pSrc = Buffer () + 4 + 2 * m_info.props.h;
	destP = remapBuf + 4 + 2 * m_info.props.h;
	}
else {
	pSrc = Buffer () + 4 + m_info.props.h;
	destP = remapBuf + 4 + m_info.props.h;
	}
for (i = 0; i < m_info.props.h; i++) {
	start = destP;
	if (bBigRLE)
		nLineSize = INTEL_SHORT (*reinterpret_cast<ushort*> (Buffer (4 + 2 * i)));
	else
		nLineSize = Buffer () [4 + i];
	for (j = 0; j < nLineSize; j++) {
		h = pSrc [j];
		if (!IS_RLE_CODE (h)) {
			if (IS_RLE_CODE (colorMap [h])) 
				*destP++ = RLE_CODE | 1; // add "escape sequence"
			*destP++ = colorMap [h]; // translate
			}
		else {
			*destP++ = h; // just copy current rle code
			if ((h & NOT_RLE_CODE) == 0)
				break;
			*destP++ = colorMap [pSrc [++j]]; // translate
			}
		}
	if (bBigRLE)                // set line size
		*(reinterpret_cast<ushort*> (remapBuf + 4 + 2 * i)) = INTEL_SHORT ((short) (destP - start));
	else
		remapBuf [4 + i] = (ubyte) (destP - start);
	pSrc += nLineSize;           // go to next line
	}
len = (int) (destP - remapBuf);
Assert (len <= m_info.props.w * m_info.props.rowSize);
*reinterpret_cast<int*> (remapBuf) = len;           // set total size
memcpy (Buffer (), remapBuf, len);
delete[] remapBuf;
return len;
}

//------------------------------------------------------------------------------

int CBitmap::RLEExpand (ubyte *colorMap, int bSwap0255)
{
	ubyte		*pSrc, *destP;
	ubyte		c, h;
	int		i, j, l, n, bBigRLE;
	ushort	nLineSize;

	static int	rleBufSize = 0;

if (!(m_info.props.flags & BM_FLAG_RLE))
	return m_info.props.h * m_info.props.rowSize;
bBigRLE = (m_info.props.flags & BM_FLAG_RLE_BIG) != 0;
if (bBigRLE)
	pSrc = Buffer () + 4 + 2 * m_info.props.h;
else
	pSrc = Buffer () + 4 + m_info.props.h;
i = 2 * (bBigRLE + 1) * m_info.props.h * m_info.props.rowSize;
if (!gameData.pig.tex.rleBuffer || (rleBufSize < i)) {
	gameData.pig.tex.rleBuffer.Resize (rleBufSize = i);
	}
if (!gameData.pig.tex.rleBuffer) {
	return -1;
	}
destP = gameData.pig.tex.rleBuffer.Buffer ();
for (i = 0; i < m_info.props.h; i++, pSrc += nLineSize) {
	if (bBigRLE)
		nLineSize = INTEL_SHORT (*(reinterpret_cast<ushort*> (Buffer () + 4 + 2 * i)));
	else
		nLineSize = Buffer () [4 + i];
	for (j = 0, n = m_info.props.w; (j < nLineSize) && n; j++) {
		h = pSrc [j];
		if (!IS_RLE_CODE (h)) {
			c = colorMap ? colorMap [h] : h; // translate
			if (bSwap0255) {
				if (c == 0)
					c = 255;
				else if (c == 255)
					c = 0;
				}
			if (destP - gameData.pig.tex.rleBuffer > m_info.props.h * m_info.props.rowSize) {
				gameData.pig.tex.rleBuffer.Destroy ();
				return -1;
				}
			*destP++ = c;
			n--;
			}
		else if ((l = (h & NOT_RLE_CODE))) {
			c = pSrc [++j];
			if (colorMap)
				c = colorMap [c];
			if (bSwap0255) {
				if (c == 0)
					c = 255;
				else if (c == 255)
					c = 0;
				}
			if (destP - gameData.pig.tex.rleBuffer + l > m_info.props.h * m_info.props.rowSize) {
				gameData.pig.tex.rleBuffer.Destroy ();
				return -1;
				}
			if (l > n)
				l = n;
			memset (destP, c, l);
			destP += l;
			n -= l;
			}
		else
			break;
		}
	if (n) {
		memset (destP, 0, n);
		destP += n;
		}
	}
l = (int) (destP - gameData.pig.tex.rleBuffer);
if (l < 0)
	return -1;
if ((l > m_info.props.h * m_info.props.rowSize) && !Resize (l))
	return -1;
memcpy (Buffer (), gameData.pig.tex.rleBuffer.Buffer (), l);
m_info.props.flags &= ~(BM_FLAG_RLE | BM_FLAG_RLE_BIG);
return l;
}

//------------------------------------------------------------------------------
// eof
