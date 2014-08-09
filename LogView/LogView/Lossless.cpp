#include "stdafx.h"
#include "png.h"
#include "lossless.h"

/* Load a PNG type image from an SDL datasource */
static void png_read_data(png_structp ctx, png_bytep area, png_size_t size)
{
	FILE *fp;

	fp = (FILE *)png_get_io_ptr(ctx);
	fread( area, size, 1,fp );
}

Lossless* load_png( LPCSTR filename,LPDWORD psize ){
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	png_bytep *volatile row_pointers;
	int row, i;
	volatile int ckey = -1;
	png_color_16 *transv;
	FILE* fp;

	fp = fopen( filename,"rb" );
	if( fp == NULL )return NULL;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
	if( png_ptr == NULL )return NULL;
	info_ptr = png_create_info_struct(png_ptr);
	if( info_ptr == NULL )return NULL;

	png_set_read_fn(png_ptr, fp, png_read_data);

	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
				&color_type, &interlace_type, NULL, NULL);

	png_set_strip_16(png_ptr);

	png_set_packing(png_ptr);

	if(color_type == PNG_COLOR_TYPE_GRAY)
		png_set_expand(png_ptr);

	if ( color_type == PNG_COLOR_TYPE_GRAY_ALPHA )
		png_set_gray_to_rgb(png_ptr);

	png_read_update_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
			&color_type, &interlace_type, NULL, NULL);

	int pitch = ALIGNS_DWORD( width*(bit_depth*info_ptr->channels)/8 );
	int len = sizeof(Lossless)+pitch*height+info_ptr->num_palette*3;
	Lossless* ploss = (Lossless*)new BYTE[len];
	if ( ploss == NULL ) {
		goto done;
	}
	memset( ploss,0,len );
	ploss->format = bit_depth*info_ptr->channels;
	ploss->pitch = pitch;
	ploss->npal = info_ptr->num_palette;
	ploss->width = width;
	ploss->height = height;
	ploss->rMask = 0x000000ff;
	ploss->gMask = 0x0000ff00;
	ploss->bMask = 0x00ff0000;

	/* Create the array of pointers to image data */
	row_pointers = (png_bytep*) malloc(sizeof(png_bytep)*height);
	if ( (row_pointers == NULL) ) {
		delete ploss;
		goto done;
	}
	for (row = 0; row < (int)height; row++) {
		row_pointers[row] = (png_bytep)((BYTE *)(ploss->bits+ploss->npal*3) + row*pitch);
	}

	/* Read the entire image in one go */
	png_read_image(png_ptr, row_pointers);
	
	/* 做一个调整
		因为BlendAlpha函数的计算方式是
		Des.Color = Src.Color + (1-Src.Alpha)*Des.Color;
		而标准的混合公式为
		Des.Color = Src.Color*Src.Alpha + (1-Src.Alpha)*Des.Color;
		所以必须在这里加入一个对源颜色乘以源Alpha值的操作
	*/

	for( row = 0; row < height;row++ ){
		LPDWORD pline = (LPDWORD)row_pointers[row];
		for( int i = 0;i < width;i++ ){
			COLORREF c = *(pline+i);
			BYTE a = GetAValue(c);
			BYTE r = GetRValue(c)*a/255;
			BYTE g = GetGValue(c)*a/255;
			BYTE b = GetBValue(c)*a/255;
			*(pline+i) = (DWORD)(a<<24)|RGB(r,g,b);
		}
	}
	/* Load the palette, if any */
	BYTE* palette = ploss->bits;
	if ( palette ) {
		if (info_ptr->num_palette > 0 ) {
			for( i=0; i<info_ptr->num_palette; ++i ) {
				palette[3*i+2] = info_ptr->palette[i].blue;
				palette[3*i+1] = info_ptr->palette[i].green;
				palette[3*i] = info_ptr->palette[i].red;
			}
	    }
	}
	
