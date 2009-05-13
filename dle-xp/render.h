// render.h

void TextureMap(int resolution,
				CDSegment *segment,
				INT16 sidenum,
				UINT8 *bmData,
				UINT16 bmWidth,
				UINT16 bmHeight,
				UINT8 *light_index,
				UINT8 *pScrnMem,
				APOINT* scrn,
				UINT16 width,
				UINT16 height,
				UINT16 rowOffset);

double dround_off(double value, double round);
/*
double CalcLength(vms_vector &center1,vms_vector &center2);
void CalcCenter(CMine &mine, vms_vector &center,INT16 segnum,INT16 sidenum);
void CalcOrthoVector(CMine *mine, vms_vector &result,INT16 segnum,INT16 sidenum);
*/