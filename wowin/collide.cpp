#include "stdafx.h"
#include "callhook.h"
#include "collide.h"
#include "wowentry.h"
#include <float.h>
#include "render.h"

Triangle g_Downs[4096];
Triangle g_Ups[4096];
Collide  g_collide( sizeof(g_Downs)/sizeof(Triangle),g_Downs,
					sizeof(g_Ups)/sizeof(Triangle),g_Ups );

extern Render g_Render;

Collide::Collide( int down,Triangle* pdowns,int up,Triangle* pups ):
iDownMax(down),iDownCount(0),pDowns(pdowns),iDownMax2(down),
iUpMax(up),iUpCount(0),pUps(pups),iUpMax2(up){
}

//从点p,到地面的距离,如果就在地面上返回0
float FallLength( float p[3] ){
	float fl = 0;
	float v[6];
	unsigned long flag = 0X100111;
	vector_copy(v,p);
	vector_copy(&(v[3]),p);
	v[5] += 2;
	if( HitWrap(v,&g_collide,flag,0) != 0 ){
		if( g_collide.iDownCount > 0 ){
			float z = FLT_MAX;
			if( g_Downs[0].n[2] != 0 )
				z = -(g_Downs[0].n[0]*p[0] + g_Downs[0].n[1]*p[1] + g_Downs[0].n[3])/g_Downs[0].n[2];
			fl = p[2] - z;
		}
	}
	return fl;
}

Collide* CalcCollide( float p[6] ){
	if( HitWrap(p,&g_collide,0x100111,0) ){
		return &g_collide;
	}
	return NULL;
}

//计算平面plane和直线lines的的交点，t
//平面方程plane,lines是线段
float CalcPlanePoint( float plane[4],float lines[6] ){
	float d = plane[0]*(lines[3]-lines[0]) + plane[1]*(lines[4]-lines[1]) + plane[2]*(lines[5]-lines[2]);
	if( d == 0 )//平行
		return FLT_MAX;
	return -(plane[0]*lines[0] + plane[1]*lines[1] + plane[2]*lines[2] + plane[3])/d;
}

//叉积k的符号
bool VectorX2D( float v0[2],float v1[2] ){
	return (v0[0]*v1[1]-v0[1]*v1[0])>0?true:false;
}
//判断p是不是在2D的3三角内
bool Inside2D( float tr[6],float p[2] ){
	float v0[2],v1[2];
	v0[0] = tr[2]-tr[0];
	v0[1] = tr[3]-tr[1];
	v1[0] = p[0]-tr[0];
	v1[1] = p[1]-tr[1];
	bool b = VectorX2D( v0,v1 );
	v0[0] = tr[4]-tr[2];
	v0[1] = tr[5]-tr[3];
	v1[0] = p[0]-tr[2];
	v1[1] = p[1]-tr[3];
	if( VectorX2D( v0,v1 ) != b )return false;
	v0[0] = tr[0]-tr[4];
	v0[1] = tr[1]-tr[5];
	v1[0] = p[0]-tr[4];
	v1[1] = p[1]-tr[5];
	if( VectorX2D( v0,v1 ) != b )return false;
	return true;
}

//计算一个三角和直线的是否相交
bool IsInTriangle( Triangle* pt,float lines[6] ){
	float t = CalcPlanePoint( pt->n,lines );
	if( t > 1 || t < 0 )return false;
	float p[3];
	p[0] = (lines[3]-lines[0])*t+lines[0];
	p[1] = (lines[4]-lines[1])*t+lines[1];
	p[2] = (lines[5]-lines[2])*t+lines[2];
	//这里基于一个点在三角形内，哪么也在它的影子内。
	//取在x,y平面的投影来判断,前提是平面不和z轴平行
	//pt->n[2] != 0表示不和z平行
	float tr[6];
	tr[0] = pt->a[0];
	tr[1] = pt->a[1];
	tr[2] = pt->b[0];
	tr[3] = pt->b[1];
	tr[4] = pt->c[0];
	tr[5] = pt->c[1];

	float pp[2];

	if( abs(pt->n[2]) > 0.001 ){
		return Inside2D( tr,p);
	}else if( abs(pt->n[1]) > 0.001 ){
		//如果和z平行或者接近平行就投影到x,z平面
		pp[0] = p[0];
		pp[1] = p[2];
		tr[0] = pt->a[0];
		tr[1] = pt->a[2];
		tr[2] = pt->b[0];
		tr[3] = pt->b[2];
		tr[4] = pt->c[0];
		tr[5] = pt->c[2];
		return Inside2D( tr,pp);
	}else{
		pp[0] = p[1];
		pp[1] = p[2];
		tr[0] = pt->a[1];
		tr[1] = pt->a[2];
		tr[2] = pt->b[1];
		tr[3] = pt->b[2];
		tr[4] = pt->c[1];
		tr[5] = pt->c[2];
		return Inside2D( tr,pp);
	}

	return false;
}

