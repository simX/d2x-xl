#ifndef __INPUT_H
#define __INPUT_H

#include "kconfig.h"

extern tControlInfo Controls [4];

int ControlsReadJoystick (int *joy_axis);
void ControlsReadFCS (int raw_axis);
int ControlsReadAll (void);
void FlushInput (void);
void ResetCruise (void);
char GetKeyValue (char key);
void SetControlType (void);
int CalcDeadzone (int d, int nDeadzone);

#endif //__INPUT_H
