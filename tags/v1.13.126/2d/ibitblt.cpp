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
COPYRIGHT 1993-1999 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include "inferno.h"

#ifdef __MSDOS__

#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "gr.h"
#include "mem.h"
#include "error.h"
#include "ibitblt.h"
#include "grdef.h"

#define MODE_NONE           0
#define MODE_SKIP           1
#define MODE_DRAW           2

#define OPCODE_ADD          0x81
#define OPCODE_ESI          0xC6        // Followed by a dword  (add esi, ????)
#define OPCODE_EDI          0xC7        // Followed by a dword  (add edi, ????)
#define OPCODE_MOV_ECX      0xB9        // Followed by a dword  (mov ecx,????)
#define OPCODE_MOVSB        0xA4        // movsb
#define OPCODE_16BIT        0x66        // movsw
#define OPCODE_MOVSD        0xA5        // movsd
#define OPCODE_REP          0xF3        // rep
#define OPCODE_RET          0xC3        // ret
#define OPCODE_MOV_EAX      0xB8        // mov eax, im dword
#define OPCODE_MOV_EBX      0xBB        // mov ebx, im dword
#define OPCODE_CALL_EBX1    0xFF        // call
#define OPCODE_CALL_EBX2    0xD3        //      ebx
#define OPCODE_MOV_EDI      0xBF        // mov edi, im dword


ubyte *Code_pointer = NULL;
int CodeCounter = 0;
int ibitblt_svga_page = 0;
int is_svga = 0;
uint linear_address;

void count_block( int ecx )
{
	int blocks;

	while ( ecx > 0 ) {
		switch(ecx) {
		case 1: CodeCounter++; ecx = 0; break;     // MOVSB
		case 2: CodeCounter+=2; ecx = 0; break;    // MOVSW
		case 3: CodeCounter+=3; ecx = 0; break;    // MOVSW, MOVSB
		case 4: CodeCounter++; ecx = 0; break;     // MOVSD
		default:
			blocks = ecx / 4;
			if ( blocks == 1 )
				CodeCounter++; // MOVSD
			else
				CodeCounter+=7;
			ecx -= blocks*4;
		}
	}
}


void move_andCount( int dsource, int ddest, int ecx )
{
	if ( ecx <= 0 )
		return;

	if ( dsource > 0 ) {
		// ADD ESI, dsource
		CodeCounter += 6;
	}
	if ( !is_svga ) {
		if ( ddest > 0 ) {
			// ADD EDI, ddest
			CodeCounter += 6;
		}
		count_block( ecx );
	} else {
		int p1, p2, o1;

		linear_address += ddest;        // Skip to next block

		p1 = linear_address >> 16; o1 = linear_address & 0xFFFF;
		p2 = (linear_address+ecx) >> 16;
		if ( p1 != ibitblt_svga_page ) {
			// Set page
			// MOV EAX, ?, CALL EBX
			CodeCounter += 7;
			ibitblt_svga_page = p1;
		}

		CodeCounter += 5;  // mov edi, ????

		if ( p1 == p2 ) {
			count_block( ecx );
		} else {
			int nbytes;
			nbytes = 0xFFFF-o1+1;
			count_block( nbytes );
			// set page
			// MOV EAX, 0
			CodeCounter += 7;  // mov eax,???? call ebx

			ibitblt_svga_page = p2;

			CodeCounter += 5;  // mov edi, ????

			nbytes = ecx - nbytes;
			if (nbytes > 0 )
				count_block( nbytes );
		}
		linear_address += ecx;
	}
}



void draw_block( int ecx )
{
	int blocks;
	int * iptr;

	while ( ecx > 0 )   {
		switch( ecx )   {
		case 1:
			// MOVSB
			*Code_pointer++ = OPCODE_MOVSB;
			ecx = 0;
			break;
		case 2:
			// MOVSW
			*Code_pointer++ = OPCODE_16BIT;
			*Code_pointer++ = OPCODE_MOVSD;
			ecx = 0;
			break;
		case 3:
			// MOVSW, MOVSB
			*Code_pointer++ = OPCODE_16BIT;
			*Code_pointer++ = OPCODE_MOVSD;
			*Code_pointer++ = OPCODE_MOVSB;
			ecx = 0;
			break;
		case 4:
			// MOVSD
			*Code_pointer++ = OPCODE_MOVSD;
			ecx = 0;
			break;
		default:
			blocks = ecx / 4;

			if ( blocks == 1 ) {
				// MOVSD
				*Code_pointer++ = OPCODE_MOVSD;
			} else {
				// MOV ECX, blocks
				*Code_pointer++ = OPCODE_MOV_ECX;
				iptr = (int *)Code_pointer;
				*iptr++ = blocks;
				Code_pointer = (ubyte *)iptr;
				// REP MOVSD
				*Code_pointer++ = OPCODE_REP;
				*Code_pointer++ = OPCODE_MOVSD;
			}
			ecx -= blocks*4;
		}
	}
}