bool Collide( float p[6] ){
	/*p[6]是一个盒子，必须是p[0]<p[3] p[1]<p[4] p[2]<p[5]
		等于是取得空间中一个盒子区域里面的三角平面
	*/
	float box[6];
	box[0] = min(p[0],p[3]);
	box[3] = max(p[0],p[3]);
	box[1] = min(p[1],p[4]);
	box[4] = max(p[1],p[4]);
	box[2] = min(p[2],p[5]);
	box[5] = max(p[2],p[5]);
	if( HitWrap(box,&g_collide,0x100111,0) != 0 ){
		/*
		{
			g_Render.Begin("test_ray");
			g_Render.Color(1,1,0);
			g_Render.MoveTo( p[0],p[1],p[2] );
			g_Render.LineTo( p[3],p[4],p[5] );
			g_Render.End();

			g_Render.Begin("test_triangle");
			g_Render.Color(1,0,0);
			g_Render.LineWidth(3);
			for( int i = 0;i < g_collide.iDownCount;i++ ){
				g_Render.MoveTo( g_collide.pDowns[i].a[0],g_collide.pDowns[i].a[1],g_collide.pDowns[i].a[2] );
				g_Render.LineTo( g_collide.pDowns[i].b[0],g_collide.pDowns[i].b[1],g_collide.pDowns[i].b[2] );
				g_Render.LineTo( g_collide.pDowns[i].c[0],g_collide.pDowns[i].c[1],g_collide.pDowns[i].c[2] );
			}
			g_Render.End();
		}*/
	//	g_Render.Begin("test_triangle");
	//	g_Render.Color(1,0,0);
	//	g_Render.LineWidth(3);
		for( int i = 0;i < g_collide.iDownCount;i++ ){
			if( IsInTriangle( &g_collide.pDowns[i],p ) ){
	//			g_Render.MoveTo( g_collide.pDowns[i].a[0],g_collide.pDowns[i].a[1],g_collide.pDowns[i].a[2] );
	//			g_Render.LineTo( g_collide.pDowns[i].b[0],g_collide.pDowns[i].b[1],g_collide.pDowns[i].b[2] );
	//			g_Render.LineTo( g_collide.pDowns[i].c[0],g_collide.pDowns[i].c[1],g_collide.pDowns[i].c[2] );
	//			g_Render.LineTo( g_collide.pDowns[i].a[0],g_collide.pDowns[i].a[1],g_collide.pDowns[i].a[2] );
	//			float t = CalcPlanePoint(g_collide.pDowns[i].n,p);
	//			float pp[3];
	//			pp[0] = (p[3]-p[0])*t+p[0];
	//			pp[1] = (p[4]-p[1])*t+p[1];
	//			pp[2] = (p[5]-p[2])*t+p[2];
	//			g_Render.Size(0.2);
	//			g_Render.Box(pp[0],pp[1],pp[2]);
	//			printf("%f %f %f %f\n",g_collide.pDowns[i].n[0],g_collide.pDowns[i].n[1],g_collide.pDowns[i].n[2],g_collide.pDowns[i].n[3] );
	//			g_Render.End();
				return true;
			}
		}
	//		g_Render.End();
	}
	return false;
}

int lua_IsCollide( void* p ){
	float v[6];
	bool isok = true;

	if( Lua_isnumber(p,1) ){
		v[0] = (float)Lua_tonumber(p,1);
	}else isok = false;
	if( Lua_isnumber(p,2) ){
		v[1] = (float)Lua_tonumber(p,2);
	}else isok = false;
	if( Lua_isnumber(p,3) ){
		v[2] = (float)Lua_tonumber(p,3);
	}else isok = false;
	if( Lua_isnumber(p,4) ){
		v[3] = (float)Lua_tonumber(p,4);
	}else isok = false;
	if( Lua_isnumber(p,5) ){
		v[4] = (float)Lua_tonumber(p,5);
	}else isok = false;
	if( Lua_isnumber(p,6) ){
		v[5] = (float)Lua_tonumber(p,6);
	}else isok = false;

	if( isok ){
		if( Collide( v ) ){
			Lua_pushnumber(p,1);
		}else{
			Lua_pushnumber(p,0);
		}
		return 1;
	}
	Lua_pushnil( p );
	return 1;
}