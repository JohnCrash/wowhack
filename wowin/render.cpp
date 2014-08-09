#include "stdafx.h"
#include "glext.h"
#include "callhook.h"
#include "wowentry.h"
#include "dllentry.h"
#include "render.h"
#include <math.h>
#include <float.h>

const char* sWow = "wow.exe";
const char* sOpengl32 = "Opengl32";
const char* sOpenglDll = "Opengl32.dll";

extern void* g_pGfx;
extern int g_iGfxType;
bool s_bRenderHookIsSet = false;
void** g_pGXInterface;
Render g_Render;

t_SwapBuff	g_pogSwapBuff;
t_DrawScene	g_pogDrawScene;
t_SetMatrix g_pogSetProjectionMatrix;
t_SetMatrix g_pogSetModelViewMatrix;
t_ClearBuff g_pogClearBuff;

typedef FARPROC (WINAPI *t_wglGetProcAddress)(LPCSTR);
typedef void (APIENTRY *t_glMatrixMode)(GLenum mode);
typedef void (APIENTRY *t_glLoadMatrixf)(const GLfloat *m);
typedef void (APIENTRY *t_glTexEnvi)(GLenum target,GLenum pname,GLint param);
typedef void (APIENTRY *t_glTexEnvf)(GLenum target,GLenum pname,GLfloat param);
typedef void (APIENTRY *t_glTexEnvfv)(GLenum target,GLenum pname,const GLfloat* params);
typedef void (APIENTRY *t_glLightf)(GLenum light,GLenum pname,GLfloat param);
typedef void (APIENTRY *t_glLightfv)(GLenum light,GLenum pname,const GLfloat* params);
typedef void (APIENTRY *t_glBlendFunc)(GLenum sfactor,GLenum dfactor);
typedef void (APIENTRY *t_glDepthFunc)(GLenum func);
typedef void (APIENTRY *t_glDepthRange)(GLclampd zNear,GLclampd zFar);
typedef void (APIENTRY *t_glAlphaFunc)(GLenum func,GLclampf ref);
typedef BOOL (APIENTRY *t_wglMakeCurrent)(HDC hdc,HGLRC hrc);
typedef void (APIENTRY *t_glEnable)(GLenum cap);

float* g_eye_position = NULL;
float* g_eye_lookat = NULL;
float* g_matrix_eye = NULL;
float* g_matrix_projection = NULL;
static float matrix_eye[16];
static float matrix_projection[16];
static float matrix2[16]={
1,0,0,0,
0,1,0,0,
0,0,-1,0,
0,0,0,1};

PROC pfnGetProcAddress;
PROC pfnglMatrixMode;
PROC pfnglLoadMatrixf;
PROC pfnglTexEnvi;
PROC pfnglTexEnvf;
PROC pfnglTexEnvfv;
PROC pfnglLightf;
PROC pfnglLightfv;
PROC pfnglBlendFunc;
PROC pfnglDepthFunc;
PROC pfnglDepthRange;
PROC pfnglAlphaFunc;
PROC pfnwglMakeCurrent;
PROC pfnglProgramStringARB;
PROC pfnglBindProgramARB;
PROC pfnglEnable;
PROC pfnglDisable;

static void clac_matrix_eye(){
	matrix_eye[0] = g_matrix_eye[0];
	matrix_eye[1] = g_matrix_eye[1];
	matrix_eye[2] = -g_matrix_eye[2];
	matrix_eye[3] = g_matrix_eye[3];

	matrix_eye[4] = g_matrix_eye[4];
	matrix_eye[5] = g_matrix_eye[5];
	matrix_eye[6] = -g_matrix_eye[6];
	matrix_eye[7] = g_matrix_eye[7];

	matrix_eye[8] = g_matrix_eye[8];
	matrix_eye[9] = g_matrix_eye[9];
	matrix_eye[10] = -g_matrix_eye[10];
	matrix_eye[11] = g_matrix_eye[11];

	matrix_eye[8] = g_matrix_eye[8];
	matrix_eye[9] = g_matrix_eye[9];
	matrix_eye[10] = -g_matrix_eye[10];
	matrix_eye[11] = g_matrix_eye[11];

	matrix_eye[12] = g_matrix_eye[12];
	matrix_eye[13] = g_matrix_eye[13];
	matrix_eye[14] = -g_matrix_eye[14];
	matrix_eye[15] = g_matrix_eye[15];
}

static void clac_matrix_projection(){
	matrix_projection[0] = g_matrix_projection[0];
	matrix_projection[1] = g_matrix_projection[1];
	matrix_projection[2] = g_matrix_projection[2];
	matrix_projection[3] = g_matrix_projection[3];

	matrix_projection[4] = g_matrix_projection[4];
	matrix_projection[5] = g_matrix_projection[5];
	matrix_projection[6] = g_matrix_projection[6];
	matrix_projection[7] = g_matrix_projection[7];

	matrix_projection[8] = g_matrix_projection[8];
	matrix_projection[9] = g_matrix_projection[9];
	matrix_projection[10] = g_matrix_projection[10];
	matrix_projection[11] = g_matrix_projection[11];

	matrix_projection[8] = -g_matrix_projection[8];
	matrix_projection[9] = -g_matrix_projection[9];
	matrix_projection[10] = -g_matrix_projection[10];
	matrix_projection[11] = -g_matrix_projection[11];

	matrix_projection[12] = g_matrix_projection[12];
	matrix_projection[13] = g_matrix_projection[13];
	matrix_projection[14] = g_matrix_projection[14];
	matrix_projection[15] = g_matrix_projection[15];
}

void MatrixVev( float m[16],float v[4] ){
	float r[4];
	r[0] = m[0]*v[0]+m[4]*v[1]+m[8]*v[2]+m[12]*v[3];
	r[1] = m[1]*v[0]+m[5]*v[1]+m[9]*v[2]+m[13]*v[3];
	r[2] = m[2]*v[0]+m[6]*v[1]+m[10]*v[2]+m[14]*v[3];
	r[3] = m[3]*v[0]+m[7]*v[1]+m[11]*v[2]+m[15]*v[3];
	v[0] = r[0];
	v[1] = r[1];
	v[2] = r[2];
	v[3] = r[3];
}

void MatrixPoint( float v[4] ){
	clac_matrix_projection();
	clac_matrix_eye();
	MatrixVev( matrix_eye,v );
	MatrixVev( matrix_projection,v );
}

extern HWND GetWowWnd();

int lua_ScreenPt(void* p){
	float v[4];
	bool isok = true;
	if( Lua_isnumber(p,1) ){
		v[0] =  (float)Lua_tonumber(p,1);
	}else isok = false;
	if( Lua_isnumber(p,2) ){
		v[1] =  (float)Lua_tonumber(p,2);
	}else isok = false;
	if( Lua_isnumber(p,3) ){
		v[2] =  (float)Lua_tonumber(p,3);
	}else isok = false;
	if( isok ){
		v[3] = 1;
		MatrixPoint( v );
		if( v[2]!=0 ){
			v[0] /= v[2];
			v[1] /= v[2];
			HWND hWOW = GetWowWnd();
			if( hWOW ){
				RECT rect;
				POINT tl,br;
				GetWindowRect( hWOW,&rect );
				tl.x = rect.left;
				tl.y = rect.top;
				ScreenToClient( hWOW,&tl );
				br.x = rect.right;
				br.y = rect.bottom;
				ScreenToClient( hWOW,&br );
				int width = br.x - tl.x;
				int height = br.y - tl.y;
				float xx = v[0]*width/2.0+width/2.0;
				float yy = -v[1]*height/2.0+height/2.0;
				if( xx >=0 && xx <= width &&
					yy >=0 && yy <= height ){
					Lua_pushnumber(p,xx);
					Lua_pushnumber(p,yy);
					return 2;
				}
			}
		}
	}
	Lua_pushnil( p );
	return 1;
}