done:	/* Clean up and return */
	fclose( fp );

	if ( png_ptr ) {
		png_destroy_read_struct(&png_ptr,
		                        info_ptr ? &info_ptr : (png_infopp)0,
								(png_infopp)0);
	}
	if ( row_pointers ) {
		free(row_pointers);
	}

	return ploss; 
}

HBITMAP create_bitmap_24( int w,int h,LPBYTE* lpBits,int* scanline ){
	BITMAPINFOHEADER bm;
	int iScanLine;
	HBITMAP  hMap;

	iScanLine = ALIGNS_DWORD(3*w);
	bm.biSize = sizeof(BITMAPINFOHEADER);
	bm.biWidth = w;
	bm.biHeight = h;
	bm.biPlanes = 1;
	bm.biBitCount = 24;
	bm.biCompression = 0;
	bm.biSizeImage = iScanLine*h;
	bm.biXPelsPerMeter = 0;
	bm.biYPelsPerMeter = 0;
	bm.biClrUsed = 0;
	bm.biClrImportant = 0;

	HDC hDC = GetDC(NULL);
	hMap = CreateDIBSection( hDC,(LPBITMAPINFO)&bm,DIB_RGB_COLORS,
		                     (VOID**)lpBits,NULL,0 );
	ReleaseDC( NULL,hDC );
	*scanline = iScanLine;
	return hMap;
}

HBITMAP create_bitmap_32( int w,int h,LPBYTE* lpBits,int* scanline ){
	BITMAPINFOHEADER bm;
	int iScanLine;
	HBITMAP  hMap;

	iScanLine = ALIGNS_DWORD(4*w);
	bm.biSize = sizeof(BITMAPINFOHEADER);
	bm.biWidth = w;
	bm.biHeight = h;
	bm.biPlanes = 1;
	bm.biBitCount = 32;
	bm.biCompression = 0;
	bm.biSizeImage = iScanLine*h;
	bm.biXPelsPerMeter = 0;
	bm.biYPelsPerMeter = 0;
	bm.biClrUsed = 0;
	bm.biClrImportant = 0;

	HDC hDC = GetDC(NULL);
	hMap = CreateDIBSection( hDC,(LPBITMAPINFO)&bm,DIB_RGB_COLORS,
		                     (VOID**)lpBits,NULL,0 );
	ReleaseDC( NULL,hDC );
	*scanline = iScanLine;
	return hMap;
}

