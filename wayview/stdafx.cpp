// stdafx.cpp : 只包括标准包含文件的源文件
// wayview.pch 将是预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"
#include "..\wowin\callhook.h"
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

//求矢量和x轴的角度0-2*pi
float vector_angle( float v[3] ){
	return atan2f( v[1],v[0] );
	/*
	if( v[0] > 0 ){
		if( v[1] > 0 )
			return atan(v[1]/v[0]);
		else if( v[1] < 0 )
			return atan(v[1]/v[0])+2*M_PI;
		else
			return 0;
	}else if( v[0] < 0 ){
		if( v[1] == 0 )
			return M_PI;
		else
			return (atan(v[1]/v[0])+M_PI);
	}else if( v[0] == 0 ){
		if( v[1] > 0 )
			return M_PI/2;
		else
			return 3*M_PI/2;
	}
	return 0;*/
}