static void PrintMatrix( const float* m ){
	for(int i = 0;i<16;i++){
		printf("%f",m[i]);
		if( (i+1)%4 == 0 )
			printf("\n");
		else
			printf(",");
	}
}

static bool IsEqual(const float m[16],const float m2[16] ){
	for( int i = 0;i<16;i++ ){
		if( m[i] != m2[i] )return false;
	}
	return true;
}

static bool IsEqual2(const float m[16],const float m2[16] ){
	for( int i = 0;i<12;i++ ){
		if( m[i] != m2[i] )return false;
	}
	return true;
}

static void WorldDraw(){
	glBegin(GL_LINE_STRIP);

	glVertex3f(1498.98f,-4413.59f,23.144f);
	glVertex3f(1499.62f,-4413.6f,23.0392f);
	glVertex3f(1528.27f,-4414.89f,14.0982f);
	glVertex3f(1529.29f,-4416.01f,13.9524f);
	glVertex3f(1540.38f,-4426.46f,12.2898f);
	glVertex3f(1541.73f,-4425.86f,10.0442f);
	glVertex3f(1555.48f,-4417.9f,7.41515f);
	glVertex3f(1555.23f,-4416.43f,7.4073f);
	glVertex3f(1555.4f,-4411.11f,7.17876f);
	glVertex3f(1557.96f,-4404.78f,6.00415f);

	glEnd();
}

static void LocalDraw(){
	glLineWidth(3);
	glColor3f(0,0,1);
	glBegin(GL_LINES);
	glVertex3f(0,-1,0);
	glVertex3f(0,1,0);
	glVertex3f(-1,0,0);
	glVertex3f(1,0,0);
	glVertex3f(0,0,-1);
	glVertex3f(0,0,1);
	glEnd();
}

void myDraw(){
	if( !g_matrix_eye || g_Render.IsEmpty() )return;

	clac_matrix_eye();
	clac_matrix_projection();

	bool vb = false;
	bool fb = false;
	bool tb = false;
	bool lb = false;
	bool db = false;

	if( glIsEnabled(GL_VERTEX_PROGRAM_ARB) ){
		glDisable(GL_VERTEX_PROGRAM_ARB);
		vb = true;
	}
	if( glIsEnabled(GL_FRAGMENT_PROGRAM_ARB) ){
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
		fb = true;
	}
	if( glIsEnabled(GL_TEXTURE_2D) ){
		glDisable(GL_TEXTURE_2D);
		tb = true;
	}
	if( glIsEnabled(GL_LIGHTING)){
		glDisable(GL_LIGHTING);
		lb = true;
	}
	//打开深度缓冲
	if( !glIsEnabled(GL_DEPTH_TEST) ){
		glEnable(GL_DEPTH_TEST);
		glDepthMask(1);
		db = true;
	}
	glDepthRange(0,.94);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(matrix_projection);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(matrix_eye);

	g_Render.Draw();

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	if( vb )
		glEnable(GL_VERTEX_PROGRAM_ARB);
	if( fb )
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
	if( tb )
		glEnable(GL_TEXTURE_2D);
	if( lb )
		glEnable(GL_LIGHTING);
	if( db ){
		glDisable(GL_DEPTH_TEST);
		glDepthMask(0);
	}
}

/*
#include <string>
#include <vector>
#include <algorithm>

std::vector<std::string> vString;
*/
void APIENTRY myglProgramStringARB(GLenum a, GLenum b, GLsizei c, const GLvoid *d){
/*	if( a == GL_VERTEX_PROGRAM_ARB ){
		std::string str;
		str = (char*)d;
		if( std::find(vString.begin(),vString.end(),str)==vString.end() ){
			vString.push_back(str);
			printf("===============%X,%X=============\n",a,b);
			printf("%s\n",d);
		}
	}*/
	((PFNGLPROGRAMSTRINGARBPROC)pfnglProgramStringARB)(a,b,c,d);
}

void APIENTRY myglBindProgramARB(GLenum a, GLuint b){
//	if( a == GL_VERTEX_PROGRAM_ARB ){
//		printf("%X,%X\n",a,b);
//	}
	((PFNGLBINDPROGRAMARBPROC)pfnglBindProgramARB)(a,b);
}

FARPROC WINAPI mywglGetProcAddress( LPCSTR lpProcName ){
	printf("wglGetProcAddress:%s\n",lpProcName);
	if( strcmp(lpProcName,"glProgramStringARB")==0 ){//替换glProgramStringARB
		pfnglProgramStringARB = ((t_wglGetProcAddress)pfnGetProcAddress)(lpProcName);
		return (FARPROC)myglProgramStringARB;
	}else if( strcmp(lpProcName,"glBindProgramARB")==0 ){//替换glBindProgramARB
		pfnglBindProgramARB = ((t_wglGetProcAddress)pfnGetProcAddress)(lpProcName);
		return (FARPROC)myglBindProgramARB;
	}
	return ((t_wglGetProcAddress)pfnGetProcAddress)(lpProcName);
}

GLenum gl_mode;
bool is_eye = false;
bool is_pro = false;

void APIENTRY myglMatrixMode(GLenum mode){
	gl_mode = mode;
	((t_glMatrixMode)pfnglMatrixMode)(mode);
}

void APIENTRY myglLoadMatrixf(const GLfloat *m){
	((t_glLoadMatrixf)pfnglLoadMatrixf)(m);

	if( gl_mode == GL_MODELVIEW && g_matrix_eye ){
		clac_matrix_eye();
		if( IsEqual(m,matrix2) ){
			is_eye = true;
		}else is_eye = false;
		
		//printf("-------------------------------\n");
		//PrintMatrix(m);

	}
	if( gl_mode == GL_PROJECTION && g_matrix_projection ){
		clac_matrix_projection();
		if( IsEqual( m,matrix_projection ) ){
			is_pro = true;
		}else is_pro = false;
	}
}

void APIENTRY myglTexEnvi( GLenum target,GLenum pname,GLint param){
	((t_glTexEnvi)pfnglTexEnvi)(target,pname,param);
}

void APIENTRY myglTexEnvf( GLenum target,GLenum pname,GLfloat param){
	((t_glTexEnvf)pfnglTexEnvf)(target,pname,param);
}

void APIENTRY myglTexEnvfv( GLenum target,GLenum pname,const GLfloat* params){
	((t_glTexEnvfv)pfnglTexEnvfv)(target,pname,params);
}

void APIENTRY myglLightf( GLenum target,GLenum pname,GLfloat param){
	((t_glLightf)pfnglLightf)(target,pname,param);
}

void APIENTRY myglLightfv( GLenum target,GLenum pname,const GLfloat* params){
	((t_glLightfv)pfnglLightfv)(target,pname,params);
}

void APIENTRY myglBlendFunc(GLenum sfactor,GLenum dfactor){
	((t_glBlendFunc)pfnglBlendFunc)(sfactor,dfactor);
	//printf("glBlendFunc;\n");
}

void APIENTRY myglAlphaFunc(GLenum func,GLclampf ref){
	((t_glAlphaFunc)pfnglAlphaFunc)(func,ref);
	//printf("glAlphaFunc\n");
}