void move_and_draw( int dsource, int ddest, int ecx )
{
	int * iptr;

	if ( ecx <= 0 )
		return;

	if ( dsource > 0 ) {
		// ADD ESI, dsource
		*Code_pointer++ = OPCODE_ADD;
		*Code_pointer++ = OPCODE_ESI;
		iptr = (int *)Code_pointer;
		*iptr++ = dsource;
		Code_pointer = (ubyte *)iptr;
	}
	if ( !is_svga ) {
		if ( ddest > 0 ) {
			// ADD EDI, ddest
			*Code_pointer++ = OPCODE_ADD;
			*Code_pointer++ = OPCODE_EDI;
			iptr = (int *)Code_pointer;
			*iptr++ = ddest;
			Code_pointer = (ubyte *)iptr;
		}
		draw_block( ecx );
	} else {
		unsigned int temp;
		int temp_offset;
		int p1, p2, o1;

		linear_address += ddest;        // Skip to next block

		p1 = linear_address >> 16; o1 = linear_address & 0xFFFF;
		p2 = (linear_address+ecx) >> 16;
		if ( p1 != ibitblt_svga_page ) {
			// Set page
			// MOV EAX, 0
			*Code_pointer++ = OPCODE_MOV_EAX;
			temp = p1;
			memcpy( Code_pointer, &temp, sizeof(int) );
			Code_pointer += sizeof(int);
			// CALL EBX
			*Code_pointer++ = OPCODE_CALL_EBX1;
			*Code_pointer++ = OPCODE_CALL_EBX2;
			ibitblt_svga_page = p1;
		}

		temp_offset = 0xA0000 + o1;
		*Code_pointer++ = OPCODE_MOV_EDI;
		iptr = (int *)Code_pointer;
		*iptr++ = temp_offset;
		Code_pointer = (ubyte *)iptr;

		if ( p1 == p2 ) {
			draw_block( ecx );
		} else {
			int nbytes;
			nbytes = 0xFFFF-o1+1;
			draw_block( nbytes );
			// set page
			// MOV EAX, 0
			*Code_pointer++ = OPCODE_MOV_EAX;
			temp = p2;
			memcpy( Code_pointer, &temp, sizeof(int) );
			Code_pointer += sizeof(int);
			// CALL EBX
			*Code_pointer++ = OPCODE_CALL_EBX1;
			*Code_pointer++ = OPCODE_CALL_EBX2;
			ibitblt_svga_page = p2;

			temp_offset = 0xA0000;
			*Code_pointer++ = OPCODE_MOV_EDI;
			iptr = (int *)Code_pointer;
			*iptr++ = temp_offset;
			Code_pointer = (ubyte *)iptr;

			nbytes = ecx - nbytes;
			if (nbytes > 0 )
				draw_block( nbytes );
		}
		linear_address += ecx;
	}

}

//-----------------------------------------------------------------------------------------
// Given bitmap, bmp, finds the size of the code