HBITMAP lossless_to_hbitmap( Lossless* ploss ){
	HBITMAP hbitmap;
	LPBYTE lpbits,pbits,pS,pPal;
	int scanline,i,j;

	if( ploss == NULL )return NULL;

	if( ploss->format < 32 )
		hbitmap = create_bitmap_24( ploss->width,ploss->height,&lpbits,&scanline );
	else
		hbitmap = create_bitmap_32( ploss->width,ploss->height,&lpbits,&scanline );
	if( hbitmap == NULL )
		return NULL;

	if( ploss->format == 8 ){
		LPBYTE ps;
		int k;

		pPal = ploss->bits;
		pS = ploss->bits+ploss->npal*3;
		for( i = 0;i<ploss->height;i++ ){
			ps = pS + i*ploss->pitch;
			pbits = lpbits + (ploss->height-i-1)*scanline;
			for( j = 0;j<ploss->width;j++ ){
				k = 3*(*ps);
				*pbits     = *(pPal+k+2); //b
				*(pbits+1) = *(pPal+k+1); //g
				*(pbits+2) = *(pPal+k);   //r
				ps++;
				pbits+=3;
			}
		}
	}else if( ploss->format == 16 ){
		LPWORD ps;
		WORD c,rMask,gMask,bMask;
		int rRight,bRight,gRight,rLeft,gLeft,bLeft;
		BYTE r,g,b;

		rMask = (WORD)ploss->rMask;
		gMask = (WORD)ploss->gMask;
		bMask = (WORD)ploss->bMask;
		rRight = get_right_bit( rMask );
		gRight = get_right_bit( gMask );
		bRight = get_right_bit( bMask );
		rLeft = get_left_bit_16( rMask,rRight );
		gLeft = get_left_bit_16( gMask,gRight );
		bLeft = get_left_bit_16( bMask,bRight );
		pPal = ploss->bits;
		pS = ploss->bits+ploss->npal*3;
		for( i = 0;i<ploss->height;i++ ){
			ps = (LPWORD)(pS + i*ploss->pitch);
			pbits = lpbits + (ploss->height-i-1)*scanline;
			for( j = 0;j<ploss->width;j++ ){
				c = *ps;
				r = (c&rMask)>>rRight;
				b = (c&bMask)>>bRight;
				g = (c&gMask)>>gRight;
				r <<= rLeft;
				g <<= gLeft;
				b <<= bLeft;
				*pbits = b;
				*(pbits+1) = g;
				*(pbits+2) = r;
				ps++;
				pbits+=3;
			}
		}
	}else if( ploss->format == 24 ){
		LPBYTE ps;
		DWORD c,rMask,gMask,bMask;
		int rRight,bRight,gRight;
		BYTE r,g,b;

		rMask = ploss->rMask;
		gMask = ploss->gMask;
		bMask = ploss->bMask;

		rRight = get_right_bit( rMask );
		gRight = get_right_bit( gMask );
		bRight = get_right_bit( bMask );
		pPal = ploss->bits;
		pS = ploss->bits+ploss->npal*3;
		for( i = 0;i<ploss->height;i++ ){
			ps = (LPBYTE)(pS + i*ploss->pitch);
			pbits = lpbits + (ploss->height-i-1)*scanline;
			for( j = 0;j<ploss->width;j++ ){
				c = *((LPDWORD)ps);
				r = (BYTE)((c&rMask)>>rRight);
				b = (BYTE)((c&bMask)>>bRight);
				g = (BYTE)((c&gMask)>>gRight);
				*pbits = b;
				*(pbits+1) = g;
				*(pbits+2) = r;
				ps+=3;
				pbits+=3;
			}
		}
	}else if( ploss->format == 32 ){
		LPDWORD ps;
		DWORD c,rMask,gMask,bMask,aMask;
		int rRight,bRight,gRight,aRight;
		BYTE r,g,b,a;

		rMask = ploss->rMask;
		gMask = ploss->gMask;
		bMask = ploss->bMask;
		aMask = 0xff000000;//FIXME ???

		rRight = get_right_bit( rMask );
		gRight = get_right_bit( gMask );
		bRight = get_right_bit( bMask );
		aRight = get_right_bit( aMask );
		pPal = ploss->bits;
		pS = ploss->bits+ploss->npal*3;
		for( i = 0;i<ploss->height;i++ ){
			ps = (LPDWORD)(pS + i*ploss->pitch);
			pbits = lpbits + (ploss->height-i-1)*scanline;
			for( j = 0;j<ploss->width;j++ ){
				c = *ps;
				r = (BYTE)((c&rMask)>>rRight);
				b = (BYTE)((c&bMask)>>bRight);
				g = (BYTE)((c&gMask)>>gRight);
				a = (BYTE)((c&aMask)>>aRight);
				*pbits = b;
				*(pbits+1) = g;
				*(pbits+2) = r;
				*(pbits+3) = a;
				ps++;
				pbits+=4;
			}
		}
	}
	return hbitmap;
}

int get_right_bit( DWORD mask ){
	int i;
	for( i = 0;i < 32;i++ ){
		if( ((mask>>i) & 1 ) == 1 ){
			return i;
		}
	}
	return i;
}

int get_left_bit_16( DWORD mask,int right ){
	if( mask>>right == 0x1f )
		return 3;
	return 2;
}