void APIENTRY myglDepthFunc(GLenum func){
	((t_glDepthFunc)pfnglDepthFunc)(func);
	//printf("glDepthFunc;\n");
}

void APIENTRY myglDepthRange(GLclampd zNear,GLclampd zFar){
	((t_glDepthRange)pfnglDepthRange)(zNear,zFar);
	//printf("glDepthRange(%d,%d)\n",zNear,zFar);
}

BOOL APIENTRY mywglMakeCurrent(HDC hdc,HGLRC hrc){
	return ((t_wglMakeCurrent)pfnwglMakeCurrent)(hdc,hrc);
}

/*
#include <vector>
#include <algorithm>
std::vector<GLenum> vE;
*/
void APIENTRY myglEnable(GLenum cap){
	((t_glEnable)pfnglEnable)(cap);
/*	if( std::find(vE.begin(),vE.end(),cap)==vE.end() ){
		vE.push_back(cap);
		printf("%X\n",cap);
	}*/
}

void APIENTRY myglDisable(GLenum cap){
	((t_glEnable)pfnglDisable)(cap);
}

void HookStandardApi(){
/*
	HMODULE hmodCaller = GetModuleHandle(sWow);
	//先挂接wglGetProcAddress,用来侦测wow使用的opengl扩展
	pfnGetProcAddress = GetProcAddress(GetModuleHandle(sOpengl32),"wglGetProcAddress");
	RelpaceEntry(sOpenglDll,pfnGetProcAddress,(PROC)mywglGetProcAddress,hmodCaller);
	//跟踪矩阵模式
	pfnglMatrixMode = GetProcAddress(GetModuleHandle(sOpengl32),"glMatrixMode");
	RelpaceEntry(sOpenglDll,pfnglMatrixMode,(PROC)myglMatrixMode,hmodCaller);
	//跟踪矩阵
	pfnglLoadMatrixf = GetProcAddress(GetModuleHandle(sOpengl32),"glLoadMatrixf");
	RelpaceEntry(sOpenglDll,pfnglLoadMatrixf,(PROC)myglLoadMatrixf,hmodCaller);
	//贴图混合
	pfnglTexEnvi = GetProcAddress(GetModuleHandle(sOpengl32),"glTexEnvi");
	RelpaceEntry(sOpenglDll,pfnglTexEnvi,(PROC)myglTexEnvi,hmodCaller);

	pfnglTexEnvf = GetProcAddress(GetModuleHandle(sOpengl32),"glTexEnvf");
	RelpaceEntry(sOpenglDll,pfnglTexEnvf,(PROC)myglTexEnvf,hmodCaller);

	pfnglTexEnvfv = GetProcAddress(GetModuleHandle(sOpengl32),"glTexEnvfv");
	RelpaceEntry(sOpenglDll,pfnglTexEnvfv,(PROC)myglTexEnvfv,hmodCaller);
	//灯
	pfnglLightf = GetProcAddress(GetModuleHandle(sOpengl32),"glLightf");
	RelpaceEntry(sOpenglDll,pfnglLightf,(PROC)myglLightf,hmodCaller);

	pfnglLightfv = GetProcAddress(GetModuleHandle(sOpengl32),"glLightfv");
	RelpaceEntry(sOpenglDll,pfnglLightfv,(PROC)myglLightfv,hmodCaller);
	//混合
	pfnglBlendFunc = GetProcAddress(GetModuleHandle(sOpengl32),"glBlendFunc");
	RelpaceEntry(sOpenglDll,pfnglBlendFunc,(PROC)myglBlendFunc,hmodCaller);

	pfnglAlphaFunc = GetProcAddress(GetModuleHandle(sOpengl32),"glAlphaFunc");
	RelpaceEntry(sOpenglDll,pfnglAlphaFunc,(PROC)myglAlphaFunc,hmodCaller);
	//深度
	pfnglDepthFunc = GetProcAddress(GetModuleHandle(sOpengl32),"glDepthFunc");
	RelpaceEntry(sOpenglDll,pfnglDepthFunc,(PROC)myglDepthFunc,hmodCaller);

	pfnglDepthRange = GetProcAddress(GetModuleHandle(sOpengl32),"glDepthRange");
	RelpaceEntry(sOpenglDll,pfnglDepthRange,(PROC)myglDepthRange,hmodCaller);
	//句柄
	pfnwglMakeCurrent = GetProcAddress(GetModuleHandle(sOpengl32),"wglMakeCurrent");
	RelpaceEntry(sOpenglDll,pfnwglMakeCurrent,(PROC)mywglMakeCurrent,hmodCaller);
	//Enable Disable
	pfnglEnable = GetProcAddress(GetModuleHandle(sOpengl32),"glEnable");
	RelpaceEntry(sOpenglDll,pfnglEnable,(PROC)myglEnable,hmodCaller);

	pfnglDisable = GetProcAddress(GetModuleHandle(sOpengl32),"glDisable");
	RelpaceEntry(sOpenglDll,pfnglDisable,(PROC)myglDisable,hmodCaller);
*/
}

void __stdcall SwapBuff(){
	__asm{
		push ecx
	}
	myDraw();
	__asm{
		pop ecx
		call g_pogSwapBuff
	}
}

void __stdcall ClearBuff(void* a,void* b){
	__asm{push ecx}
	__asm{
		pop ecx
		push b
		push a
		call g_pogClearBuff
	}
}

void __stdcall DrawScene(Grid pgrid[],int count,int mode){
	__asm{
		push ecx
	}
	__asm{
		pop ecx
		push mode
		push count
		push pgrid
		call g_pogDrawScene
	}
}

void __stdcall SetProjectionMatrix( float* m ){
	__asm{
		push ecx
	}
	__asm{
		pop ecx
		push m
		call g_pogSetProjectionMatrix
	}
}

void __stdcall SetModelViewMatrix( float* m ){
	__asm{
		push ecx
	}
	__asm{
		pop ecx
		push m
		call g_pogSetModelViewMatrix
	}
}

static int iSwapBuff = -1;

void SetRenderHook(){
	if( g_iGfxType == 0 && g_pGfx && !s_bRenderHookIsSet ){
		g_pGXInterface = (void**)(*((void**)g_pGfx));
		if( g_pGXInterface ){
			DWORD op;
			bool  isok = false;
			printf("SetRenderHook g_pGXInterface=%X\n",g_pGXInterface);
			//这里直接改变界面的函数入口,这样调用开销非常少
			VirtualProtect( GET_PAGE(g_pGXInterface),PAGE_SIZE,PAGE_EXECUTE_READWRITE,&op );
			//SwapBuff
			//这里需要做些检查，因为图形接口也回变化
			iSwapBuff = 34;
			for( int i = 0;i < 8;i++ ){ //这里检查接口38附件+-4返回里面的函数
				g_pogSwapBuff = (t_SwapBuff)(*(g_pGXInterface+iSwapBuff));
				if( g_pogSwapBuff == g_chSwapBuff.pEntryFunc ){
					printf("SwapBuf = %X ,iSwapBuff = %d PASS\n",g_pogSwapBuff,iSwapBuff );
					*(g_pGXInterface+iSwapBuff) = SwapBuff;
					isok = true;
					break;
				}
				iSwapBuff++;
			}
			if( !isok ){
				iSwapBuff = -1;
				printf("Can't find SwapBuff in g_pGXInterface TABLE\n");
			}
			//ClearBuff
			//g_pogClearBuff = (t_ClearBuff)(*(g_pGXInterface+39));
			//*(g_pGXInterface+39) = ClearBuff;
			//DrawScene
			//g_pogDrawScene = (t_DrawScene)(*(g_pGXInterface+42));
			//*(g_pGXInterface+42) = DrawScene;
			//SetProjectionMatrix
			//g_pogSetProjectionMatrix = (t_SetMatrix)(*(g_pGXInterface+40));
			//*(g_pGXInterface+40) = SetProjectionMatrix;
			//SetModelViewMatrix
			//g_pogSetModelViewMatrix = (t_SetMatrix)(*(g_pGXInterface+41));
			//*(g_pGXInterface+41) = SetModelViewMatrix;
			VirtualProtect( GET_PAGE(g_pGXInterface),PAGE_SIZE,PAGE_EXECUTE_READ,&op );
			s_bRenderHookIsSet = true;
		}
	}	
}

