//=====================================================================
// Copyright 2002-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team//    
//  Workfile: XltCallbacks.h
//
//  Description:
//      ILTextParserEnv class definition.
//=====================================================================

#ifndef XLTCALLBACKS_H
#define XLTCALLBACKS_H

#include <stdarg.h>

// callback accessor functions
void* xlt_malloc( unsigned int nSizeInBytes );
void  xlt_free( void* pBuffer );
int   xlt_printf( const char* pszBuffer, ... );
int   xlt_error( const char* pszBuffer, ... );
int   xlt_debug( const char* pszBuffer, ... );
void  xlt_outputBuffer( const void* pBuffer, unsigned int nBufferSize );
void  xlt_assert(void);
int   xlt_FloatToString(char *pszBuffer, float Value);
bool  xlt_isLineMode(void);

#endif // XLTCALLBACKS_H