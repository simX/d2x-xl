/* $Id: ogl.c,v 1.14 2004/05/11 23:15:55 btb Exp $ */
/*
 *
 * Graphics support functions for OpenGL.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef _WIN32
#	include <windows.h>
#	include <stddef.h>
#	include <io.h>
#endif
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#ifdef __macosx__
# include <stdlib.h>
# include <SDL/SDL.h>
#else
# include <malloc.h>
# include <SDL.h>
#endif

#include "inferno.h"
#include "error.h"
#include "u_mem.h"
#include "lighting.h"
#include "lightmap.h"
#include "texmerge.h"
#include "ogl_defs.h"
#include "ogl_shader.h"

//------------------------------------------------------------------------------

char *LoadShader (char* fileName) //, char* Shadersource)
{
	FILE	*fp; 
	char	*bufP = NULL; 
	int 	fSize;
#ifdef _WIN32
	int	f;
#endif 
	char 	fn [FILENAME_LEN];

if (!(fileName && *fileName))
	return NULL;	// no fileName

sprintf (fn, "%s%s%s", gameFolders.szShaderDir, *gameFolders.szShaderDir ? "/" : "", fileName);
#ifdef _WIN32
if (0 > (f = _open (fn, _O_RDONLY)))
	return NULL;	// couldn't open file
fSize = _lseek (f, 0, SEEK_END);
_close (f); 
if (fSize <= 0)
	return NULL;	// empty file or seek error
#endif

if (!(fp = fopen (fn, "rt")))
	return NULL;	// couldn't open file

#ifndef _WIN32
fseek (fp, 0, SEEK_END);
fSize = ftell (fp);
if (fSize <= 0) {
	fclose (fp);
	return NULL;	// empty file or seek error
	}
#endif

if (!(bufP = (char *) D2_ALLOC (sizeof (char) *(fSize + 1)))) {
	fclose (fp);
	return NULL;	// out of memory
	}
fSize = (int) fread (bufP, sizeof (char), fSize, fp); 
bufP [fSize] = '\0'; 
fclose (fp); 
return bufP; 
}

//------------------------------------------------------------------------------

#ifdef GL_VERSION_20

void PrintShaderInfoLog (GLuint handle, int bProgram)
{
   int nLogLen = 0;
   int charsWritten = 0;
   char *infoLog;

if (bProgram) {
	glGetProgramiv (handle, GL_INFO_LOG_LENGTH, &nLogLen);
	if ((nLogLen > 0) && (infoLog = (char *) D2_ALLOC (nLogLen))) {
		infoLog = (char *) D2_ALLOC (nLogLen);
		glGetProgramInfoLog (handle, nLogLen, &charsWritten, infoLog);
		if (*infoLog)
			LogErr ("\n%s\n\n", infoLog);
		D2_FREE (infoLog);
		}
	}
else {
	glGetShaderiv (handle, GL_INFO_LOG_LENGTH, &nLogLen);
	if ((nLogLen > 0) && (infoLog = (char *) D2_ALLOC (nLogLen))) {
		glGetShaderInfoLog (handle, nLogLen, &charsWritten, infoLog);
		if (*infoLog)
			LogErr ("\n%s\n\n", infoLog);
		D2_FREE (infoLog);
		}
	}
}

#else

void PrintShaderInfoLog (GLhandleARB handle, int bProgram)
{
   int nLogLen = 0;
   int charsWritten = 0;
   char *infoLog;

glGetObjectParameteriv (handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &nLogLen);
if ((nLogLen > 0) && (infoLog = (char *) D2_ALLOC (nLogLen))) {
	glGetInfoLog (handle, nLogLen, &charsWritten, infoLog);
	if (*infoLog)
		LogErr ("\n%s\n\n", infoLog);
	D2_FREE (infoLog);
	}
}

#endif

//------------------------------------------------------------------------------

char *progVS [] = {
	"void TexMergeVS ();" \
	"void main (void) {TexMergeVS ();}"
,
	"void LightingVS ();" \
	"void main (void) {LightingVS ();}"
,
	"void LightingVS ();" \
	"void TexMergeVS ();" \
	"void main (void) {TexMergeVS (); LightingVS ();}"
	};

char *progFS [] = {
	"void TexMergeFS ();" \
	"void main (void) {TexMergeFS ();}"
,
	"void LightingFS ();" \
	"void main (void) {LightingFS ();}"
,
	"void LightingFS ();" \
	"void TexMergeFS ();" \
	"void main (void) {TexMergeFS (); LightingFS ();}" 
	};

GLhandleARB mainVS = 0;
GLhandleARB mainFS = 0;

//------------------------------------------------------------------------------

GLhandleARB	genShaderProg = 0;

int CreateShaderProg (GLhandleARB *progP)
{
if (!progP)
	progP = &genShaderProg;
if (*progP)
	return 1;
*progP = glCreateProgramObject (); 
if (*progP)
	return 1;
LogErr ("   Couldn't create shader program tObject\n");
return 0; 
}

//------------------------------------------------------------------------------

void DeleteShaderProg (GLhandleARB *progP)
{
if (!progP)
	progP = &genShaderProg;
if (progP && *progP) {
	glDeleteObject (*progP);
	*progP = 0;
	}
}

//------------------------------------------------------------------------------

int CreateShaderFunc (GLhandleARB *progP, GLhandleARB *fsP, GLhandleARB *vsP, char *fsName, char *vsName, int bFromFile)
{
	GLhandleARB	fs, vs;
	GLint bFragCompiled, bVertCompiled; 

if (!gameStates.ogl.bShadersOk)
	return 0;
if (!CreateShaderProg (progP))
	return 0;
if (*fsP) {
	glDeleteObject (*fsP); 
	*fsP = 0;
	}
if (*vsP) {
	glDeleteObject (*vsP);
	*vsP = 0;
	}
if (!(vs = glCreateShaderObject (GL_VERTEX_SHADER)))
	return 0;
if (!(fs = glCreateShaderObject (GL_FRAGMENT_SHADER))) {
	glDeleteObject (vs);
	return 0;
	}
#if DBG_SHADERS	
if (bFromFile) {
	vsName = LoadShader (vsName); 
	fsName = LoadShader (fsName); 
	if (!vsName || !fsName) 
		return 0; 
	}
#endif
glShaderSource (vs, 1, (const GLcharARB **) &vsName, NULL); 
glShaderSource (fs, 1, (const GLcharARB **) &fsName, NULL); 
#if DBG_SHADERS	
if (bFromFile) {
	D2_FREE (vsName); 
	D2_FREE (fsName); 
	}
#endif
glCompileShader (vs); 
glCompileShader (fs); 
glGetObjectParameteriv (vs, GL_OBJECT_COMPILE_STATUS_ARB, &bVertCompiled); 
glGetObjectParameteriv (fs, GL_OBJECT_COMPILE_STATUS_ARB, &bFragCompiled); 
if (!bVertCompiled || !bFragCompiled) {
	if (!bVertCompiled) {
		LogErr ("   Couldn't compile vertex shader\n   \"%s\"\n", vsName);
		PrintShaderInfoLog (vs, 0);
		}
	if (!bFragCompiled) {
		LogErr ("   Couldn't compile fragment shader\n   \"%s\"\n", fsName);
		PrintShaderInfoLog (fs, 0);
		}
	return 0; 
	}
glAttachObject (*progP, vs); 
glAttachObject (*progP, fs); 
*fsP = fs;
*vsP = vs;
return 1;
}

//------------------------------------------------------------------------------

int LinkShaderProg (GLhandleARB *progP)
{
	int	i = 0;
	GLint	bLinked;

if (!progP) {
	progP = &genShaderProg;
	if (!*progP)
		return 0;
	if (gameOpts->ogl.bGlTexMerge)
		i |= 1;
	if (gameOpts->render.bDynLighting)
		i |= 2;
	if (!i)
		return 0;
	if (!CreateShaderFunc (progP, &mainFS, &mainVS, progFS [i - 1], progVS [i - 1], 0)) {
		DeleteShaderProg (progP);
		return 0;
		}
	}
glLinkProgram (*progP); 
glGetObjectParameteriv (*progP, GL_OBJECT_LINK_STATUS_ARB, &bLinked); 
if (bLinked)
	return 1;
LogErr ("   Couldn't link shader programs\n");
PrintShaderInfoLog (*progP, 1);
DeleteShaderProg (progP);
return 0; 
}

//------------------------------------------------------------------------------

void InitShaders (void)
{
	GLint	nTMUs;
	
glGetIntegerv (GL_MAX_TEXTURE_UNITS, &nTMUs);
if (gameStates.ogl.bShadersOk)
	gameStates.ogl.bShadersOk = (nTMUs > 1);
if (gameStates.render.color.bLightMapsOk)
	gameStates.render.color.bLightMapsOk = (nTMUs > 2);
DeleteShaderProg (NULL);
#if LIGHTMAPS
InitLightmapShaders ();
#endif
InitTexMergeShaders ();
InitLightingShaders ();
InitVertLightShader ();
LinkShaderProg (NULL);
}

//------------------------------------------------------------------------------

void OglInitShaders (void)
{
if (!gameOpts->render.bUseShaders ||
	 !gameStates.ogl.bMultiTexturingOk || 
	 !pszOglExtensions ||
	 !strstr (pszOglExtensions, "GL_ARB_shading_language_100") || 
	 !strstr (pszOglExtensions, "GL_ARB_shader_objects")) 
	gameStates.ogl.bShadersOk = 0;
else {
#ifndef GL_VERSION_20
	glCreateProgramObject		= (PFNGLCREATEPROGRAMOBJECTARBPROC) wglGetProcAddress ("glCreateProgramObjectARB");
	glDeleteObject					= (PFNGLDELETEOBJECTARBPROC) wglGetProcAddress ("glDeleteObjectARB");
	glUseProgramObject			= (PFNGLUSEPROGRAMOBJECTARBPROC) wglGetProcAddress ("glUseProgramObjectARB");
	glCreateShaderObject			= (PFNGLCREATESHADEROBJECTARBPROC) wglGetProcAddress ("glCreateShaderObjectARB");
	glShaderSource					= (PFNGLSHADERSOURCEARBPROC) wglGetProcAddress ("glShaderSourceARB");
	glCompileShader				= (PFNGLCOMPILESHADERARBPROC) wglGetProcAddress ("glCompileShaderARB");
	glGetObjectParameteriv		= (PFNGLGETOBJECTPARAMETERIVARBPROC) wglGetProcAddress ("glGetObjectParameterivARB");
	glAttachObject					= (PFNGLATTACHOBJECTARBPROC) wglGetProcAddress ("glAttachObjectARB");
	glGetInfoLog					= (PFNGLGETINFOLOGARBPROC) wglGetProcAddress ("glGetInfoLogARB");
	glLinkProgram					= (PFNGLLINKPROGRAMARBPROC) wglGetProcAddress ("glLinkProgramARB");
	glGetUniformLocation			= (PFNGLGETUNIFORMLOCATIONARBPROC) wglGetProcAddress ("glGetUniformLocationARB");
	glUniform4f						= (PFNGLUNIFORM4FARBPROC) wglGetProcAddress ("glUniform4fARB");
	glUniform3f						= (PFNGLUNIFORM3FARBPROC) wglGetProcAddress ("glUniform3fARB");
	glUniform1f						= (PFNGLUNIFORM1FARBPROC) wglGetProcAddress ("glUniform1fARB");
	glUniform4fv					= (PFNGLUNIFORM4FVARBPROC) wglGetProcAddress ("glUniform4fvARB");
	glUniform3fv					= (PFNGLUNIFORM3FVARBPROC) wglGetProcAddress ("glUniform3fvARB");
	glUniform1fv					= (PFNGLUNIFORM1FVARBPROC) wglGetProcAddress ("glUniform1fvARB");
	glUniform1i						= (PFNGLUNIFORM1IARBPROC) wglGetProcAddress ("glUniform1iARB");
	gameStates.ogl.bShadersOk =
		glCreateProgramObject && glDeleteObject && glUseProgramObject &&
		glCreateShaderObject && glCreateShaderObject && glCompileShader && 
		glGetObjectParameteriv && glAttachObject && glGetInfoLog && 
		glLinkProgram && glGetUniformLocation && 
		glUniform4f && glUniform3f &&glUniform1f && glUniform4fv && glUniform3fv &&glUniform1fv && glUniform1i;
#else
	gameStates.ogl.bShadersOk = 1;
#endif
	}
LogErr (gameStates.ogl.bShadersOk ? "Shaders are available\n" : "No shaders available\n");
}

//------------------------------------------------------------------------------
//eof
