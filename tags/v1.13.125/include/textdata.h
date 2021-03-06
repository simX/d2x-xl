#ifndef _TEXTDATA_H
#define _TEXTDATA_H

void LoadTextData (const char *pszLevelName, const char *pszExt, tTextData *msgP);
void FreeTextData (tTextData *msgP);
int ShowGameMessage (tTextData *msgP, int nId, int nDuration);
tTextIndex *FindTextData (tTextData *msgP, int nId);

#endif //_TEXTDATA_H