void ClearRenderHook(){
	if( g_iGfxType == 0 && g_pGfx && s_bRenderHookIsSet ){
		DWORD op;
		VirtualProtect( GET_PAGE(g_pGXInterface),PAGE_SIZE,PAGE_EXECUTE_READWRITE,&op );
		if( iSwapBuff != -1 )
			*(g_pGXInterface+iSwapBuff) = g_pogSwapBuff;
	//	*(g_pGXInterface+42) = g_pogDrawScene;
	//	*(g_pGXInterface+40) = g_pogSetProjectionMatrix;
	//	*(g_pGXInterface+41) = g_pogSetModelViewMatrix;
	//	*(g_pGXInterface+39) = g_pogClearBuff;
		VirtualProtect( GET_PAGE(g_pGXInterface),PAGE_SIZE,PAGE_EXECUTE_READ,&op );
		s_bRenderHookIsSet = false;
	}
}


LinePoint::LinePoint(){
	code = 0;
	x = 0;
	y = 0;
	z = 0;
}

LinePoint::LinePoint(int c,float w){
	code = c;
	x = w;
	y = w;
	z = w;
}

LinePoint::LinePoint(int c,float xx,float yy,float zz){
	code = c;
	x = xx;
	y = yy;
	z = zz;
}

LinePoint::LinePoint(int c,float xx,float yy,float zz,const char* txt){
	code = c;
	x = xx;
	y = yy;
	z = zz;
	text = txt;
}

Render::Render(){
	listArrow = -1;
	listBox = -1;
	listCircle = -1;
	listRect = -1;
	listTriangle = -1;
	for( int i = 0;i < sizeof(listChar)/sizeof(GLuint);i++ ){
		listChar[i] = - 1;
	}
}

void Render::Draw(){
	for( std::vector<RenderItem>::iterator i = vpi.begin();
		i!=vpi.end();i++ ){
		if( i->b ){
			if( i->o )
				glDisable(GL_DEPTH_TEST);
			glPushMatrix();
			glTranslatef(i->ox,i->oy,i->oz);
			glRotatef(i->rta,i->rtx,i->rty,i->rtz);
			glScalef(i->s,i->s,i->s);
			glCallList(i->list);
			glPopMatrix();
			if( i->o )
				glEnable(GL_DEPTH_TEST);
		}
	}
}

bool Render::IsEmpty(){
	return vpi.empty();
}

void Render::Begin( std::string name ){
	if( name.empty() )return;
	Clear( name );

	if( !vlp.empty() )vlp.clear();

	current.name = name;
	current.ox = 0;
	current.oy = 0;
	current.oz = 0;
	current.rtx = 0;
	current.rty = 0;
	current.rtz = 1;
	current.rta = 0;
	current.s = 1;
	current.b = true;
	current.o = false;
	bLineStipple = false;
	fWidth = 1;
}

void Render::Translate(std::string name,float x,float y,float z){
	if( current.name == name ){
		current.ox = x;
		current.oy = y;
		current.oz = z;
	}else{
		for( std::vector<RenderItem>::iterator i = vpi.begin();
			i!=vpi.end();i++ ){
			if( i->name == name ){
				i->ox = x;
				i->oy = y;
				i->oz = z;
				break;
			}
		}
	}
}

void Render::Rotate(std::string name,float a,float x,float y,float z){
	if( current.name == name ){
		current.rtx = x;
		current.rty = y;
		current.rtz = z;
		current.rta = a;
	}else{
		for( std::vector<RenderItem>::iterator i = vpi.begin();
			i!=vpi.end();i++ ){
			if( i->name == name ){
				i->rtx = x;
				i->rty = y;
				i->rtz = z;
				i->rta = a;
				break;
			}
		}
	}
}

void Render::Show(std::string name,bool b){
	if( current.name == name ){
		current.b = b;
	}else{
		for( std::vector<RenderItem>::iterator i = vpi.begin();
			i!=vpi.end();i++ ){
			if( i->name == name ){
				i->b = b;
				break;
			}
		}
	}
}

void Render::Overlay(std::string name,bool b){
	if( current.name == name ){
		current.o = b;
	}else{
		for( std::vector<RenderItem>::iterator i = vpi.begin();
			i!=vpi.end();i++ ){
			if( i->name == name ){
				i->o = b;
				break;
			}
		}
	}
}

void Render::Scale(std::string name,float a){
	if( current.name == name ){
		current.s = a;
	}else{
		for( std::vector<RenderItem>::iterator i = vpi.begin();
			i!=vpi.end();i++ ){
			if( i->name == name ){
				i->s = a;
				break;
			}
		}
	}
}

void Render::LineWidth( float w ){
	vlp.push_back( LinePoint(LP_LINEWIDTH,w) );	
}

void Render::Color( float r,float g,float b ){
	vlp.push_back(LinePoint(LP_COLOR,r,g,b));
}

void Render::MoveTo( float x,float y,float z ){
	vlp.push_back(LinePoint(LP_MOVETO,x,y,z));
}

void Render::LineTo( float x,float y,float z ){
	vlp.push_back(LinePoint(LP_LINETO,x,y,z));
}

void Render::Normal( float x,float y,float z ){
	float d = sqrt(x*x+y*y+z*z);
	if( d != 0 ){
		x /= d;
		y /= d;
		z /= d;
	}
	vlp.push_back(LinePoint(LP_NORMAL,x,y,z));
}

void Render::Size( float s ){
	vlp.push_back(LinePoint(LP_SIZE,s));
}

void Render::Stipple( float s){
	vlp.push_back(LinePoint(LP_STIPPLE,s));
}

void Render::Box( float x,float y,float z ){
	vlp.push_back(LinePoint(LP_BOX,x,y,z));
}

void Render::Circle( float x,float y,float z ){
	vlp.push_back(LinePoint(LP_CIRCLE,x,y,z));
}

void Render::String(float x,float y,float z,const char* text){
	vlp.push_back(LinePoint(LP_STRING,x,y,z,text));
}

void Render::Rect( float x,float y,float z ){
	vlp.push_back(LinePoint(LP_RECT,x,y,z));
}

void Render::Triangle( float x,float y,float z ){
	vlp.push_back(LinePoint(LP_TRIANGLE,x,y,z));
}

void Render::Arrow( float x,float y,float z ){
	vlp.push_back(LinePoint(LP_ARROW,x,y,z));
}

//cos(30)=0.866
#define COS30 .866f
#define SIN30 .5f

static GLfloat fArrow[]={
	0,.2f,.0,
	.2f*COS30,-.2f*SIN30,0,
	-.2f*COS30,-.2f*SIN30,0,
	0,0,1
};