int gr_ibitblt_find_code_size_sub( grsBitmap * mask_bmp, int sx, int sy, int sw, int sh, int srowSize, int destType )
{
	int x,y;
	ubyte pixel;
	int draw_mode = MODE_NONE;
	int source_offset = 0;
	int dest_offset = 0;
	int num_to_draw, draw_start_source, draw_start_dest;
	int esi, edi;

	Assert( (!(mask_bmp->bmProps.flags&BM_FLAG_RLE)) );

	CodeCounter = 0;

	if ( destType == BM_SVGA ) {
		CodeCounter += 1+4;    // move ebx, gr_vesa_set_page
		CodeCounter += 1+4;    // move eax, 0
		CodeCounter += 2;      // call ebx
		ibitblt_svga_page = 0;
		linear_address = 0;
		is_svga = 1;
	} else {
		is_svga = 0;
	}

	esi = source_offset = 0;
	edi = dest_offset = 0;
	draw_start_source = draw_start_dest = 0;

	for ( y=sy; y<sy+sh; y++ ) {
		for ( x=sx; x<sx+sw; x++ ) {
			dest_offset = y*mask_bmp->bmProps.rowSize+x;
			pixel = mask_bmp->bmTexBuf[dest_offset];
			if ( pixel!=255 ) {
				switch ( draw_mode) {
				case MODE_DRAW:
					move_andCount( draw_start_source-esi, draw_start_dest-edi, num_to_draw );
					esi = draw_start_source + num_to_draw;
					edi = draw_start_dest + num_to_draw;
					// fall through!!!
				case MODE_NONE:
				case MODE_SKIP:
					break;
				}
				draw_mode = MODE_SKIP;
			} else {
				switch ( draw_mode) {
				case MODE_SKIP:
				case MODE_NONE:
					draw_start_source = source_offset;
					draw_start_dest = dest_offset;
					num_to_draw = 0;
					// fall through
				case MODE_DRAW:
					num_to_draw++;
					break;
				}
				draw_mode = MODE_DRAW;
			}
			source_offset++;
		}
		if ( draw_mode == MODE_DRAW ) {
			move_andCount( draw_start_source-esi, draw_start_dest-edi, num_to_draw );
			esi = draw_start_source + num_to_draw;
			edi = draw_start_dest + num_to_draw;
		}
		draw_mode = MODE_NONE;
		source_offset += (srowSize - sw);
	}
	CodeCounter++;     // for return

	////printf( "Code will be %d bytes\n", CodeCounter );

	CodeCounter += 16; // for safety was 16

	return CodeCounter;
}

int gr_ibitblt_find_code_size( grsBitmap * mask_bmp, int sx, int sy, int sw, int sh, int srowSize )
{
	return gr_ibitblt_find_code_size_sub( mask_bmp, sx, sy, sw, sh, srowSize, BM_LINEAR );
}

int gr_ibitblt_find_code_size_svga( grsBitmap * mask_bmp, int sx, int sy, int sw, int sh, int srowSize )
{
	return gr_ibitblt_find_code_size_sub( mask_bmp, sx, sy, sw, sh, srowSize, BM_SVGA );
}

//-----------------------------------------------------------------------------------------
// Given bitmap, bmp, create code that transfers a bitmap of size sw*sh to position
// (sx,sy) on top of bmp, only overwritting transparent pixels of the bitmap.

ubyte	*gr_ibitblt_create_mask_sub( grsBitmap * mask_bmp, int sx, int sy, int sw, int sh, int srowSize, int destType )
{
	int x,y;
	ubyte pixel;
	int draw_mode = MODE_NONE;
	int source_offset = 0;
	int dest_offset = 0;
	int num_to_draw, draw_start_source, draw_start_dest;
	int esi, edi;
	int code_size;
	ubyte *code;
	uint temp;

	Assert( (!(mask_bmp->bmProps.flags&BM_FLAG_RLE)) );

	if ( destType == BM_SVGA )
		code_size = gr_ibitblt_find_code_size_svga( mask_bmp, sx, sy, sw, sh, srowSize );
	else
		code_size = gr_ibitblt_find_code_size( mask_bmp, sx, sy, sw, sh, srowSize );

	code = D2_ALLOC( code_size );
	if ( code == NULL )
		return NULL;

	Code_pointer = code;

	if ( destType == BM_SVGA ) {
		// MOV EBX, gr_vesa_setpage
		*Code_pointer++ = OPCODE_MOV_EBX;
		temp = (uint)gr_vesa_setpage;
		memcpy( Code_pointer, &temp, sizeof(int) );
		Code_pointer += sizeof(int);
		// MOV EAX, 0
		*Code_pointer++ = OPCODE_MOV_EAX;
		temp = 0;
		memcpy( Code_pointer, &temp, sizeof(int) );
		Code_pointer += sizeof(int);
		// CALL EBX
		*Code_pointer++ = OPCODE_CALL_EBX1;
		*Code_pointer++ = OPCODE_CALL_EBX2;

		ibitblt_svga_page = 0;
		is_svga = 1;
		linear_address = 0;
	} else {
		is_svga = 0;
	}
	esi = source_offset = 0;
	edi = dest_offset = 0;
	draw_start_source = draw_start_dest = 0;

	for ( y=sy; y<sy+sh; y++ ) {
		for ( x=sx; x<sx+sw; x++ ) {
			dest_offset = y*mask_bmp->bmProps.rowSize+x;
			pixel = mask_bmp->bmTexBuf[dest_offset];
			if ( pixel!=255 ) {
				switch ( draw_mode) {
				case MODE_DRAW:
					move_and_draw( draw_start_source-esi, draw_start_dest-edi, num_to_draw );
					esi = draw_start_source + num_to_draw;
					edi = draw_start_dest + num_to_draw;
					// fall through!!!
				case MODE_NONE:
				case MODE_SKIP:
					break;
				}
				draw_mode = MODE_SKIP;
			} else {
				switch ( draw_mode) {
				case MODE_SKIP:
				case MODE_NONE:
					draw_start_source = source_offset;
					draw_start_dest = dest_offset;
					num_to_draw = 0;
					// fall through
				case MODE_DRAW:
					num_to_draw++;
					break;
				}
				draw_mode = MODE_DRAW;
			}
			source_offset++;
		}
		if ( draw_mode == MODE_DRAW ) {
			move_and_draw( draw_start_source-esi, draw_start_dest-edi, num_to_draw );
			esi = draw_start_source + num_to_draw;
			edi = draw_start_dest + num_to_draw;
		}
		draw_mode = MODE_NONE;
		source_offset += (srowSize - sw);
	}
	*Code_pointer++ = OPCODE_RET;

	if ( Code_pointer >= &code[code_size-1] )
		Error( "ibitblt overwrote allocated code block\n" );

	////printf( "Code is %d bytes\n", Code_pointer - code );

	return code;
}

