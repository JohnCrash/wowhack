#include "stdafx.h"
#include "callhook.h"
#include <math.h>

int memcmp_mask( void* ps,const void* pcode,const void* pmask,int size ){
	unsigned char* p = (unsigned char*)ps;
	unsigned char* mask = (unsigned char*)pmask;
	unsigned char* code = (unsigned char*)pcode;

	for( int i=0;i<size;i++ ){
		if( mask[i]==0){
			if( p[i]!=code[i] )
				return 1;
		}
	}
	return 0;
}

void* ScreechFunctionEntry( void* ps,void* pe,const unsigned char code[16],const unsigned char mask[16] ){
	int i;
	BOOL bPrevBlackIsCommit;
	MEMORY_BASIC_INFORMATION mbi;
	BOOL bNoMask = FALSE;
	for( i = 0;i < 16;i++ ){
		if( mask[i]!=0 ){
			bNoMask = TRUE;
			break;
		}
	}
	bPrevBlackIsCommit = FALSE;
	for( char* p=(char*)ps;p<=pe;p+=PAGE_SIZE ){
		if( VirtualQuery( p,&mbi,sizeof(mbi) ) == sizeof(mbi) ){
			if( bPrevBlackIsCommit ){
				for( i = 0;i < 16;i++ ){
					if( !bNoMask ){
						if( memcmp( (p-i),code,16 ) == 0 ){
							return (void*)(p+i);
						}
					}else{
						if( memcmp_mask( (p-i),code,mask,16 ) == 0 ){
							return (void*)(p+i);
						}
					}
				}
			}
			if( mbi.State == MEM_COMMIT ){
				for( i = 0;i < PAGE_SIZE-16;i++ ){
					if( !bNoMask ){
						if( memcmp( (p+i),code,16 ) == 0 ){
							return (void*)(p+i);
						}
					}else{
						if( memcmp_mask( (p+i),code,mask,16 ) == 0 ){
							return (void*)(p+i);
						}
					}
				}
			}
			bPrevBlackIsCommit = TRUE;
		}else{
			bPrevBlackIsCommit = FALSE;
		}
	}

	return NULL;
}

//a=b-c
void vector_sub( float a[3],float b[3],float c[3] ){
	a[0] = b[0]-c[0];
	a[1] = b[1]-c[1];
	a[2] = b[2]-c[2];
}
//d=|a|
float vector_mod( float a[3] ){
	return sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
}
float vector_mod2( float a[3] ){
	return a[0]*a[0]+a[1]*a[1]+a[2]*a[2];
}
//点积
float vector_dot( float b[3],float c[3] ){
	return (b[0]*c[0]+b[1]*c[1]+b[2]*c[2]);
}
void vector_mul( float c[3],float a[3],float b[3]){
	c[0] = a[1]*b[2]-a[2]*b[1];
	c[1] = a[2]*b[0]-a[0]*b[2];
	c[2] = a[0]*b[1]-a[1]*b[0];
}

//矢量copy
void vector_copy( float vt[3],float vs[3] ){
	vt[0] = vs[0];
	vt[1] = vs[1];
	vt[2] = vs[2];
}

//求两个矢量的夹角
float vector_angle( float v[3],float s[3] ){
	float a;
	a = atan2f( v[1],v[0] ) - atan2f( s[1],s[0] );
	
	if( a > M_PI )
		a = a-2*M_PI;
	else if( a < -M_PI )
		a = 2*M_PI+a;
	return a;
}

//utf8 to ansi
std::string ansi( std::string s ){
	std::wstring wstr;
	std::string str;

	int len = MultiByteToWideChar(CP_UTF8,0,(char *)s.c_str(),-1,NULL,NULL);
	wstr.resize( len );
	MultiByteToWideChar(CP_UTF8,0,(char *)s.c_str(),-1,&wstr[0],wstr.size()); 
	len = WideCharToMultiByte(CP_ACP,0,wstr.c_str(),-1,NULL,NULL,NULL,NULL); 
	str.resize( len );
	WideCharToMultiByte(CP_ACP,0,wstr.c_str(),-1,&str[0],str.size(),NULL,NULL); 
	return str;
}

//ansi to utf8
std::string utf8( std::string s ){
	std::wstring wstr;
	std::string str;

	int len = MultiByteToWideChar(CP_ACP,0,(char *)s.c_str(),-1,NULL,NULL);
	wstr.resize( len );
	MultiByteToWideChar(CP_ACP,0,(char *)s.c_str(),-1,&wstr[0],wstr.size()); 
	len = WideCharToMultiByte(CP_UTF8,0,wstr.c_str(),-1,NULL,NULL,NULL,NULL); 
	str.resize( len );
	WideCharToMultiByte(CP_UTF8,0,wstr.c_str(),-1,&str[0],str.size(),NULL,NULL); 
	return str;
}