static GLubyte iArrow[]={
	2,1,0,1,3,0,
	3,2,0,2,3,1
};

static GLfloat fBox[]={
	.5f,.5f,.5f,
	-.5f,.5f,.5f,
	-.5f,-.5f,.5f,
	.5f,-.5f,.5f,
	.5f,.5f,-.5f,
	-.5f,.5f,-.5f,
	-.5f,-.5f,-.5f,
	.5f,-.5f,-.5f
};

static GLubyte iBox[]={
	7,6,5,4,5,6,2,1,
	4,5,1,0,1,2,3,0,
	3,7,4,0,6,7,3,2
};

bool Render::BuildArrow(){
	int index;
	if( listArrow==-1 || !glIsList(listArrow) ){
		listArrow = glGenLists(1);
		glNewList(listArrow,GL_COMPILE);
		glBegin(GL_TRIANGLES);
		for( int i = 0;i < 12;i++ ){
			index = 3*iArrow[i];
			glVertex3f(fArrow[index],fArrow[index+1],fArrow[index+2]);
		}
		glEnd();
		glEndList();
		return true;
	}
	return false;
}

void Render::BuildCharacter( int c,int n,float* g ){
	if( c < 0 || c >= 256 )return;
	float ox,oy,w;
	if( listChar[c]==-1 || !glIsList(listChar[c]) ){
		float minx,miny,maxx,maxy;
		miny = minx = FLT_MAX;
		maxx = maxy = -FLT_MAX;
		for( int i = 0;i<n;i+=2 ){
			minx = min(minx,*(g+i));
			miny = min(miny,*(g+i+1));
			maxx = max(maxx,*(g+i));
			maxy = max(maxy,*(g+i+1));
		}
		ox = (maxx+minx)/2;
		oy = (maxy+miny)/2;
		w = maxy-miny;
		listChar[c] = glGenLists(1);
		glNewList(listChar[c],GL_COMPILE);
		
		if( c=='.' ){
			w = 1;
			ox = 0;
			oy = 0;
		}
		if( c=='4' ){
			glBegin(GL_LINE_STRIP);
			for( i = 0;i < 6;i+=2 ){
				glVertex3f((*(g+i)-ox)/w,(*(g+i+1)-oy)/w,0);
			}
			glEnd();
			glBegin(GL_LINE_STRIP);
			for( i = 6;i < n;i+=2 ){
				glVertex3f((*(g+i)-ox)/w,(*(g+i+1)-oy)/w,0);
			}
			glEnd();
		}else{
			glBegin(GL_LINE_STRIP);
			for( i = 0;i < n;i+=2 ){
				glVertex3f((*(g+i)-ox)/w,(*(g+i+1)-oy)/w,0);
			}
			glEnd();
		}
		
		glEndList();
	}
}

#pragma warning( disable : 4305 )

static float num0[]={
	 -191.06092,129.25365,
	 -180.70378,133.53936,
	 -168.91806,150.68222,
	 -161.77521,178.53936,
	 -164.27521,227.11079,
	 -175.34663,251.39651,
	 -187.48949,261.39651,
	 -197.13235,260.68222,
	 -208.91806,249.96793,
	 -218.56092,223.53936,
	 -221.06092,195.32508,
	 -219.27521,166.75365,
	 -207.13235,138.89651,
	 -200.34663,131.75365,
	 -191.06092,129.25365
};

static float num1[]={
	-58.203772,160.32508,
	-27.846629,130.68222,
	-27.132344,263.89651
};

static float num2[]={
	78.224799,152.82508,
	82.510514,141.03936,
	96.081945,129.96793,
	118.22479,131.03936,
	134.65337,147.11079,
	133.22479,171.75365,
	77.153371,261.75365,
	138.22479,261.75365
};

static float num3[]={
	224.29622,129.96793,
	264.29622,130.68222,
	281.08194,139.96793,
	285.36765,158.89651,
	279.29622,177.46793,
	267.51051,186.03936,
	242.15337,188.18222,
	267.51051,189.61079,
	284.29622,208.89651,
	285.72479,237.11079,
	269.65337,258.18222,
	259.29622,261.75365,
	223.93908,261.39651
};

static float num4[]={
	397.86765,127.82508,
	366.79622,233.18222,
	443.93908,232.82508,
	418.93908,202.11079,
	419.29622,262.82508
};

static float num5[]={
	576.43908,128.53936,
	522.86765,128.18222,
	524.65337,188.53936,
	562.51051,189.25365,
	581.08194,201.39651,
	583.93908,221.03936,
	580.01051,248.18222,
	561.79622,261.75365,
	522.15337,261.03936
};

static float num6[]={
	719.65337,129.25365,
	695.01051,141.03936,
	675.01051,171.03936,
	671.79622,203.53936,
	675.36765,247.11079,
	696.08194,262.11079,
	717.86765,259.61079,
	731.79622,238.18222,
	732.51051,204.25365,
	719.65337,189.61079,
	673.2248,188.18222
};

static float num7[]={
	823.2248,143.89651,
	822.51051,129.25365,
	882.15337,130.32508,
	853.2248,254.96793,
	852.86765,262.82508
};

static float num8[]={
	988.2248,185.68222,
	973.2248,172.11079,
	972.15337,149.96793,
	983.58194,133.53936,
	1003.582,128.18222,
	1023.582,138.89651,
	1031.4391,161.39651,
	1025.7248,175.68222,
	1010.0105,186.03936,
	991.08194,189.25365,
	973.58194,205.32508,
	971.43908,243.53936,
	986.08194,258.53936,
	1005.3677,261.39651,
	1028.582,247.82508,
	1031.082,223.53936,
	1029.2962,204.25365,
	1020.0105,194.61079,
	1007.5105,188.53936,
	988.2248,185.68222
};

static float num9[]={
	1178.2248,202.82508,
	1141.4391,203.18222,
	1120.0105,188.53936,
	1119.2962,158.53936,
	1128.2248,136.39651,
	1147.8677,128.89651,
	1166.4391,133.89651,
	1180.3677,152.11079,
	1178.2248,212.11079,
	1166.7962,238.53936,
	1145.3677,256.75365,
	1133.2248,261.39651
};

static float numdot[]={
	0.1,0.5,
	-0.1,0.4,

};

#pragma warning( default : 4305 )

#define BuildCHAR( c,num ) BuildCharacter( c,sizeof(num)/sizeof(float),num )

void Render::BuildChar(){
	//0-9
	BuildCHAR( '0',num0 );
	BuildCHAR( '1',num1 );
	BuildCHAR( '2',num2 );
	BuildCHAR( '3',num3 );
	BuildCHAR( '4',num4 );
	BuildCHAR( '5',num5 );
	BuildCHAR( '6',num6 );
	BuildCHAR( '7',num7 );
	BuildCHAR( '8',num8 );
	BuildCHAR( '9',num9 );
	BuildCHAR( '.',numdot );
}

void Render::BuildBox(){
	int index;
	if( listBox==-1 || !glIsList(listBox)){
		listBox = glGenLists(1);
		glNewList(listBox,GL_COMPILE);
		glBegin(GL_QUADS);
		for( int i = 0;i < 24;i++ ){
			index = 3*iBox[i];
			glVertex3f(fBox[index],fBox[index+1],fBox[index+2]);
		}
		glEnd();
		glEndList();
	}
}

