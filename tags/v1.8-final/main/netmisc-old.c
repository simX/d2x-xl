/* $Id: netmisc.c,v 1.9 2003/10/04 19:13:32 btb Exp $ */
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
 * Misc routines for network.
 *
 * Old Log:
 * Revision 1.1  1995/05/16  15:28:41  allender
 * Initial revision
 *
 * Revision 2.0  1995/02/27  11:27:24  john
 * New version 2.0, which has no anonymous unions, builds with
 * Watcom 10.0, and doesn't require parsing BITMAPS.TBL.
 *
 * Revision 1.3  1994/11/19  15:19:34  mike
 * rip out unused code and data.
 *
 * Revision 1.2  1994/08/09  19:31:53  john
 * Networking changes.
 *
 * Revision 1.1  1994/08/08  11:06:07  john
 * Initial revision
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef RCS
static char rcsid[] = "$Id: netmisc.c,v 1.9 2003/10/04 19:13:32 btb Exp $";
#endif

#include <stdio.h>
#include <string.h>

#include "inferno.h"
#include "pstypes.h"
#include "mono.h"

#if defined(WORDS_BIGENDIAN) || defined(__BIG_ENDIAN__)

#include "byteswap.h"
#include "segment.h"
#include "gameseg.h"
#include "network.h"
#include "wall.h"

// routine to calculate the checksum of the segments.  We add these specialized routines
// since the current way is byte order dependent.

void BEDoCheckSumCalc(ubyte *b, int len, unsigned int *s1, unsigned int *s2)
{

	while(len--) {
		*s1 += *b++;
		if (*s1 >= 255) *s1 -= 255;
		*s2 += *s1;
	}
}

ushort BECalcSegmentCheckSum()
{
	int i, j, k;
	unsigned int sum1,sum2;
	short s;
	int t;

	sum1 = sum2 = 0;
	for (i = 0; i < gameData.segs.nLastSegment + 1; i++) {
		for (j = 0; j < MAX_SIDES_PER_SEGMENT; j++) {
			BEDoCheckSumCalc(&(gameData.segs.segments[i].sides[j].nType), 1, &sum1, &sum2);
			BEDoCheckSumCalc(&(gameData.segs.segments[i].sides[j].nFrame), 1, &sum1, &sum2);
			s = INTEL_SHORT(WallNumI (i, j));
			BEDoCheckSumCalc((ubyte *)&s, 2, &sum1, &sum2);
			s = INTEL_SHORT(gameData.segs.segments[i].sides[j].nBaseTex);
			BEDoCheckSumCalc((ubyte *)&s, 2, &sum1, &sum2);
			s = INTEL_SHORT(gameData.segs.segments[i].sides[j].nOvlTex);
			BEDoCheckSumCalc((ubyte *)&s, 2, &sum1, &sum2);
			for (k = 0; k < 4; k++) {
				t = INTEL_INT(((int)gameData.segs.segments[i].sides[j].uvls[k].u));
				BEDoCheckSumCalc((ubyte *)&t, 4, &sum1, &sum2);
				t = INTEL_INT(((int)gameData.segs.segments[i].sides[j].uvls[k].v));
				BEDoCheckSumCalc((ubyte *)&t, 4, &sum1, &sum2);
				t = INTEL_INT(((int)gameData.segs.segments[i].sides[j].uvls[k].l));
				BEDoCheckSumCalc((ubyte *)&t, 4, &sum1, &sum2);
			}
			for (k = 0; k < 2; k++) {
				t = INTEL_INT(((int)gameData.segs.segments[i].sides[j].normals[k].x));
				BEDoCheckSumCalc((ubyte *)&t, 4, &sum1, &sum2);
				t = INTEL_INT(((int)gameData.segs.segments[i].sides[j].normals[k].y));
				BEDoCheckSumCalc((ubyte *)&t, 4, &sum1, &sum2);
				t = INTEL_INT(((int)gameData.segs.segments[i].sides[j].normals[k].z));
				BEDoCheckSumCalc((ubyte *)&t, 4, &sum1, &sum2);
			}
		}
		for (j = 0; j < MAX_SIDES_PER_SEGMENT; j++) {
			s = INTEL_SHORT(gameData.segs.segments[i].children[j]);
			BEDoCheckSumCalc((ubyte *)&s, 2, &sum1, &sum2);
		}
		for (j = 0; j < MAX_VERTICES_PER_SEGMENT; j++) {
			s = INTEL_SHORT(gameData.segs.segments[i].verts[j]);
			BEDoCheckSumCalc((ubyte *)&s, 2, &sum1, &sum2);
		}
		t = INTEL_INT(gameData.segs.segments[i].objects);
		BEDoCheckSumCalc((ubyte *)&t, 4, &sum1, &sum2);
	}
	sum2 %= 255;
	return ((sum1<<8)+ sum2);
}

// this routine totally and completely relies on the fact that the network
//  checksum must be calculated on the segments!!!!!

ushort NetMiscCalcCheckSum(void * vptr, int len)
{
	vptr = vptr;
	len = len;
	return BECalcSegmentCheckSum();
}

// following are routine for macintosh only that will swap the elements of
// structures send through the networking code.  The structures and
// this code must be kept in total sync

