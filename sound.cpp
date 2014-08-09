#include "stdafx.h"
#include <process.h>
#include "wave.h"
#include "wowin\soundnote.h"

const char* myWndClassName = "wowSoundClass";
const char* myWndName = "wowsound";

DWORD WINAPI playProc(LPVOID lpParameter){
	char* fname = (char*)lpParameter;
	if( fname ){
		CWaveDevice waveDevice;
		CWaveOut waveOut;
		CWave waveFile;

		if( waveFile.Load(fname) ){
			waveOut.SetDevice(waveDevice);
			waveOut.SetWave(waveFile);
			if( waveOut.Open() ){
				waveOut.Play();
				while(1){
					Sleep(50);
					if( !waveOut.IsPlaying() )
						break;
				}
				waveOut.Close();
			}
		}
		delete [] fname;
	}
	return 0;
}

void Play( char* fname ){
	DWORD id;
	char* p;
	if( fname ){
		p = new char[255];
		sprintf(p,"H:\\Source\\wowhack\\wave\\%s",fname);
		_strlwr(p);
		int len = strlen(p);
		if( p[len-1]!='v' || p[len-2]!='a' || p[len-3]!='w' ){
			strcat(p,".wav");
		}
	}
	CreateThread( NULL,0,playProc,p,0,&id );
}

void Play2( char* fn1,char* fn2 ){
}

void Play3( char* fn1,char* fn2 ){
}

void attackNote(int t){
	char* fname;
	switch(t){
		case 1:fname="at_druid.wav";break;
		case 2:fname="at_priest.wav";break;
		case 3:fname="at_ss.wav";break;
		case 4:fname="at_sm.wav";break;
		case 5:fname="at_rouge.wav";break;
		case 6:fname="at_mage.wav";break;
		case 7:fname="at_hunter.wav";break;
		case 8:fname="at_paladin.wav";break;
		case 9:fname="at_dk.wav";break;
		case 10:fname="at_warrior.wav";break;
		default:fname="at_nobody.wav";break;
	}
	Play( fname );
}

void defanceNote(int t){
	char* fname;
	switch(t){
		case 1:fname="df_me.wav";break;
		case 2:fname="df_wind.wav";break;
		case 3:fname="df_hy.wav";break;
		case 4:fname="df_ison.wav";break;
		case 5:fname="df_sim.wav";break;
		default:return;
	}
	Play( fname );
}

void yunNote(int t,int z){
	char fname[255];
	char* target;
	switch(t){
		case 1:
			target="yun_me";
			break;
		case 2:
			target="yun_wind";
			break;
		case 3:
			target="yun_hy";
			break;
		case 4:
			target="yun_ison";
			break;
		case 5:
			target="yun_sim";
			break;
		default:return;
	}
	sprintf(fname,"%s%d.wav",target,z);
	Play( fname );
}

void targetHeal( int t ){
	if( t==60 )
		Play("60.wav");
	else if(t==30)
		Play("30.wav");
	else if(t==10)
		Play("10.wav");
}

void zhiyouNote(){
	Play( "zhiyou.wav" );
}

void notTarget(){
	Play( "notarget.wav" );
}

static int len = 0;
static char sPSound[64];

void luaPlaySound(UINT p0,UINT p1){
	*((UINT*)(sPSound+len)) = p0;
	*((UINT*)(sPSound+len+4)) = p1;
	for( int i = 0;i<8;i++ ){
		if( sPSound[i+len]==0 ){
			len=0;
			Play( sPSound );
			return;
		}
	}
	len+=8;
	if( len>=64 )
		len=0;
}

LRESULT CALLBACK mySoundProc( HWND hwnd, 
  UINT uMsg, 
  WPARAM wParam, 
  LPARAM lParam 
  ){
	  switch(uMsg){
		  case CMD_ATTACK://集火目标
			  attackNote(wParam);
			  break;
		  case CMD_DEFANCE: //我们的目标被集火
			  defanceNote(wParam);
			  break;
		  case CMD_YUN: //解控制
			  yunNote(wParam,lParam);
			  break;
		  case CMD_ZHIYOU: //给我自由
			  zhiyouNote();
			  break;
		  case CMD_TARGET_HEAL:
			   targetHeal(wParam);
			   break;
		  case CMD_NOTARGET:
			  notTarget();
			  break;
		  case CMD_STRING:
			  luaPlaySound( (UINT)wParam,(UINT)lParam );
			  break;
		  case WM_DESTROY:
				PostQuitMessage(0);
				break;
		  default:
			  return DefWindowProc(hwnd,uMsg,wParam,lParam);
	  }
	  return 0;
}

void waitForQuit( void * p ){
	HANDLE he = (HANDLE)p;
	WaitForSingleObject( he,INFINITE );
	ExitProcess(0);
}

//先创建个窗口用来接受语音消息
bool InitSoundNote( HINSTANCE hinst,HANDLE he ){
	WNDCLASS myClass;

	memset(&myClass,0,sizeof(WNDCLASS));
	myClass.style = CS_VREDRAW;
	myClass.lpfnWndProc = mySoundProc;
	myClass.hInstance = hinst;
	myClass.lpszClassName = myWndClassName;

	if( RegisterClass( &myClass ) ){
		HWND hwnd = CreateWindow(myWndClassName,myWndName,
				WS_OVERLAPPEDWINDOW,
				0,0,100,100,NULL,NULL,hinst,NULL );
		MSG msg;
		if( he )
			_beginthread(waitForQuit,0,he);
		while( GetMessage(&msg,NULL,0,0) ){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return true;
	}
	return false;
}