void Render::BuildCircle(){
	float x,y;
	if( listCircle==-1 || !glIsList(listCircle)  ){
		listCircle = glGenLists(1);
		glNewList(listCircle,GL_COMPILE);
		glBegin(GL_LINE_LOOP);
		for( int i = 0;i < 360;i+=15 ){
			x = cos(M_PI/180.0*i);
			y = sin(M_PI/180.0*i);
			glVertex3f(x,y,0);
		}
		glEnd();
		glEndList();
	}
}

void Render::BuildRect(){
	if( listRect==-1 || !glIsList(listRect)  ){
		listRect = glGenLists(1);
		glNewList(listRect,GL_COMPILE);
		glBegin(GL_LINE_LOOP);
		glVertex3f(1,1,0);
		glVertex3f(-1,1,0);
		glVertex3f(-1,-1,0);
		glVertex3f(1,-1,0);
		glEnd();
		glEndList();
	}
}

void Render::BuildTriangle(){
	if( listTriangle==-1 || !glIsList(listTriangle)  ){
		listTriangle = glGenLists(1);
		glNewList(listTriangle,GL_COMPILE);
		glBegin(GL_LINE_LOOP);
		glVertex3f(0,1,0);
		glVertex3f(-cos(M_PI/6),-sin(M_PI/6),0);
		glVertex3f(cos(M_PI/6),-sin(M_PI/6),0);
		glEnd();
		glEndList();
	}
}

void Render::glString( float ov[3],float on[3],float s,std::string text ){
	float v[3];
	float z[3];
	float a;
	z[0] = 0;
	z[1] = 0;
	z[2] = -1;
	vector_mul(v,on,z);

	if( text.empty() )return;

	a = vector_dot(on,z);
	a = a>1?1:a;
	a = a<-1?-1:a;
	a = (float)(acos(a)*180/M_PI);
	if( v[0]==0&&v[1]==0&&v[2]==0 )//如果是对角的就随便选择一个垂直矢量
		v[0] = 1;

	glPushMatrix();
	glTranslatef(ov[0],ov[1],ov[2]);
	glRotatef(a,v[0],v[1],v[2]);
	glScalef(s,s,s);
	for( int i = 0;i < text.size();i++ ){
		unsigned char c = (unsigned char)text[i];
		if( listChar[c] != -1 ){
			glTranslatef(0.8f,0,0);
			glCallList(listChar[c]);
		}
	}
	glPopMatrix();	
}

void Render::glCircle( float ov[3],float on[3],float s){
	float v[3];
	float z[3];
	float a;
	z[0] = 0;
	z[1] = 0;
	z[2] = -1;
	vector_mul(v,on,z);

	a = vector_dot(on,z);
	a = a>1?1:a;
	a = a<-1?-1:a;
	a = (float)(acos(a)*180/M_PI);
	if( v[0]==0&&v[1]==0&&v[2]==0 )//如果是对角的就随便选择一个垂直矢量
		v[0] = 1;

	glPushMatrix();
	glTranslatef(ov[0],ov[1],ov[2]);
	glRotatef(a,v[0],v[1],v[2]);
	glScalef(s,s,s);
	glCallList(listCircle);
	glPopMatrix();	
}

void Render::glRect( float ov[3],float on[3],float s){
	float v[3];
	float z[3];
	float a;
	z[0] = 0;
	z[1] = 0;
	z[2] = -1;
	vector_mul(v,on,z);

	a = vector_dot(on,z);
	a = a>1?1:a;
	a = a<-1?-1:a;
	a = (float)(acos(a)*180/M_PI);
	if( v[0]==0&&v[1]==0&&v[2]==0 )//如果是对角的就随便选择一个垂直矢量
		v[0] = 1;

	glPushMatrix();
	glTranslatef(ov[0],ov[1],ov[2]);
	glRotatef(a,v[0],v[1],v[2]);
	glScalef(s,s,s);
	glCallList(listRect);
	glPopMatrix();	
}

void Render::glTriangle( float ov[3],float on[3],float s){
	float v[3];
	float z[3];
	float a;
	z[0] = 0;
	z[1] = 0;
	z[2] = -1;
	vector_mul(v,on,z);

	a = vector_dot(on,z);
	a = a>1?1:a;
	a = a<-1?-1:a;
	a = (float)(acos(a)*180/M_PI);
	if( v[0]==0&&v[1]==0&&v[2]==0 )//如果是对角的就随便选择一个垂直矢量
		v[0] = 1;

	glPushMatrix();
	glTranslatef(ov[0],ov[1],ov[2]);
	glRotatef(a,v[0],v[1],v[2]);
	glScalef(s,s,s);
	glCallList(listTriangle);
	glPopMatrix();	
}

void Render::glBox(float ov[3],float on[3],float s){
	float v[3];
	float z[3];
	float a;
	z[0] = 0;
	z[1] = 0;
	z[2] = -1;
	vector_mul(v,on,z);

	a = vector_dot(on,z);
	a = a>1?1:a;
	a = a<-1?-1:a;
	a = (float)(acos(a)*180/M_PI);
	if( v[0]==0&&v[1]==0&&v[2]==0 )//如果是对角的就随便选择一个垂直矢量
		v[0] = 1;

	glPushMatrix();
	glTranslatef(ov[0],ov[1],ov[2]);
	glRotatef(a,v[0],v[1],v[2]);
	glScalef(s,s,s);
	glCallList(listBox);
	glPopMatrix();	
}

void Render::glArrow(float ov[3],float on[3],float s){
	float v[3];
	float z[3];
	float a;
	z[0] = 0;
	z[1] = 0;
	z[2] = -1;
	vector_mul(v,on,z);

	a = vector_dot(on,z);
	a = a>1?1:a;
	a = a<-1?-1:a;
	a = (float)(acos(a)*180/M_PI);
	if( v[0]==0&&v[1]==0&&v[2]==0 )//如果是对角的就随便选择一个垂直矢量
		v[0] = 1;
	
//	a = vector_mod(v);
//	a = a>1?1:a;
//	a = (float)(asin(a)*180/M_PI);

	glPushMatrix();
	glTranslatef(ov[0],ov[1],ov[2]);
	glRotatef(a,v[0],v[1],v[2]);
	glScalef(s,s,s);
	glCallList(listArrow);
	glPopMatrix();
}