#include "ipx.h"
#include "multi.h"
#ifdef NETWORK
#include "network.h"
#endif
#include "object.h"
#include "powerup.h"
#include "error.h"

sbyte out_buffer[IPX_MAX_DATA_SIZE];    // used for tmp netgame packets as well as sending tObject data

extern struct ipx_recv_data ipx_udpSrc;

void BEReceiveNetPlayerInfo(ubyte *data, netplayer_info *info)
{
	int loc = 0;

	memcpy(info->callsign, data + loc, CALLSIGN_LEN+1);       
	loc += CALLSIGN_LEN+1;
  memcpy(&(info->network.ipx.server), data + loc, 4);       
  loc += 4;
  memcpy(&(info->network.ipx.node), data + loc, 6);         
  loc += 6;
	info->version_major = data[loc];                            
	loc++;
	info->version_minor = data[loc];                            
	loc++;
	memcpy(&(info->computerType), data + loc, 1);            
	loc++;      // memcpy to avoid compile time warning about enum
	info->connected = data[loc];                                
	loc++;
	memcpy(&(info->socket), data + loc, 2);                   
	loc += 2;
	memcpy (&(info->rank),data + loc,1);                      
	loc++;
	// MWA don't think we need to swap this because we need it in high
	// order  info->socket = INTEL_SHORT(info->socket);
}

void BESendNetPlayersPacket(ubyte *server, ubyte *node)
{
	int i, tmpi;
	int loc = 0;
	short tmps;

	memset(out_buffer, 0, sizeof(out_buffer));
	out_buffer[0] = netPlayers.nType;                            loc++;
	tmpi = INTEL_INT(netPlayers.Security);
	memcpy(out_buffer + loc, &tmpi, 4);                       loc += 4;
	for (i = 0; i < MAX_PLAYERS+4; i++) {
		memcpy(out_buffer + loc, netPlayers.players[i].callsign, CALLSIGN_LEN+1); loc += CALLSIGN_LEN+1;
		memcpy(out_buffer + loc, netPlayers.players[i].network.ipx.server, 4);    loc += 4;
		memcpy(out_buffer + loc, netPlayers.players[i].network.ipx.node, 6);      loc += 6;
		memcpy(out_buffer + loc, &(netPlayers.players[i].version_major), 1);      loc++;
		memcpy(out_buffer + loc, &(netPlayers.players[i].version_minor), 1);      loc++;
		memcpy(out_buffer + loc, &(netPlayers.players[i].computerType), 1);      loc++;
		memcpy(out_buffer + loc, &(netPlayers.players[i].connected), 1);          loc++;
		tmps = INTEL_SHORT(netPlayers.players[i].socket);
		memcpy(out_buffer + loc, &tmps, 2);                                       loc += 2;
		memcpy(out_buffer + loc, &(netPlayers.players[i].rank), 1);               loc++;
	}

	if ((server == NULL) && (node == NULL))
		IPXSendBroadcastData(out_buffer, loc);
	else
		IPXSendInternetPacketData(out_buffer, loc, server, node);

}

void BEReceiveNetPlayersPacket(ubyte *data, allNetPlayers_info *pinfo)
{
	int i, loc = 0;

	pinfo->nType = data[loc];                            
	loc++;
	memcpy(&(pinfo->Security), data + loc, 4);        
	loc += 4;
	pinfo->Security = INTEL_INT(pinfo->Security);
	for (i = 0; i < MAX_PLAYERS+4; i++) {
		BEReceiveNetPlayerInfo(data + loc, &(pinfo->players[i]));
		loc += 26;          // sizeof(netplayer_info) on the PC
	}
}

void BESendSequencePacket(sequence_packet seq, ubyte *server, ubyte *node, ubyte *netAddress)
{
	short tmps;
	int loc, tmpi;

	loc = 0;
	memset(out_buffer, 0, sizeof(out_buffer));
	out_buffer[0] = seq.nType;                                       
	loc++;
	tmpi = INTEL_INT(seq.Security);
	memcpy(out_buffer + loc, &tmpi, 4);                           
	loc += 4;       
	loc += 3;
	memcpy(out_buffer + loc, seq.player.callsign, CALLSIGN_LEN+1);
	loc += CALLSIGN_LEN+1;
	memcpy(out_buffer + loc, seq.player.network.ipx.server, 4);   
	loc += 4;
	memcpy(out_buffer + loc, seq.player.network.ipx.node, 6);     
	loc += 6;
	out_buffer[loc] = seq.player.version_major;                     
	loc++;
	out_buffer[loc] = seq.player.version_minor;                     
	loc++;
	out_buffer[loc] = seq.player.computerType;                     
	loc++;
	out_buffer[loc] = seq.player.connected;                         
	loc++;
	tmps = INTEL_SHORT(seq.player.socket);
	memcpy(out_buffer + loc, &tmps, 2);                           
	loc += 2;
	out_buffer[loc]=seq.player.rank;                                
	loc++;      // for pad byte
	if (netAddress != NULL)
		IPXSendPacketData(out_buffer, loc, server, node, netAddress);
	else if (!server && !node)
		IPXSendBroadcastData(out_buffer, loc);
	else
		IPXSendInternetPacketData(out_buffer, loc, server, node);
}

