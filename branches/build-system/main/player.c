/* $Id: player.c,v 1.3 2003/10/10 09:36:35 btb Exp $ */

/*
 *
 * Player Stuff
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include "inferno.h"

#ifdef RCS
static char rcsid[] = "$Id: player.c,v 1.3 2003/10/10 09:36:35 btb Exp $";
#endif

/*
 * reads a player_ship structure from a CFILE
 */
void player_ship_read(player_ship *ps, CFILE *fp)
{
	int i;

	ps->model_num = CFReadInt(fp);
	ps->expl_vclip_num = CFReadInt(fp);
	ps->mass = CFReadFix(fp);
	ps->drag = CFReadFix(fp);
	ps->max_thrust = CFReadFix(fp);
	ps->reverse_thrust = CFReadFix(fp);
	ps->brakes = CFReadFix(fp);
	ps->wiggle = CFReadFix(fp);
	ps->max_rotthrust = CFReadFix(fp);
	for (i = 0; i < N_PLAYER_GUNS; i++)
		CFReadVector(&(ps->gun_points[i]), fp);
}
