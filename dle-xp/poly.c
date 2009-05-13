#include <windows.h>            // Window defines
#include <stdlib.h>
#include <assert.h>
#include <mem.h>
#include <math.h>
#include "gl.h"                 // OpenGL
#include "glu.h"                // GLU library
#include "types.h"
#include "poly.h"

#define ENABLE_TEXTURE_MAPPING TRUE

extern void calcNormal(float v[3][3], float out[3]);


/*
//-----------------------------------------------------------------------
// CONSTANTS
//-----------------------------------------------------------------------
*/

#define OP_EOF        0	/* eof                                 */
#define OP_DEFPOINTS  1	/* defpoints                (not used) */
#define OP_FLATPOLY   2	/* flat-shaded polygon                 */
#define OP_TMAPPOLY   3	/* texture-mapped polygon              */
#define OP_SORTNORM   4	/* sort by normal                      */
#define OP_RODBM      5	/* rod bitmap               (not used) */
#define OP_SUBCALL    6	/* call a subobject                    */
#define OP_DEFP_START 7	/* defpoints with start                */
#define OP_GLOW       8	/* glow value for next poly            */

#define MAX_INTERP_COLORS 100
#define MAX_POINTS_PER_POLY 25

/*
//-----------------------------------------------------------------------
// MACROS
//-----------------------------------------------------------------------
*/
#define W(p)   (*((INT16 *)(p)))
#define WP(p)  ((INT16 *)(p))
#define VP(p)  ((VMS_VECTOR *)(p))