void Render::End(){
	bool b = false;
	int  n = 0;
	float x,y,z;
	float nx=0,ny=0,nz=-1;
	float s = 1;
	float ov[3],nv[3];

	if( !vlp.empty() ){
		//这里假设：如果一个glist失效，则其他的也失效。
		if( BuildArrow() ){
			BuildBox();
			BuildCircle();
			BuildRect();
			BuildTriangle();
			BuildChar();
		}

		current.list = glGenLists(1);

//		glMatrixMode(GL_MODELVIEW);
//		glPushMatrix();
//		glLoadIdentity();

		glNewList(current.list,GL_COMPILE);
		for( std::vector<LinePoint>::iterator i = vlp.begin();
			i!=vlp.end();i++){
			switch(i->code){
				case LP_MOVETO:
					if( b ){
						if( n < 1 )//补一个点
							glVertex3f(x,y,z);
						glEnd();
						if( !bLineStipple ){
							glDisable(GL_LINE_STIPPLE);
						}else{
							glEnable(GL_LINE_STIPPLE);
							glLineStipple(1,0x00ff);
						}
						glLineWidth(fWidth);
					}
					glBegin(GL_LINE_STRIP);
					glVertex3f(i->x,i->y,i->z);
					x = i->x;
					y = i->y;
					z = i->z;
					b = true;
					n = 0;
					break;
				case LP_LINETO:
					if( b ){
						glVertex3f(i->x,i->y,i->z);
						x = i->x;
						y = i->y;
						z = i->z;
						n++;
					}
					break;
				case LP_LINEWIDTH:
					if( i->x>0&&i->x<10 ){
						glLineWidth(i->x);
						fWidth = i->x;
					}else{
						glLineWidth(1);
						fWidth = 1;
					}
					break;
				case LP_STIPPLE:
					if( i->x==1 ){
						if( !b ){
							glEnable(GL_LINE_STIPPLE);
							glLineStipple(1,0x00ff);
						}
						bLineStipple = true;
					}else{
						bLineStipple = false;
					}
					break;
				case LP_COLOR:
					glColor3f(i->x,i->y,i->z);
					break;
			}
		}
		if( b ){
			if( n < 1 )//补一个点
				glVertex3f(x,y,z);
			glEnd();
			if( !bLineStipple ){
				glDisable(GL_LINE_STIPPLE);
			}else{
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1,0x00ff);
			}
			glLineWidth(fWidth);
		}
		for( std::vector<LinePoint>::iterator i = vlp.begin();
			i!=vlp.end();i++){
			switch(i->code){
				case LP_ARROW:
					ov[0] = i->x;
					ov[1] = i->y;
					ov[2] = i->z;
					nv[0] = nx;
					nv[1] = ny;
					nv[2] = nz;
					glArrow(ov,nv,s);
					break;
				case LP_BOX:
					ov[0] = i->x;
					ov[1] = i->y;
					ov[2] = i->z;
					nv[0] = nx;
					nv[1] = ny;
					nv[2] = nz;
					glBox(ov,nv,s);
					break;
				case LP_CIRCLE:
					ov[0] = i->x;
					ov[1] = i->y;
					ov[2] = i->z;
					nv[0] = nx;
					nv[1] = ny;
					nv[2] = nz;
					glCircle(ov,nv,s);
					break;
				case LP_STRING:
					ov[0] = i->x;
					ov[1] = i->y;
					ov[2] = i->z;
					nv[0] = nx;
					nv[1] = ny;
					nv[2] = nz;
					glString(ov,nv,s,i->text);
					break;
				case LP_RECT:
					ov[0] = i->x;
					ov[1] = i->y;
					ov[2] = i->z;
					nv[0] = nx;
					nv[1] = ny;
					nv[2] = nz;
					glRect(ov,nv,s);
					break;
				case LP_TRIANGLE:
					ov[0] = i->x;
					ov[1] = i->y;
					ov[2] = i->z;
					nv[0] = nx;
					nv[1] = ny;
					nv[2] = nz;
					glTriangle(ov,nv,s);
					break;
				case LP_NORMAL:
					nx = i->x;
					ny = i->y;
					nz = i->z;
					break;
				case LP_SIZE:
					s = i->x;
					break;
				case LP_COLOR:
					glColor3f(i->x,i->y,i->z); //颜色有一些多余的调用
					break;
			}
		}
		glEndList();

//		glPopMatrix();

		vlp.clear();
		vpi.push_back(current);
		current.name.clear();
	}else{
		Clear(current.name);
		current.name.clear();
	}
}

void Render::Clear( std::string name ){
	for( std::vector<RenderItem>::iterator i = vpi.begin();
		i!=vpi.end();i++ ){
		if( i->name == name ){
			glDeleteLists(i->list,1);
			vpi.erase(i);
			break;
		}
	}
}

void Render::ClearAll(){
	for( std::vector<RenderItem>::iterator i = vpi.begin();
		i!=vpi.end();i++ ){
		glDeleteLists(i->list,1);
	}
	vpi.clear();
	printf("ClearALL()\n");
}

int lua_RenderBegin( void* p ){
	if( Lua_isstring(p,1) ){
		size_t len;
		const char* name = Lua_tolstring(p,1,&len);
		if( name && strlen( name ) > 0 ){
			g_Render.Begin( name );
			Lua_pushnumber(p,1);
			return 1;
		}
	}
	Lua_pushnil( p );
	return 1;
}

int lua_RenderEnd( void* p ){
	g_Render.End();
	Lua_pushnil( p );
	return 1;
}

int lua_RenderClear( void* p ){
	if( Lua_isstring(p,1) ){
		size_t len;
		const char* name = Lua_tolstring(p,1,&len);
		if( name && strlen( name ) > 0 ){
			g_Render.Clear( name );
		}else{
			g_Render.ClearAll();
		}
		Lua_pushnumber(p,1);
		return 1;
	}else{
		g_Render.ClearAll();
		Lua_pushnumber(p,1);
		return 1;
	}
	Lua_pushnil( p );
	return 1;
}

int lua_RenderMoveTo( void* p ){
	bool isok = true;
	float x,y,z;

	if( Lua_isnumber(p,1) ){
		x = (float)Lua_tonumber(p,1);
	}else isok = false;
	if( Lua_isnumber(p,2) ){
		y = (float)Lua_tonumber(p,2);
	}else isok = false;
	if( Lua_isnumber(p,3) ){
		z = (float)Lua_tonumber(p,3);
	}else isok = false;

	if( isok ){
		g_Render.MoveTo( x,y,z);
		Lua_pushnumber(p,1);
		return 1;
	}

	Lua_pushnil( p );
	return 1;
}

int lua_RenderLineTo( void* p ){
	bool isok = true;
	float x,y,z;

	if( Lua_isnumber(p,1) ){
		x = (float)Lua_tonumber(p,1);
	}else isok = false;
	if( Lua_isnumber(p,2) ){
		y = (float)Lua_tonumber(p,2);
	}else isok = false;
	if( Lua_isnumber(p,3) ){
		z = (float)Lua_tonumber(p,3);
	}else isok = false;

	if( isok ){
		g_Render.LineTo( x,y,z);
		Lua_pushnumber(p,1);
		return 1;
	}

	Lua_pushnil( p );
	return 1;
}

int lua_RenderColor( void* p ){
	bool isok = true;
	float r,g,b;

	if( Lua_isnumber(p,1) ){
		r = (float)Lua_tonumber(p,1);
	}else isok = false;
	if( Lua_isnumber(p,2) ){
		g = (float)Lua_tonumber(p,2);
	}else isok = false;
	if( Lua_isnumber(p,3) ){
		b = (float)Lua_tonumber(p,3);
	}else isok = false;

	if( isok ){
		g_Render.Color( r,g,b);
		Lua_pushnumber(p,1);
		return 1;
	}

	Lua_pushnil( p );
	return 1;
}

int lua_RenderLineWidth( void* p ){
	if( Lua_isnumber(p,1) ){
		float w = (float)Lua_tonumber(p,1);
		g_Render.LineWidth( w );
		Lua_pushnumber(p,1);
		return 1;
	}
	Lua_pushnil( p );
	return 1;
}

int lua_RenderStipple( void* p ){
	if( Lua_isnumber(p,1) ){
		float w = (float)Lua_tonumber(p,1);
		g_Render.Stipple( w );
		Lua_pushnumber(p,1);
		return 1;
	}
	Lua_pushnil( p );
	return 1;
}

