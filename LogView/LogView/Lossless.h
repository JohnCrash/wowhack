#pragma once

#define ALIGNS_DWORD( x ) (x%4 != 0?(x+(4-x%4)):x)

#define RGBA( r,g,b,a ) ( ((DWORD)(a))|(((DWORD)(r)) << 8)|(((DWORD)(g)) << 16)|(((DWORD)(b)) << 24) )
#define RValue( a ) (BYTE)((((DWORD)(a))&0x0000ff00)>>8)
#define GValue( a ) (BYTE)((((DWORD)(a))&0x00ff0000)>>16)
#define BValue( a ) (BYTE)((((DWORD)(a))&0xff000000)>>24)
#define AValue( a ) (BYTE)(((DWORD)(a))&0x000000ff)

#define GetAValue(rgb)      ((BYTE)((rgb)>>24))
class Lossless{
public:
	BYTE   format;  //8,16,24,32
	WORD   npal;    //0-256
	WORD   width;
	WORD   height;
	WORD   pitch;
	DWORD  rMask;
	DWORD  gMask;
	DWORD  bMask;
	BYTE   bits[1];
};

Lossless* load_png( LPCSTR filename,LPDWORD psize );
int get_right_bit( DWORD mask );
int get_left_bit_16( DWORD mask,int right );
HBITMAP create_bitmap_32( int w,int h,LPBYTE* lpBits,int* scanline );
HBITMAP create_bitmap_24( int w,int h,LPBYTE* lpBits,int* scanline );
HBITMAP lossless_to_hbitmap( Lossless* ploss );