/* $Id: netmisc.h,v 1.5 2003/10/10 09:36:35 btb Exp $ */
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

/*
 *
 * Header for netmisc.c
 *
 * Old Log:
 * Revision 1.1  1995/05/16  16:00:08  allender
 * Initial revision
 *
 * Revision 2.0  1995/02/27  11:30:18  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 *
 * Revision 1.2  1994/08/09  19:31:54  john
 * Networking changes.
 *
 * Revision 1.1  1994/08/08  11:18:40  john
 * Initial revision
 *
 *
 */

#ifndef _NETMISC_H
#define _NETMISC_H

#include "multi.h"
#include "network.h"

// Returns a checksum of a block of memory.
ushort NetMiscCalcCheckSum(void *vptr, int len);

// Finds the difference between block1 and block2.  Fills in
// diff_buffer and returns the size of diff_buffer.
extern int netmisc_find_diff(void *block1, void *block2, int block_size, void *diff_buffer);

// Applies diff_buffer to block1 to create a new block1.  Returns the
// final size of block1.
extern int netmisc_apply_diff(void *block1, void *diff_buffer, int diff_size);

void BEReceiveNetPlayerInfo (ubyte *data, netplayer_info *info);
void BEReceiveNetPlayersPacket(ubyte *data, allNetPlayers_info *pinfo);
void BESendNetPlayersPacket(ubyte *server, ubyte *node);
void BESendSequencePacket(sequence_packet seq, ubyte *server, ubyte *node, ubyte *netAddress);
void BEReceiveSequencePacket(ubyte *data, sequence_packet *seq);
void BESendNetGamePacket(ubyte *server, ubyte *node, ubyte *netAddress, int liteFlag);
void BEReceiveNetGamePacket(ubyte *data, netgame_info *netgame, int liteFlag);
void BESendExtraGameInfo(ubyte *server, ubyte *node, ubyte *netAddress);
void BEReceiveExtraGameInfo(ubyte *data, extra_gameinfo *extraGameInfo);
void BESwapObject (tObject *obj);

#if defined(WORDS_BIGENDIAN) || defined(__BIG_ENDIAN__)

#define ReceiveNetPlayerInfo BEReceiveNetPlayerInfo
#define ReceiveNetPlayersPacket BEReceiveNetPlayersPacket
#define SendNetPlayersPacket BESendNetPlayersPacket
#define SendSequencePacket BESendSequencePacket
#define ReceiveSequencePacket BEReceiveSequencePacket
#define SendNetGamePacket BESendNetGamePacket
#define ReceiveNetGamePacket BEReceiveNetGamePacket
#define SendExtraGameInfo BESendExtraGameInfo
#define ReceiveExtraGameInfo BEReceiveExtraGameInfo
#define SwapObject BESwapObject

// some mac only routines to deal with incorrectly aligned network structures

#define SendBroadcastNetPlayersPacket() \
	BESendNetPlayersPacket(NULL, NULL)
#define SendInternetSequencePacket(seq, server, node) \
	BESendSequencePacket(seq, server, node, NULL)
#define SendBroadcastSequencePacket(seq) \
	BESendSequencePacket(seq, NULL, NULL, NULL)
#define SendFullNetGamePacket(server, node, netAddress) \
	BESendNetGamePacket(server, node, netAddress, 0)
#define SendLiteNetGamePacket(server, node, netAddress) \
	BESendNetGamePacket(server, node, netAddress, 1)
#define SendInternetFullNetGamePacket(server, node) \
	BESendNetGamePacket(server, node, NULL, 0)
#define SendInternetLiteNetGamePacket(server, node) \
	BESendNetGamePacket(server, node, NULL, 1)
#define SendBroadcastFullNetGamePacket() \
	BESendNetGamePacket(NULL, NULL, NULL, 0)
#define SendBroadcastLiteNetGamePacket() \
	BESendNetGamePacket(NULL, NULL, NULL, 1)
#define ReceiveFullNetGamePacket(data, netgame) \
	BEReceiveNetGamePacket(data, netgame, 0)
#define ReceiveLiteNetGamePacket(data, netgame) \
	BEReceiveNetGamePacket(data, netgame, 1)

#define SendExtraGameInfoPacket(server, node, netAddress) \
	BESendExtraGameInfo(server, node, netAddress)
#define SendInternetExtraGameInfoPacket(server, node) \
	BESendExtraGameInfo(server, node, NULL)
#define SendBroadcastExtraGameInfoPacket() \
	BESendExtraGameInfo(NULL, NULL, NULL)
#define ReceiveExtraGameInfoPacket(data, _extraGameInfo) \
	BEReceiveExtraGameInfo(data, _extraGameInfo);

#else

#define ReceiveNetPlayersPacket(data, pinfo) \
	memcpy(pinfo, data, sizeof (allNetPlayers_info))
#define SendNetPlayersPacket(server, node) \
	IPXSendInternetPacketData((ubyte *)&netPlayers, sizeof(allNetPlayers_info), server, node)
#define SendBroadcastNetPlayersPacket() \
	IPXSendBroadcastData((ubyte *)&netPlayers, sizeof(allNetPlayers_info))

#define SendSequencePacket(seq, server, node, netAddress) \
	IPXSendPacketData((ubyte *)&seq, sizeof(sequence_packet), server, node, netAddress)
#define SendInternetSequencePacket(seq, server, node) \
	IPXSendInternetPacketData((ubyte *)&seq, sizeof(sequence_packet), server, node)
#define SendBroadcastSequencePacket(seq) \
	IPXSendBroadcastData((ubyte *)&seq, sizeof(sequence_packet))

#define SendFullNetGamePacket(server, node, netAddress) \
	IPXSendPacketData((ubyte *)&netGame, sizeof(netgame_info), server, node, netAddress)
#define SendLiteNetGamePacket(server, node, netAddress) \
	IPXSendPacketData((ubyte *)&netGame, sizeof(lite_info), server, node, netAddress)
#define SendInternetFullNetGamePacket(server, node) \
	IPXSendInternetPacketData((ubyte *)&netGame, sizeof(netgame_info), server, node)
#define SendInternetLiteNetGamePacket(server, node) \
	IPXSendInternetPacketData((ubyte *)&netGame, sizeof(lite_info), server, node)
#define SendBroadcastFullNetGamePacket() \
	IPXSendBroadcastData((ubyte *)&netGame, sizeof(netgame_info))
#define SendBroadcastLiteNetGamePacket() \
	IPXSendBroadcastData((ubyte *)&netGame, sizeof(lite_info))
#define ReceiveFullNetGamePacket(data, netgame) \
	memcpy((ubyte *)(netgame), data, sizeof(netgame_info))
#define ReceiveLiteNetGamePacket(data, netgame) \
	memcpy((ubyte *)(netgame), data, sizeof(lite_info))

#define SendExtraGameInfoPacket(server, node, netAddress) \
	IPXSendPacketData((ubyte *) (extraGameInfo + 1), sizeof(extra_gameinfo), server, node, netAddress)
#define SendInternetExtraGameInfoPacket(server, node) \
	IPXSendInternetPacketData((ubyte *)(extraGameInfo + 1), sizeof(extra_gameinfo), server, node)
#define SendBroadcastExtraGameInfoPacket() \
	IPXSendBroadcastData((ubyte *) (extraGameInfo + 1), sizeof(extra_gameinfo))
#define ReceiveExtraGameInfoPacket(data, _extraGameInfo) \
	memcpy((ubyte *)(_extraGameInfo), data, sizeof(extra_gameinfo))
#define SwapObject(obj)

#endif

#endif /* _NETMISC_H */
