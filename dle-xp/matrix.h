// matrix.h
#ifndef __matrix_h
#define __matrix_h

class CMatrix
{
public:

	double M[4][4];  /* 4x4 matrix used in coordinate transformation */
	double IM[4][4]; /* inverse matrix of M[4][4] */
	double Msave[4][4]; 
	double IMsave[4][4];

private:
	double _depthPerception;
	double _scale, _scaleSave;
	double _angles [3], _angleSave [3];
	INT16 _viewWidth;
	INT16 _viewHeight;

	void ClampAngle (int i);
	void RotateAngle (int i, double a);

public:
	CMatrix ();
	void Set(
		double movex, double movey, double movez, 
		double sizex, double sizey, double sizez,
		double spinx, double spiny, double spinz);
	void SetViewInfo(double depthPerception, INT16 viewWidth, INT16 viewHeight);

	void Rotate(char axis, double angle);
	void Scale(double scale);
	void Multiply(double A[4][4], double B[4][4]);
	void Calculate(double movex, double movey, double movez);
	void CalculateInverse(double movex, double movey, double movez);
	void SetPoint(vms_vector *vert, APOINT *apoint);
	void UnsetPoint(vms_vector *vert, APOINT *apoint);
	int CheckNormal(CDObject *obj, vms_vector *a, vms_vector *b);
	inline double Aspect (void) { return (double) _viewHeight / (double) _viewWidth; }
	inline double Scale (void) { return _scale; }
	void Push (void);
	void Pop (void);
	void Unrotate (void);
};

#endif //__matrix_h