int lua_RenderCircle( void* p ){
	bool isok = true;
	float x,y,z;

	if( Lua_isnumber(p,1) ){
		x = (float)Lua_tonumber(p,1);
	}else isok = false;
	if( Lua_isnumber(p,2) ){
		y = (float)Lua_tonumber(p,2);
	}else isok = false;
	if( Lua_isnumber(p,3) ){
		z = (float)Lua_tonumber(p,3);
	}else isok = false;

	if( isok ){
		g_Render.Circle( x,y,z);
		Lua_pushnumber(p,1);
		return 1;
	}

	Lua_pushnil( p );
	return 1;
}

int lua_RenderRect( void* p ){
	bool isok = true;
	float x,y,z;

	if( Lua_isnumber(p,1) ){
		x = (float)Lua_tonumber(p,1);
	}else isok = false;
	if( Lua_isnumber(p,2) ){
		y = (float)Lua_tonumber(p,2);
	}else isok = false;
	if( Lua_isnumber(p,3) ){
		z = (float)Lua_tonumber(p,3);
	}else isok = false;

	if( isok ){
		g_Render.Rect( x,y,z);
		Lua_pushnumber(p,1);
		return 1;
	}

	Lua_pushnil( p );
	return 1;
}

int lua_RenderTriangle( void* p ){
	bool isok = true;
	float x,y,z;

	if( Lua_isnumber(p,1) ){
		x = (float)Lua_tonumber(p,1);
	}else isok = false;
	if( Lua_isnumber(p,2) ){
		y = (float)Lua_tonumber(p,2);
	}else isok = false;
	if( Lua_isnumber(p,3) ){
		z = (float)Lua_tonumber(p,3);
	}else isok = false;

	if( isok ){
		g_Render.Triangle( x,y,z);
		Lua_pushnumber(p,1);
		return 1;
	}

	Lua_pushnil( p );
	return 1;
}

int lua_RenderBox( void* p ){
	bool isok = true;
	float x,y,z;

	if( Lua_isnumber(p,1) ){
		x = (float)Lua_tonumber(p,1);
	}else isok = false;
	if( Lua_isnumber(p,2) ){
		y = (float)Lua_tonumber(p,2);
	}else isok = false;
	if( Lua_isnumber(p,3) ){
		z = (float)Lua_tonumber(p,3);
	}else isok = false;

	if( isok ){
		g_Render.Box( x,y,z);
		Lua_pushnumber(p,1);
		return 1;
	}

	Lua_pushnil( p );
	return 1;
}

int lua_RenderArrow( void* p ){
	bool isok = true;
	float x,y,z;

	if( Lua_isnumber(p,1) ){
		x = (float)Lua_tonumber(p,1);
	}else isok = false;
	if( Lua_isnumber(p,2) ){
		y = (float)Lua_tonumber(p,2);
	}else isok = false;
	if( Lua_isnumber(p,3) ){
		z = (float)Lua_tonumber(p,3);
	}else isok = false;

	if( isok ){
		g_Render.Arrow( x,y,z);
		Lua_pushnumber(p,1);
		return 1;
	}

	Lua_pushnil( p );
	return 1;
}

int lua_RenderString( void* p ){
	bool isok = true;
	float x,y,z;
	const char* text;
	if( Lua_isnumber(p,1) ){
		x = (float)Lua_tonumber(p,1);
	}else isok = false;
	if( Lua_isnumber(p,2) ){
		y = (float)Lua_tonumber(p,2);
	}else isok = false;
	if( Lua_isnumber(p,3) ){
		z = (float)Lua_tonumber(p,3);
	}else isok = false;
	if( Lua_isstring(p,4) ){
		size_t len;
		text = Lua_tolstring(p,4,&len);
	}else isok = false;

	if( isok ){
		g_Render.String( x,y,z,text);
		Lua_pushnumber(p,1);
		return 1;
	}

	Lua_pushnil( p );
	return 1;
}

int lua_RenderNormal( void* p ){
	bool isok = true;
	float x,y,z;

	if( Lua_isnumber(p,1) ){
		x = (float)Lua_tonumber(p,1);
	}else isok = false;
	if( Lua_isnumber(p,2) ){
		y = (float)Lua_tonumber(p,2);
	}else isok = false;
	if( Lua_isnumber(p,3) ){
		z = (float)Lua_tonumber(p,3);
	}else isok = false;

	if( isok ){
		g_Render.Normal( x,y,z);
		Lua_pushnumber(p,1);
		return 1;
	}

	Lua_pushnil( p );
	return 1;
}

int lua_RenderSize( void* p ){
	if( Lua_isnumber(p,1) ){
		float w = (float)Lua_tonumber(p,1);
		g_Render.Size( w );
		Lua_pushnumber(p,1);
		return 1;
	}
	Lua_pushnil( p );
	return 1;
}

int lua_RenderTranslate( void* p ){
	bool isok = true;
	float x,y,z;

	if( Lua_isstring(p,1) ){
		size_t len;
		const char* name = Lua_tolstring(p,1,&len);

		if( Lua_isnumber(p,2) ){
			x = (float)Lua_tonumber(p,2);
		}else isok = false;
		if( Lua_isnumber(p,3) ){
			y = (float)Lua_tonumber(p,3);
		}else isok = false;
		if( Lua_isnumber(p,4) ){
			z = (float)Lua_tonumber(p,4);
		}else isok = false;

		if(isok ){
			g_Render.Translate( name,x,y,z );
			Lua_pushnumber(p,1);
			return 1;
		}
	}
	Lua_pushnil( p );
	return 1;
}

int lua_RenderRotate( void* p ){
	bool isok = true;
	float x,y,z,a;

	if( Lua_isstring(p,1) ){
		size_t len;
		const char* name = Lua_tolstring(p,1,&len);

		if( Lua_isnumber(p,2) ){
			a = (float)Lua_tonumber(p,2);
		}else isok = false;
		if( Lua_isnumber(p,3) ){
			x = (float)Lua_tonumber(p,3);
		}else isok = false;
		if( Lua_isnumber(p,4) ){
			y = (float)Lua_tonumber(p,4);
		}else isok = false;
		if( Lua_isnumber(p,5) ){
			z = (float)Lua_tonumber(p,5);
		}else isok = false;

		if(isok ){
			g_Render.Rotate( name,a,x,y,z );
			Lua_pushnumber(p,1);
			return 1;
		}
	}
	Lua_pushnil( p );
	return 1;
}

int lua_RenderScale( void* p ){
	if( Lua_isstring(p,1) ){
		size_t len;
		const char* name = Lua_tolstring(p,1,&len);
		if( Lua_isnumber(p,2) ){
			float s = (float)Lua_tonumber(p,2);
			g_Render.Scale( name,s );
			Lua_pushnumber(p,1);
			return 1;
		}
	}
	Lua_pushnil( p );
	return 1;
}

int lua_RenderShow( void* p ){
	if( Lua_isstring(p,1) ){
		size_t len;
		const char* name = Lua_tolstring(p,1,&len);
		if( Lua_isnumber(p,2) ){
			float s = (float)Lua_tonumber(p,2);
			g_Render.Show( name,true );
			Lua_pushnumber(p,1);
			return 1;
		}else
			g_Render.Show( name,false );
	}
	Lua_pushnil( p );
	return 1;
}

int lua_RenderOverlay( void* p ){
	if( Lua_isstring(p,1) ){
		size_t len;
		const char* name = Lua_tolstring(p,1,&len);
		if( Lua_isnumber(p,2) ){
			float s = (float)Lua_tonumber(p,2);
			g_Render.Overlay( name,true );
			Lua_pushnumber(p,1);
			return 1;
		}else
			g_Render.Overlay( name,false );
	}
	Lua_pushnil( p );
	return 1;
}
