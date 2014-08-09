#pragma once


#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <dbghelp.h>
#include <gl\gl.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <math.h>
#include <io.h>
#include "log.h"

#define M_PI 3.1415926535897932384626433832795

#define DLL_EXPORT extern "C" __declspec(dllexport)

#define DLLEXPORT __declspec(dllexport)

int DLLEXPORT memcmp_mask( void* ps,const void* pcode,const void* pmask,int size );
void* ScreechFunctionEntry( void* ps,void* pe,const unsigned char code[16],const unsigned char mask[16] );
void DLLEXPORT vector_sub( float a[3],float b[3],float c[3] );
float DLLEXPORT vector_mod( float a[3] );
float DLLEXPORT vector_mod2( float a[3] );
float DLLEXPORT vector_dot( float b[3],float c[3] );
float DLLEXPORT vector_angle( float v[3],float s[3] );
void DLLEXPORT vector_copy( float vt[3],float vs[3] );
void DLLEXPORT vector_mul( float c[3],float a[3],float b[3]);
std::string DLLEXPORT ansi( std::string s );
std::string DLLEXPORT utf8( std::string s );

#pragma warning( disable: 4251 )
#pragma warning( disable: 4267 )
#pragma warning( disable: 4311 )
#pragma warning( disable: 4312 )
#pragma warning( disable: 4313 )

#pragma warning( disable: 4018 )
#pragma warning( disable: 4244 )
