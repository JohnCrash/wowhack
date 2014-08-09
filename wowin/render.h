#pragma once

/* mode =	0 GL_LINES
			1 GL_LINE_STRIP
			2 GL_TRIANGLES
			3 GL_TRIANGLE_STRIP
			4 GL_TRIANGLE_FAN
*/
struct Grid{
	int mode;
	unsigned int first;
	unsigned int size;
	unsigned short start;
	unsigned short end;
};

typedef void (__stdcall *t_SwapBuff)();
typedef void (__stdcall *t_DrawScene)(Grid pgrid[],int size,int mode);
typedef void (__stdcall *t_SetMatrix)(float* m);
typedef void (__stdcall *t_ClearBuff)(unsigned long a,unsigned long b);


void  SetRenderHook();
void  ClearRenderHook();

void HookStandardApi();

//一个简单的渲染接口
struct DLLEXPORT RenderItem{
	std::string name;
	GLuint		list;//显示列表
	float ox;
	float oy;
	float oz;
	float rtx;
	float rty;
	float rtz;
	float rta;
	float s;
	bool  b;
	bool  o;
};

#define LP_MOVETO 0
#define LP_LINETO 1
#define LP_LINEWIDTH 2
#define LP_COLOR 3
#define LP_ARROW 4
#define LP_BOX 5
#define LP_NORMAL 6
#define LP_SIZE 7
#define LP_CIRCLE 8
#define LP_RECT 9
#define LP_TRIANGLE 10
#define LP_STIPPLE 11
#define LP_STRING 12

struct DLLEXPORT LinePoint{
	int   code;
	float x;
	float y;
	float z;
	std::string text;

	LinePoint();
	LinePoint(int c,float w);
	LinePoint(int c,float x,float y,float z);
	LinePoint(int c,float x,float y,float z,const char* txt);
};

struct DLLEXPORT Render{
	std::vector<RenderItem> vpi;
	std::vector<LinePoint>  vlp;
	RenderItem	current;
	GLuint	listChar[256]; //字符串
	GLuint	listArrow;
	GLuint	listBox;
	GLuint  listCircle;
	GLuint	listRect;
	GLuint	listTriangle;
	bool bLineStipple;
	float fWidth;
	Render();
	void Begin( std::string name ); //绘制
	void LineWidth( float w );
	void Color( float r,float g,float b );
	void MoveTo( float x,float y,float z );
	void LineTo( float x,float y,float z );
	void Normal( float x,float y,float z );
	void Size( float s );
	void Stipple( float s );
	void Box( float x,float y,float z );
	void Arrow( float x,float y,float z );
	void Circle( float x,float y,float z );
	void Rect( float x,float y,float z );
	void Triangle(float x,float y,float z );
	void End();
	void Clear( std::string name ); //清除
	void ClearAll(); 
	void Draw();
	bool IsEmpty();
	void glArrow(float ov[3],float on[3],float s);
	void glBox(float ov[3],float on[3],float s);
	void glCircle( float ov[3],float on[3],float s);
	void glString( float ov[3],float on[3],float s,std::string text);
	void glRect( float ov[3],float on[3],float s);
	void glTriangle( float ov[3],float on[3],float s);
	void BuildCircle();
	bool BuildArrow();
	void BuildBox();
	void BuildRect();
	void BuildTriangle();
	void BuildChar();
	void String(float x,float y,float z,const char* ptext);
	void BuildCharacter( int c,int n,float* g );
	void Translate(std::string name,float x,float y,float z);
	void Rotate(std::string name,float a,float x,float y,float z);
	void Scale(std::string name,float a);
	void Show(std::string name,bool b);
	void Overlay(std::string name,bool b);
};

extern Render g_Render;
extern void myDraw();
//template class DLLEXPORT std::vector<RenderItem>;
//template class DLLEXPORT std::vector<LinePoint>;

int lua_RenderBegin( void* p );
int lua_RenderEnd( void* p );
int lua_RenderMoveTo( void* p );
int lua_RenderLineTo( void* p );
int lua_RenderColor( void* p );
int lua_RenderLineWidth( void* p );
int lua_RenderCircle( void* p );
int lua_RenderRect( void* p );
int lua_RenderTriangle( void* p );
int lua_RenderBox( void* p );
int lua_RenderArrow( void* p );
int lua_RenderNormal( void* p );
int lua_RenderSize( void* p );
int lua_RenderStipple( void* p );
int lua_RenderClear( void* p );
int lua_RenderTranslate( void* p );
int lua_RenderRotate( void* p );
int lua_RenderScale( void* p );
int lua_RenderShow( void* p );
int lua_RenderOverlay( void* p );
int lua_RenderString( void*p);
int lua_ScreenPt(void* p);