ubyte   *gr_ibitblt_create_mask( grsBitmap * mask_bmp, int sx, int sy, int sw, int sh, int srowSize )
{
	return gr_ibitblt_create_mask_sub( mask_bmp, sx, sy, sw, sh, srowSize, BM_LINEAR );
}

ubyte   *gr_ibitblt_create_mask_svga( grsBitmap * mask_bmp, int sx, int sy, int sw, int sh, int srowSize )
{
	return gr_ibitblt_create_mask_sub( mask_bmp, sx, sy, sw, sh, srowSize, BM_SVGA );
}


void gr_ibitblt(grsBitmap * source_bmp, grsBitmap * dest_bmp, ubyte * mask )
{
	if (mask != NULL )
		gr_ibitblt_do_asm( source_bmp->bmTexBuf, dest_bmp->bmTexBuf, mask );
}


void    gr_ibitblt_find_hole_size( grsBitmap * mask_bmp, int *minx, int *miny, int *maxx, int *maxy )
{
	int x, y, count=0;
	ubyte c;

	Assert( (!(mask_bmp->bmProps.flags&BM_FLAG_RLE)) );

	*minx = mask_bmp->bmProps.w-1;
	*maxx = 0;
	*miny = mask_bmp->bmProps.h-1;
	*maxy = 0;

	for ( y=0; y<mask_bmp->bmProps.h; y++ )
		for ( x=0; x<mask_bmp->bmProps.w; x++ ) {
			c = mask_bmp->bmTexBuf[mask_bmp->bmProps.rowSize*y+x];
			if (c == 255 ) {
				if ( x < *minx ) *minx = x;
				if ( y < *miny ) *miny = y;
				if ( x > *maxx ) *maxx = x;
				if ( y > *maxy ) *maxy = y;
				count++;
			}
		}

	if ( count == 0 ) {
		Error( "Bitmap for ibitblt doesn't have transparency!\n" );
	}
}

#else /* __MSDOS__ */

#include "pa_enabl.h"
#include "pstypes.h"
#include "gr.h"
#include "ibitblt.h"
#include "error.h"
#include "u_mem.h"
#include "grdef.h"

#define FIND_START      1
#define FIND_STOP       2

#define MAX_WIDTH       1600
#define MAX_SCANLINES   1200
#define MAX_HOLES       5

static short start_points[MAX_SCANLINES][MAX_HOLES];
static short hole_length[MAX_SCANLINES][MAX_HOLES];
static double *scanline = NULL;