void BEReceiveSequencePacket(ubyte *data, sequence_packet *seq)
{
	int loc = 0;

	seq->nType = data[0];                        loc++;
	memcpy(&(seq->Security), data + loc, 4);  loc += 4;   loc += 3;   // +3 for pad byte
	seq->Security = INTEL_INT(seq->Security);
	BEReceiveNetPlayerInfo(data + loc, &(seq->player));
}

void BESendNetGamePacket(ubyte *server, ubyte *node, ubyte *netAddress, int liteFlag)     // lite says shorter netgame packets
{
	uint tmpi;
	ushort tmps; // p;
	int i, j;
	int loc = 0;

	memset(out_buffer, 0, IPX_MAX_DATA_SIZE);
	memcpy(out_buffer + loc, &(netGame.nType), 1);                 
	loc++;
	tmpi = INTEL_INT(netGame.Security);
	memcpy(out_buffer + loc, &tmpi, 4);                           
	loc += 4;
	memcpy(out_buffer + loc, netGame.game_name, NETGAME_NAME_LEN+1);  
	loc += (NETGAME_NAME_LEN+1);
	memcpy(out_buffer + loc, netGame.mission_title, MISSION_NAME_LEN+1);  
	loc += (MISSION_NAME_LEN+1);
	memcpy(out_buffer + loc, netGame.mission_name, 9);            
	loc += 9;
	tmpi = INTEL_INT(netGame.levelnum);
	memcpy(out_buffer + loc, &tmpi, 4);                           
	loc += 4;
	memcpy(out_buffer + loc, &(netGame.gamemode), 1);             
	loc++;
	memcpy(out_buffer + loc, &(netGame.RefusePlayers), 1);        
	loc++;
	memcpy(out_buffer + loc, &(netGame.difficulty), 1);           
	loc++;
	memcpy(out_buffer + loc, &(netGame.game_status), 1);          
	loc++;
	memcpy(out_buffer + loc, &(netGame.numplayers), 1);           
	loc++;
	memcpy(out_buffer + loc, &(netGame.max_numplayers), 1);       
	loc++;
	memcpy(out_buffer + loc, &(netGame.numconnected), 1);         
	loc++;
	memcpy(out_buffer + loc, &(netGame.gameFlags), 1);           
	loc++;
	memcpy(out_buffer + loc, &(netGame.protocol_version), 1);     
	loc++;
	memcpy(out_buffer + loc, &(netGame.version_major), 1);        
	loc++;
	memcpy(out_buffer + loc, &(netGame.version_minor), 1);        
	loc++;
	memcpy(out_buffer + loc, &(netGame.team_vector), 1);          
	loc++;

	if (liteFlag)
		goto do_send;

// will this work -- damn bitfields -- totally bogus when trying to do
// this nType of stuff
// Watcom makes bitfields from left to right.  CW7 on the mac goes
// from right to left.  then they are endian swapped

	tmps = *(ushort *)((ubyte *)(&netGame.team_vector) + 1);    // get the values for the first short bitfield
	tmps = INTEL_SHORT(tmps);
	memcpy(out_buffer + loc, &tmps, 2);                           
	loc += 2;

	tmps = *(ushort *)((ubyte *)(&netGame.team_vector) + 3);    // get the values for the second short bitfield
	tmps = INTEL_SHORT(tmps);
	memcpy(out_buffer + loc, &tmps, 2);                           
	loc += 2;

#if 0       // removed since I reordered bitfields on mac
	p = *(ushort *)((ubyte *)(&netGame.team_vector) + 1);       // get the values for the first short bitfield
	tmps = 0;
	for (i = 15; i >= 0; i--) {
		if (p & (1 << i))
			tmps |= (1 << (15 - i);
	}
	tmps = INTEL_SHORT(tmps);
	memcpy(out_buffer + loc, &tmps, 2);                           
	loc += 2;
	p = *(ushort *)((ubyte *)(&netGame.team_vector) + 3);       // get the values for the second short bitfield
	tmps = 0;
	for (i = 15; i >= 0; i--) {
		if (p & (1 << i))
			tmps |= (1 << (15 - i);
	}
	tmps = INTEL_SHORT(tmps);
	memcpy(out_buffer + loc, &tmps, 2);                           
	loc += 2;
#endif

	memcpy(out_buffer + loc, netGame.team_name, 2*(CALLSIGN_LEN+1)); loc += 2*(CALLSIGN_LEN+1);
	for (i = 0; i < MAX_PLAYERS; i++) {
		tmpi = INTEL_INT(netGame.locations[i]);
		memcpy(out_buffer + loc, &tmpi, 4);       
		loc += 4;   // SWAP HERE!!!
	}

	for (i = 0; i < MAX_PLAYERS; i++) {
		for (j = 0; j < MAX_PLAYERS; j++) {
			tmps = INTEL_SHORT(netGame.kills[i][j]);
			memcpy(out_buffer + loc, &tmps, 2);   
			loc += 2;   // SWAP HERE!!!
		}
	}

	tmps = INTEL_SHORT(netGame.segments_checksum);
	memcpy(out_buffer + loc, &tmps, 2);           
	loc += 2;   // SWAP_HERE
	tmps = INTEL_SHORT(netGame.teamKills[0]);
	memcpy(out_buffer + loc, &tmps, 2);           
	loc += 2;   // SWAP_HERE
	tmps = INTEL_SHORT(netGame.teamKills[1]);
	memcpy(out_buffer + loc, &tmps, 2);           
	loc += 2;   // SWAP_HERE
	for (i = 0; i < MAX_PLAYERS; i++) {
		tmps = INTEL_SHORT(netGame.killed[i]);
		memcpy(out_buffer + loc, &tmps, 2);       
		loc += 2;   // SWAP HERE!!!
	}
	for (i = 0; i < MAX_PLAYERS; i++) {
		tmps = INTEL_SHORT(netGame.playerKills[i]);
		memcpy(out_buffer + loc, &tmps, 2);       
		loc += 2;   // SWAP HERE!!!
	}

	tmpi = INTEL_INT(netGame.KillGoal);
	memcpy(out_buffer + loc, &tmpi, 4);           
	loc += 4;   // SWAP_HERE
	tmpi = INTEL_INT(netGame.PlayTimeAllowed);
	memcpy(out_buffer + loc, &tmpi, 4);           
	loc += 4;   // SWAP_HERE
	tmpi = INTEL_INT(netGame.levelTime);
	memcpy(out_buffer + loc, &tmpi, 4);           
	loc += 4;   // SWAP_HERE
	tmpi = INTEL_INT(netGame.control_invulTime);
	memcpy(out_buffer + loc, &tmpi, 4);           
	loc += 4;   // SWAP_HERE
	tmpi = INTEL_INT(netGame.monitor_vector);
	memcpy(out_buffer + loc, &tmpi, 4);           
	loc += 4;   // SWAP_HERE
	for (i = 0; i < MAX_PLAYERS; i++) {
		tmpi = INTEL_INT(netGame.player_score[i]);
		memcpy(out_buffer + loc, &tmpi, 4);       
		loc += 4;   // SWAP_HERE
	}
	for (i = 0; i < MAX_PLAYERS; i++) {
		memcpy(out_buffer + loc, &(netGame.playerFlags[i]), 1); loc++;
	}
	tmps = INTEL_SHORT(netGame.nPacketsPerSec);
	memcpy(out_buffer + loc, &tmps, 2);                   
	loc += 2;
	memcpy(out_buffer + loc, &(netGame.bShortPackets), 1); 
	loc++;

do_send:
	if (netAddress != NULL)
		IPXSendPacketData(out_buffer, loc, server, node, netAddress);
	else if ((server == NULL) && (node == NULL))
		IPXSendBroadcastData(out_buffer, loc);
	else
		IPXSendInternetPacketData(out_buffer, loc, server, node);
}

void BEReceiveNetGamePacket(ubyte *data, netgame_info *netgame, int liteFlag)
{
	int i, j;
	int loc = 0;
	short bitfield; // new_field;

	memcpy(&(netgame->nType), data + loc, 1);                      
	loc++;
	memcpy(&(netgame->Security), data + loc, 4);                  
	loc += 4;
	netgame->Security = INTEL_INT(netgame->Security);
	memcpy(netgame->game_name, data + loc, NETGAME_NAME_LEN+1);   
	loc += (NETGAME_NAME_LEN+1);
	memcpy(netgame->mission_title, data + loc, MISSION_NAME_LEN+1); 
	loc += (MISSION_NAME_LEN+1);
	memcpy(netgame->mission_name, data + loc, 9);                 
	loc += 9;
	memcpy(&(netgame->levelnum), data + loc, 4);                  
	loc += 4;
	netgame->levelnum = INTEL_INT(netgame->levelnum);
	memcpy(&(netgame->gamemode), data + loc, 1);                  
	loc++;
	memcpy(&(netgame->RefusePlayers), data + loc, 1);             
	loc++;
	memcpy(&(netgame->difficulty), data + loc, 1);                
	loc++;
	memcpy(&(netgame->game_status), data + loc, 1);               
	loc++;
	memcpy(&(netgame->numplayers), data + loc, 1);                
	loc++;
	memcpy(&(netgame->max_numplayers), data + loc, 1);            
	loc++;
	memcpy(&(netgame->numconnected), data + loc, 1);              
	loc++;
	memcpy(&(netgame->gameFlags), data + loc, 1);                
	loc++;
	memcpy(&(netgame->protocol_version), data + loc, 1);          
	loc++;
	memcpy(&(netgame->version_major), data + loc, 1);             
	loc++;
	memcpy(&(netgame->version_minor), data + loc, 1);             
	loc++;
	memcpy(&(netgame->team_vector), data + loc, 1);               
	loc++;

	if (liteFlag)
		return;

	memcpy(&bitfield, data + loc, 2);                             
	loc += 2;
	bitfield = INTEL_SHORT(bitfield);
	memcpy(((ubyte *)(&netgame->team_vector) + 1), &bitfield, 2);

	memcpy(&bitfield, data + loc, 2);                             
	loc += 2;
	bitfield = INTEL_SHORT(bitfield);
	memcpy(((ubyte *)(&netgame->team_vector) + 3), &bitfield, 2);

#if 0       // not used since reordering mac bitfields
	memcpy(&bitfield, data + loc, 2);                             
	loc += 2;
	new_field = 0;
	for (i = 15; i >= 0; i--) {
		if (bitfield & (1 << i))
			new_field |= (1 << (15 - i);
	}
	new_field = INTEL_SHORT(new_field);
	memcpy(((ubyte *)(&netgame->team_vector) + 1), &new_field, 2);

	memcpy(&bitfield, data + loc, 2);                             
	loc += 2;
	new_field = 0;
	for (i = 15; i >= 0; i--) {
		if (bitfield & (1 << i))
			new_field |= (1 << (15 - i);
	}
	new_field = INTEL_SHORT(new_field);
	memcpy(((ubyte *)(&netgame->team_vector) + 3), &new_field, 2);
#endif

	memcpy(netgame->team_name, data + loc, 2*(CALLSIGN_LEN+1));   
	loc += 2*(CALLSIGN_LEN+1);
	for (i = 0; i < MAX_PLAYERS; i++) {
		memcpy(&(netgame->locations[i]), data + loc, 4);          
		loc += 4;
		netgame->locations[i] = INTEL_INT(netgame->locations[i]);
	}

	for (i = 0; i < MAX_PLAYERS; i++) {
		for (j = 0; j < MAX_PLAYERS; j++) {
			memcpy(&(netgame->kills[i][j]), data + loc, 2);       
			loc += 2;
			netgame->kills[i][j] = INTEL_SHORT(netgame->kills[i][j]);
		}
	}

	memcpy(&(netgame->segments_checksum), data + loc, 2);         
	loc += 2;
	netgame->segments_checksum = INTEL_SHORT(netgame->segments_checksum);
	memcpy(&(netgame->teamKills[0]), data + loc, 2);             
	loc += 2;
	netgame->teamKills[0] = INTEL_SHORT(netgame->teamKills[0]);
	memcpy(&(netgame->teamKills[1]), data + loc, 2);             
	loc += 2;
	netgame->teamKills[1] = INTEL_SHORT(netgame->teamKills[1]);
	for (i = 0; i < MAX_PLAYERS; i++) {
		memcpy(&(netgame->killed[i]), data + loc, 2);             
		loc += 2;
		netgame->killed[i] = INTEL_SHORT(netgame->killed[i]);
	}
	for (i = 0; i < MAX_PLAYERS; i++) {
		memcpy(&(netgame->playerKills[i]), data + loc, 2);       
		loc += 2;
		netgame->playerKills[i] = INTEL_SHORT(netgame->playerKills[i]);
	}
	memcpy(&(netgame->KillGoal), data + loc, 4);                  
	loc += 4;
	netgame->KillGoal = INTEL_INT(netgame->KillGoal);
	memcpy(&(netgame->PlayTimeAllowed), data + loc, 4);           
	loc += 4;
	netgame->PlayTimeAllowed = INTEL_INT(netgame->PlayTimeAllowed);

	memcpy(&(netgame->levelTime), data + loc, 4);                
	loc += 4;
	netgame->levelTime = INTEL_INT(netgame->levelTime);
	memcpy(&(netgame->control_invulTime), data + loc, 4);        
	loc += 4;
	netgame->control_invulTime = INTEL_INT(netgame->control_invulTime);
	memcpy(&(netgame->monitor_vector), data + loc, 4);            
	loc += 4;
	netgame->monitor_vector = INTEL_INT(netgame->monitor_vector);
	for (i = 0; i < MAX_PLAYERS; i++) {
		memcpy(&(netgame->player_score[i]), data + loc, 4);       
		loc += 4;
		netgame->player_score[i] = INTEL_INT(netgame->player_score[i]);
	}
	for (i = 0; i < MAX_PLAYERS; i++) {
		memcpy(&(netgame->playerFlags[i]), data + loc, 1);       
		loc++;
	}
	memcpy(&(netgame->nPacketsPerSec), data + loc, 2);             
	loc += 2;
	netgame->nPacketsPerSec = INTEL_SHORT(netgame->nPacketsPerSec);
	memcpy(&(netgame->bShortPackets), data + loc, 1);              
	loc ++;

}

#define EGI_INTEL_SHORT_2BUF(_m) \
  *((short *) (out_buffer + ((char *) &extraGameInfo [1]. _m - (char *) &extraGameInfo [1]))) = INTEL_SHORT (extraGameInfo [1]. _m);

#define EGI_INTEL_INT_2BUF(_m) \
	*((int *) (out_buffer + ((char *) &extraGameInfo [1]. _m - (char *) &extraGameInfo [1]))) = INTEL_INT (extraGameInfo [1]. _m);

#define BUF2_EGI_INTEL_SHORT(_m) \
	extraGameInfo [1]. _m = INTEL_SHORT (*((short *) (out_buffer + ((char *) &extraGameInfo [1]. _m - (char *) &extraGameInfo [1]))));

#define BUF2_EGI_INTEL_INT(_m) \
	extraGameInfo [1]. _m = INTEL_INT (*((int *) (out_buffer + ((char *) &extraGameInfo [1]. _m - (char *) &extraGameInfo [1]))));
	
void BESendExtraGameInfo(ubyte *server, ubyte *node, ubyte *netAddress)
{
memcpy (out_buffer, &extraGameInfo [1], sizeof (extra_gameinfo));
EGI_INTEL_SHORT_2BUF (entropy.nMaxVirusCapacity);
EGI_INTEL_SHORT_2BUF (entropy.nEnergyFillRate);
EGI_INTEL_SHORT_2BUF (entropy.nShieldFillRate);
EGI_INTEL_SHORT_2BUF (entropy.nShieldDamageRate);
EGI_INTEL_INT_2BUF (nSpawnDelay);
if (netAddress != NULL)
	IPXSendPacketData(out_buffer, sizeof (extra_gameinfo), server, node, netAddress);
else if ((server == NULL) && (node == NULL))
	IPXSendBroadcastData(out_buffer, sizeof (extra_gameinfo));
else
	IPXSendInternetPacketData(out_buffer, sizeof (extra_gameinfo), server, node);
}


void BEReceiveExtraGameInfo(ubyte *data, extra_gameinfo *extraGameInfo)
{
memcpy (&extraGameInfo [1], data, sizeof (extra_gameinfo));
BUF2_EGI_INTEL_SHORT (entropy.nMaxVirusCapacity);
BUF2_EGI_INTEL_SHORT (entropy.nEnergyFillRate);
BUF2_EGI_INTEL_SHORT (entropy.nShieldFillRate);
BUF2_EGI_INTEL_SHORT (entropy.nShieldDamageRate);
BUF2_EGI_INTEL_INT (nSpawnDelay);
}


void BESwapObject(tObject *objP)
{
	// swap the short and int entries for this tObject
	objP->nSignature     = INTEL_INT(objP->nSignature);
	objP->next          = INTEL_SHORT(objP->next);
	objP->prev          = INTEL_SHORT(objP->prev);
	objP->nSegment        = INTEL_SHORT(objP->nSegment);
	objP->pos.x         = INTEL_INT(objP->pos.x);
	objP->pos.y         = INTEL_INT(objP->pos.y);
	objP->pos.z         = INTEL_INT(objP->pos.z);

	objP->orient.rVec.x = INTEL_INT(objP->orient.rVec.x);
	objP->orient.rVec.y = INTEL_INT(objP->orient.rVec.y);
	objP->orient.rVec.z = INTEL_INT(objP->orient.rVec.z);
	objP->orient.fVec.x = INTEL_INT(objP->orient.fVec.x);
	objP->orient.fVec.y = INTEL_INT(objP->orient.fVec.y);
	objP->orient.fVec.z = INTEL_INT(objP->orient.fVec.z);
	objP->orient.uVec.x = INTEL_INT(objP->orient.uVec.x);
	objP->orient.uVec.y = INTEL_INT(objP->orient.uVec.y);
	objP->orient.uVec.z = INTEL_INT(objP->orient.uVec.z);

	objP->size          = INTEL_INT(objP->size);
	objP->shields       = INTEL_INT(objP->shields);

	objP->last_pos.x    = INTEL_INT(objP->last_pos.x);
	objP->last_pos.y    = INTEL_INT(objP->last_pos.y);
	objP->last_pos.z    = INTEL_INT(objP->last_pos.z);

	objP->lifeleft      = INTEL_INT(objP->lifeleft);

	switch (objP->movementType) {

	case MT_PHYSICS:

		objP->mType.physInfo.velocity.x = INTEL_INT(objP->mType.physInfo.velocity.x);
		objP->mType.physInfo.velocity.y = INTEL_INT(objP->mType.physInfo.velocity.y);
		objP->mType.physInfo.velocity.z = INTEL_INT(objP->mType.physInfo.velocity.z);

		objP->mType.physInfo.thrust.x   = INTEL_INT(objP->mType.physInfo.thrust.x);
		objP->mType.physInfo.thrust.y   = INTEL_INT(objP->mType.physInfo.thrust.y);
		objP->mType.physInfo.thrust.z   = INTEL_INT(objP->mType.physInfo.thrust.z);

		objP->mType.physInfo.mass       = INTEL_INT(objP->mType.physInfo.mass);
		objP->mType.physInfo.drag       = INTEL_INT(objP->mType.physInfo.drag);
		objP->mType.physInfo.brakes     = INTEL_INT(objP->mType.physInfo.brakes);

		objP->mType.physInfo.rotVel.x   = INTEL_INT(objP->mType.physInfo.rotVel.x);
		objP->mType.physInfo.rotVel.y   = INTEL_INT(objP->mType.physInfo.rotVel.y);
		objP->mType.physInfo.rotVel.z   = INTEL_INT(objP->mType.physInfo.rotVel.z);

		objP->mType.physInfo.rotThrust.x = INTEL_INT(objP->mType.physInfo.rotThrust.x);
		objP->mType.physInfo.rotThrust.y = INTEL_INT(objP->mType.physInfo.rotThrust.y);
		objP->mType.physInfo.rotThrust.z = INTEL_INT(objP->mType.physInfo.rotThrust.z);

		objP->mType.physInfo.turnRoll   = INTEL_INT(objP->mType.physInfo.turnRoll);
		objP->mType.physInfo.flags      = INTEL_SHORT(objP->mType.physInfo.flags);

		break;

	case MT_SPINNING:

		objP->mType.spinRate.x = INTEL_INT(objP->mType.spinRate.x);
		objP->mType.spinRate.y = INTEL_INT(objP->mType.spinRate.y);
		objP->mType.spinRate.z = INTEL_INT(objP->mType.spinRate.z);
		break;
	}

	switch (objP->controlType) {

	case CT_WEAPON:
		objP->cType.laserInfo.parentType       = INTEL_SHORT(objP->cType.laserInfo.parentType);
		objP->cType.laserInfo.nParentObj        = INTEL_SHORT(objP->cType.laserInfo.nParentObj);
		objP->cType.laserInfo.nParentSig  = INTEL_INT(objP->cType.laserInfo.nParentSig);
		objP->cType.laserInfo.creationTime     = INTEL_INT(objP->cType.laserInfo.creationTime);
		objP->cType.laserInfo.nLastHitObj       = INTEL_SHORT(objP->cType.laserInfo.nLastHitObj);
		objP->cType.laserInfo.nTrackGoal        = INTEL_SHORT(objP->cType.laserInfo.nTrackGoal);
		objP->cType.laserInfo.multiplier        = INTEL_INT(objP->cType.laserInfo.multiplier);
		break;

	case CT_EXPLOSION:
		objP->cType.explInfo.nSpawnTime     = INTEL_INT(objP->cType.explInfo.nSpawnTime);
		objP->cType.explInfo.nDeleteTime    = INTEL_INT(objP->cType.explInfo.nDeleteTime);
		objP->cType.explInfo.nDeleteObj  = INTEL_SHORT(objP->cType.explInfo.nDeleteObj);
		objP->cType.explInfo.nAttachParent  = INTEL_SHORT(objP->cType.explInfo.nAttachParent);
		objP->cType.explInfo.nPrevAttach    = INTEL_SHORT(objP->cType.explInfo.nPrevAttach);
		objP->cType.explInfo.nNextAttach    = INTEL_SHORT(objP->cType.explInfo.nNextAttach);
		break;

	case CT_AI:
		objP->cType.aiInfo.nHideSegment         = INTEL_SHORT(objP->cType.aiInfo.nHideSegment);
		objP->cType.aiInfo.nHideIndex           = INTEL_SHORT(objP->cType.aiInfo.nHideIndex);
		objP->cType.aiInfo.nPathLength          = INTEL_SHORT(objP->cType.aiInfo.nPathLength);
		objP->cType.aiInfo.nDangerLaser     = INTEL_SHORT(objP->cType.aiInfo.nDangerLaser);
		objP->cType.aiInfo.nDangerLaserSig = INTEL_INT(objP->cType.aiInfo.nDangerLaserSig);
		objP->cType.aiInfo.xDyingStartTime     = INTEL_INT(objP->cType.aiInfo.xDyingStartTime);
		break;

	case CT_LIGHT:
		objP->cType.lightInfo.intensity = INTEL_INT(objP->cType.lightInfo.intensity);
		break;

	case CT_POWERUP:
		objP->cType.powerupInfo.count = INTEL_INT(objP->cType.powerupInfo.count);
		objP->cType.powerupInfo.creationTime = INTEL_INT(objP->cType.powerupInfo.creationTime);
		// Below commented out 5/2/96 by Matt.  I asked Allender why it was
		// here, and he didn't know, and it looks like it doesn't belong.
		// if (objP->id == POW_VULCAN_WEAPON)
		// objP->cType.powerupInfo.count = VULCAN_WEAPON_AMMO_AMOUNT;
		break;

	}

	switch (objP->renderType) {

	case RT_MORPH:
	case RT_POLYOBJ: {
		int i;

		objP->rType.polyObjInfo.nModel      = INTEL_INT(objP->rType.polyObjInfo.nModel);

		for (i=0;i<MAX_SUBMODELS;i++) {
			objP->rType.polyObjInfo.animAngles[i].p = INTEL_INT(objP->rType.polyObjInfo.animAngles[i].p);
			objP->rType.polyObjInfo.animAngles[i].b = INTEL_INT(objP->rType.polyObjInfo.animAngles[i].b);
			objP->rType.polyObjInfo.animAngles[i].h = INTEL_INT(objP->rType.polyObjInfo.animAngles[i].h);
		}

		objP->rType.polyObjInfo.nSubObjFlags   = INTEL_INT(objP->rType.polyObjInfo.nSubObjFlags);
		objP->rType.polyObjInfo.nTexOverride  = INTEL_INT(objP->rType.polyObjInfo.nTexOverride);
		objP->rType.polyObjInfo.nAltTextures   = INTEL_INT(objP->rType.polyObjInfo.nAltTextures);
		break;
	}

	case RT_WEAPON_VCLIP:
	case RT_HOSTAGE:
	case RT_POWERUP:
	case RT_FIREBALL:
	case RT_THRUSTER:
		objP->rType.vClipInfo.nClipIndex = INTEL_INT(objP->rType.vClipInfo.nClipIndex);
		objP->rType.vClipInfo.xFrameTime = INTEL_INT(objP->rType.vClipInfo.xFrameTime);
		break;

	case RT_LASER:
		break;

	}
//  END OF SWAPPING OBJECT STRUCTURE

}

#else /* !WORDS_BIGENDIAN */


// Calculates the checksum of a block of memory.
ushort NetMiscCalcCheckSum(void * vptr, int len)
{
	ubyte *ptr = (ubyte *)vptr;
	unsigned int sum1,sum2;

	sum1 = sum2 = 0;

	for (; len; len--) {
		sum1 += *ptr++;
		if (sum1 >= 255)
			sum1 -= 255;
		sum2 += sum1;
	}
return (sum1 * 256 + sum2 % 255);
}

#endif /* WORDS_BIGENDIAN */

// needs to be recoded to actually work with big endian!
//--unused-- //Finds the difference between block1 and block2.  Fills in diff_buffer and
//--unused-- //returns the size of diff_buffer.
//--unused-- int netmisc_find_diff(void *block1, void *block2, int block_size, void *diff_buffer)
//--unused-- {
//--unused-- 	int mode;
//--unused-- 	ushort *c1, *c2, *diff_start, *c3;
//--unused-- 	int i, j, size, diff, n , same;
//--unused--
//--unused-- 	size=(block_size+1)/sizeof(ushort);
//--unused-- 	c1 = (ushort *)block1;
//--unused-- 	c2 = (ushort *)block2;
//--unused-- 	c3 = (ushort *)diff_buffer;
//--unused--
//--unused-- 	mode = same = diff = n = 0;
//--unused--
//--unused-- 	for (i=0; i<size; i++, c1++, c2++) {
//--unused-- 		if (*c1 != *c2) {
//--unused-- 			if (mode==0) {
//--unused-- 				mode = 1;
//--unused-- 				c3[n++] = same;
//--unused-- 				same=0; diff=0;
//--unused-- 				diff_start = c2;
//--unused-- 			}
//--unused-- 			*c1 = *c2;
//--unused-- 			diff++;
//--unused-- 			if (diff==65535) {
//--unused-- 				mode = 0;
//--unused-- 				// send how many diff ones.
//--unused-- 				c3[n++]=diff;
//--unused-- 				// send all the diff ones.
//--unused-- 				for (j=0; j<diff; j++)
//--unused-- 					c3[n++] = diff_start[j];
//--unused-- 				same=0; diff=0;
//--unused-- 				diff_start = c2;
//--unused-- 			}
//--unused-- 		} else {
//--unused-- 			if (mode==1) {
//--unused-- 				mode=0;
//--unused-- 				// send how many diff ones.
//--unused-- 				c3[n++]=diff;
//--unused-- 				// send all the diff ones.
//--unused-- 				for (j=0; j<diff; j++)
//--unused-- 					c3[n++] = diff_start[j];
//--unused-- 				same=0; diff=0;
//--unused-- 				diff_start = c2;
//--unused-- 			}
//--unused-- 			same++;
//--unused-- 			if (same==65535) {
//--unused-- 				mode=1;
//--unused-- 				// send how many the same
//--unused-- 				c3[n++] = same;
//--unused-- 				same=0; diff=0;
//--unused-- 				diff_start = c2;
//--unused-- 			}
//--unused-- 		}
//--unused--
//--unused-- 	}
//--unused-- 	if (mode==0) {
//--unused-- 		// send how many the same
//--unused-- 		c3[n++] = same;
//--unused-- 	} else {
//--unused-- 		// send how many diff ones.
//--unused-- 		c3[n++]=diff;
//--unused-- 		// send all the diff ones.
//--unused-- 		for (j=0; j<diff; j++)
//--unused-- 			c3[n++] = diff_start[j];
//--unused-- 	}
//--unused--
//--unused-- 	return n*2;
//--unused-- }

//--unused-- //Applies diff_buffer to block1 to create a new block1.  Returns the final
//--unused-- //size of block1.
//--unused-- int netmisc_apply_diff(void *block1, void *diff_buffer, int diff_size)
//--unused-- {
//--unused-- 	unsigned int i, j, n, size;
//--unused-- 	ushort *c1, *c2;
//--unused--
//--unused-- 	c1 = (ushort *)diff_buffer;
//--unused-- 	c2 = (ushort *)block1;
//--unused--
//--unused-- 	size = diff_size/2;
//--unused--
//--unused-- 	i=j=0;
//--unused-- 	while (1) {
//--unused-- 		j += c1[i];         // Same
//--unused-- 		i++;
//--unused-- 		if (i>=size) break;
//--unused-- 		n = c1[i];          // ndiff
//--unused-- 		i++;
//--unused-- 		if (n>0) {
//--unused-- 			//Assert(n* < 256);
//--unused-- 			memcpy(&c2[j], &c1[i], n*2);
//--unused-- 			i += n;
//--unused-- 			j += n;
//--unused-- 		}
//--unused-- 		if (i>=size) break;
//--unused-- 	}
//--unused-- 	return j*2;
//--unused-- }
