#ifndef ROXLU_OPENGLINCLUDESH
#define ROXLU_OPENGLINCLUDESH

#define ROXLU_OPENGLES			1
#define ROXLU_OPENFRAMEWORKS	2
#define ROXLU_COCOA				3

#ifdef _WIN32
	#define ROXLU_VARIANT	ROXLU_OPENFRAMEWORKS
#else
//	#define ROXLU_VARIANT 	ROXLU_COCOA
#endif

#define ROXLU_VARIANT 	ROXLU_OPENFRAMEWORKS




#if ROXLU_VARIANT == ROXLU_OPENGLES
	#include <OpenGLES/ES2/gl.h>
#elif ROXLU_VARIANT == ROXLU_OPENFRAMEWORKS
	#include "ofMain.h"
#elif ROXLU_VARIANT == ROXLU_COCOA
	#include <OpenGL/gl.h>
#endif

#endif