void gr_ibitblt(grsBitmap *src_bmp, grsBitmap *dest_bmp, ubyte pixel_double)
{
	int x, y, sw, sh, srowSize, drowSize, dstart, sy, dy;
	ubyte *src, *dest;
	short *current_hole, *current_hole_length;

// variable setup

	sw = src_bmp->bmProps.w;
	sh = src_bmp->bmProps.h;
	srowSize = src_bmp->bmProps.rowSize;
	drowSize = dest_bmp->bmProps.rowSize;
	src = src_bmp->bmTexBuf;
	dest = dest_bmp->bmTexBuf;

	sy = 0;
	while (start_points[sy][0] == -1) {
		sy++;
		dest += drowSize;
	}

 	if (pixel_double) {
		ubyte *scan = (ubyte *)scanline;    // set up for byte processing of scanline

		dy = sy;
		for (y = sy; y < sy + sh; y++) {
			gr_linear_rep_movsd_2x(src, scan, sw); // was: gr_linear_movsd_double(src, scan, sw*2);
			current_hole = start_points[dy];
			current_hole_length = hole_length[dy];
			for (x = 0; x < MAX_HOLES; x++) {
				if (*current_hole == -1)
					break;
				dstart = *current_hole;
				gr_linear_movsd(&(scan[dstart]), &(dest[dstart]), *current_hole_length);
				current_hole++;
				current_hole_length++;
			}
			dy++;
			dest += drowSize;
			current_hole = start_points[dy];
			current_hole_length = hole_length[dy];
			for (x = 0;x < MAX_HOLES; x++) {
				if (*current_hole == -1)
					break;
				dstart = *current_hole;
				gr_linear_movsd(&(scan[dstart]), &(dest[dstart]), *current_hole_length);
				current_hole++;
				current_hole_length++;
			}
			dy++;
			dest += drowSize;
			src += srowSize;
		}
	} else {
		Assert(sw <= MAX_WIDTH);
		Assert(sh <= MAX_SCANLINES);
		for (y = sy; y < sy + sh; y++) {
			for (x = 0; x < MAX_HOLES; x++) {
				if (start_points[y][x] == -1)
					break;
				dstart = start_points[y][x];
				gr_linear_movsd(&(src[dstart]), &(dest[dstart]), hole_length[y][x]);
			}
			dest += drowSize;
			src += srowSize;
		}
	}
}

void gr_ibitblt_create_mask(grsBitmap *mask_bmp, int sx, int sy, int sw, int sh, int srowSize)
{
	int x, y;
	ubyte mode;
	int count = 0;

	Assert( (!(mask_bmp->bmProps.flags&BM_FLAG_RLE)) );

	for (y = 0; y < MAX_SCANLINES; y++) {
		for (x = 0; x < MAX_HOLES; x++) {
			start_points[y][x] = -1;
			hole_length[y][x] = -1;
		}
	}

	for (y = sy; y < sy+sh; y++) {
		count = 0;
		mode = FIND_START;
		for (x = sx; x < sx + sw; x++) {
			if ((mode == FIND_START) && (mask_bmp->bmTexBuf[mask_bmp->bmProps.rowSize*y+x] == TRANSPARENCY_COLOR)) {
				start_points[y][count] = x;
				mode = FIND_STOP;
			} else if ((mode == FIND_STOP) && (mask_bmp->bmTexBuf[mask_bmp->bmProps.rowSize*y+x] != TRANSPARENCY_COLOR)) {
				hole_length[y][count] = x - start_points[y][count];
				count++;
				mode = FIND_START;
			}
		}
		if (mode == FIND_STOP) {
			hole_length[y][count] = x - start_points[y][count];
			count++;
		}
		Assert(count <= MAX_HOLES);
	}
}

void gr_ibitblt_find_hole_size(grsBitmap *mask_bmp, int *minx, int *miny, int *maxx, int *maxy)
{
	ubyte c;
	int x, y, count = 0;

	Assert( (!(mask_bmp->bmProps.flags&BM_FLAG_RLE)) );
	Assert( mask_bmp->bmProps.flags&BM_FLAG_TRANSPARENT );

	*minx = mask_bmp->bmProps.w - 1;
	*maxx = 0;
	*miny = mask_bmp->bmProps.h - 1;
	*maxy = 0;

	if (scanline == NULL)
		scanline = (double *)D2_ALLOC(sizeof(double) * (MAX_WIDTH / sizeof(double)));

	for (y = 0; y < mask_bmp->bmProps.h; y++) {
		for (x = 0; x < mask_bmp->bmProps.w; x++) {
			c = mask_bmp->bmTexBuf[mask_bmp->bmProps.rowSize*y+x];
			if (c == TRANSPARENCY_COLOR) { // don't look for transparancy color here.
				count++;
				if (x < *minx) *minx = x;
				if (y < *miny) *miny = y;
				if (x > *maxx) *maxx = x;
				if (y > *maxy) *maxy = y;
			}
		}
	}
	Assert (count);
}

#endif /* __MSDOS__ */