/*
//-----------------------------------------------------------------------
//
// Calls the object interpreter to render an object.
// The object renderer is really a seperate pipeline. returns true if drew
//
//-----------------------------------------------------------------------
*/
void interp_model_data(
    void *model_data,
    VMS_VECTOR *offset,
    MODEL *model,
	UINT16 call_level
) {

	UINT8 *p = model_data;
	INT16 glow_num = -1;
	float normal[3];	// Storeage for calculated surface normal
	float v[MAX_POLY_POINTS][3];
	float uv[MAX_POLY_POINTS][2];

	assert(model_data);
	assert(model);
	assert(model->polys);

	if (call_level == 0) {
	  model->n_polys = 0;
	  model->n_points = 0;
	}

	/*
	// UINT16 Type (see below)
	*/

	while (W(p) != OP_EOF) {
		switch (W(p)) {

			/*
			// Point Definitions with Start Offset:
			//
			// 2  UINT16      n_points       number of points
			// 4  UINT16      start_point    starting point
			// 6  UINT16      unknown
			// 8  VMS_VECTOR  pts[n_points]  x,y,z data
			*/
			case OP_DEFP_START: {
				UINTW pt,pt0,n_points;
				pt0 = W(p+4);
				n_points = W(p+2);
				model->n_points += n_points;
				assert(W(p+6)==0);
				assert(model->n_points < MAX_POLY_MODEL_POINTS);
				assert(pt0+n_points < MAX_POLY_MODEL_POINTS);
				for (pt=0;pt< n_points;pt++) {
					model->points[pt+pt0].x = VP(p+8)[pt].x + offset->x;
					model->points[pt+pt0].y = VP(p+8)[pt].y + offset->y;
					model->points[pt+pt0].z = VP(p+8)[pt].z + offset->z;

				}
				p += W(p+2)*sizeof(VMS_VECTOR) + 8;
				break;
			}

			/*
			// Flat Shaded Polygon:
			//
			// 2  UINT16     n_verts
			// 4  VMS_VECTOR vector1
			// 16 VMS_VECTOR vector2
			// 28 UINT16     color
			// 30 UINT16     verts[n_verts]
			//
			*/
			case OP_FLATPOLY: {
				UINT16 nv = W(p+2);
				POLY *panel = &model->polys[model->n_polys];
				UINT16 i;
				assert(nv>=MIN_POLY_POINTS);
				assert(nv<=MAX_POLY_POINTS);
				assert(model->n_polys < MAX_POLYS);
				panel->n_verts  = nv;
				panel->offset   = *VP(p+4);
				panel->normal   = *VP(p+16);
				panel->color    = W(p+28);
				panel->tmap_num = -1;
				panel->glow_num = glow_num;
				for (i=0;i<nv;i++) {
					panel->verts[i] = WP(p+30)[i];
				}
				for (i=0;i<nv;i++) {
					panel->uvls[i].u  = 0;
					panel->uvls[i].v  = 0;
					panel->uvls[i].l  = 0;
				}
				model->n_polys ++;

				for (i=0;i<nv;i++) {
					v[i][0] = ((float)model->points[panel->verts[i]].x) / 0x1000;
					v[i][1] = ((float)model->points[panel->verts[i]].y) / 0x1000;
					v[i][2] = ((float)model->points[panel->verts[i]].z) / 0x1000;
				}
				calcNormal(v,normal);
#if 0
				glColor3ub(255, 0, 0);

				glBegin(GL_POLYGON);
				assert(nv<=MAX_POLY_POINTS);
				for (i=0;i<nv;i++) {
					glVertex3fv(v[i]);
				}
				glEnd();
#endif

				p += 30 + ((nv&~1)+1)*2;
				break;
			}

			/*
			// Texture Mapped Polygon:
			//
			// 2  UINT16     n_verts
			// 4  VMS_VECTOR vector1
			// 16 VMS_VECTOR vector2
			// 28 UINT16     tmap_num
			// 30 UINT16     verts[n_verts]
			// -- UVL        uvls[n_verts]
			*/
			case OP_TMAPPOLY: {
				UINT16 nv = W(p+2);
				POLY *panel = &model->polys[model->n_polys];
				UINT16 i;
				assert(nv>=MIN_POLY_POINTS);
				assert(nv<=MAX_POLY_POINTS);
				assert(model->n_polys < MAX_POLYS);
				panel->n_verts  = nv;
				panel->offset   = *VP(p+4);
				panel->normal   = *VP(p+16);
				panel->color    = -1;
				panel->tmap_num = W(p+28);
				panel->glow_num = glow_num;
				for (i=0;i<nv;i++) {
					panel->verts[i] = WP(p+30)[i];
				}

				p += 30 + ((nv&~1)+1)*2;

				for (i=0;i<nv;i++) {
					panel->uvls[i].u  = ((UVL*)(p))[i].u;
					panel->uvls[i].v  = ((UVL*)(p))[i].v;
					panel->uvls[i].l  = ((UVL*)(p))[i].l;
				}
				model->n_polys ++;

				for (i=0;i<nv;i++) {
					v[i][0] = ((float)model->points[panel->verts[i]].x) / 0x1000;
					v[i][1] = ((float)model->points[panel->verts[i]].y) / 0x1000;
					v[i][2] = ((float)model->points[panel->verts[i]].z) / 0x1000;
				}

				for (i=0;i<nv;i++) {
					uv[i][0] = ((float)panel->uvls[i].u) / (16*0x1000);
					uv[i][1] = ((float)panel->uvls[i].v) / (16*0x1000);
				}

				calcNormal(v,normal);

				if (call_level <= 100) {
				glNormal3fv(normal);
#if !ENABLE_TEXTURE_MAPPING
				glColor3ub(255, 0, 0);
#endif
				glBegin(GL_POLYGON);
				for (i=0;i<nv;i++) {
#if ENABLE_TEXTURE_MAPPING
					glTexCoord2fv(uv[i]);
#endif
					glVertex3fv(v[i]);
				}
				glEnd();
				}

				p += nv * 12;
				break;
			}

			/*
			// Sort by Normal
			//
			// 2  UINT16      unknown
			// 4  VMS_VECTOR  Front Model normal
			// 16 VMS_VECTOR  Back Model normal
			// 28 UINT16      Front Model Offset
			// 30 UINT16      Back Model Offset
			//
			*/
			case OP_SORTNORM: {
				/* = W(p+2); */
				/* = W(p+4); */
				/* = W(p+16); */
				assert(W(p+2)==0);
				assert(W(p+28)>0);
				assert(W(p+30)>0);
				interp_model_data(p + W(p+28),offset,model,call_level+1);
				interp_model_data(p + W(p+30),offset,model,call_level+1);
				p += 32;
				break;
			}

			/*
			// Call a Sub Object
			//
			// 2  UINT16     n_anims
			// 4  VMS_VECTOR offset
			// 16 UINT16     model offset
			//
			*/
			case OP_SUBCALL: {
				assert(W(p+16)>0);
				/* = VP(p+4) */
				interp_model_data(p + W(p+16),VP(p+4),model,call_level+1);
				p += 20;
				break;

			}


			/*
			// Glow Number for Next Poly
			//
            // 2 UINTW  Glow_Value
			*/
			case OP_GLOW: {
				glow_num = W(p+2);
				p += 4;
				break;
			}

			default: {
//				printf("unknown OP type (%d)\n",W(p));
				assert(0);
			}
		}
	}
	return;
}


