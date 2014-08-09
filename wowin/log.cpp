#include "stdafx.h"
#include "log.h"
#include "callhook.h"
#include "wowentry.h"
#include "dllentry.h"
#include "wowin.h"
#include <time.h>

HANDLE  log_fp=INVALID_HANDLE_VALUE;

//简单的写入一个日志文件
void Log( const char* str ){
	FILE* fp;
	char buf[1024];
	char databuf[128];
	char timebuf[128];
	time_t ltime;

	fp = fopen( "wowinlog.txt","a+" );
	if( fp ){
		time( &ltime );
		_strdate( databuf );
		_strtime( timebuf );
		sprintf( buf,"[%s %s] %s\n",databuf,timebuf,str );
		fwrite( buf,1,strlen(buf),fp );
		fclose( fp );
	}
}

bool Log_open( const char* name ){
	char fname[255];
	if( name ){
		strcpy(fname,"d:\\wow\\log\\");
		strcat(fname,ansi(std::string(name)).c_str());
		strcat(fname,".log");
		if( log_fp!=INVALID_HANDLE_VALUE )
			Log_close();
		log_fp = CreateFile(fname,GENERIC_WRITE,0,NULL,
						CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if( log_fp!=INVALID_HANDLE_VALUE)
			return true;
	}
	return false;
}

bool Log_line( const char* str ){
	if( log_fp!=INVALID_HANDLE_VALUE && str ){
		DWORD len;
		WriteFile( log_fp,str,strlen(str),&len,NULL);
		WriteFile( log_fp,"\r\n",strlen("\r\n"),&len,NULL);
		return true;
	}
	return false;
}

void Log_close(){
	if( log_fp ){
		CloseHandle(log_fp);
		log_fp=INVALID_HANDLE_VALUE;
	}
}

int lua_LogOpen( void* p){
	if( Lua_isstring(p,1) ){
		size_t len;
		const char* name = Lua_tolstring(p,1,&len);
		if( name ){
			if( Log_open( name ) )
				Lua_pushnumber(p,1);
			else
				Lua_pushnil( p );
			return 1;
		}
	}
	Lua_pushnil( p );
	return 1;
}

int lua_LogLine( void* p){
	if( Lua_isstring(p,1) ){
		size_t len;
		const char* buf = Lua_tolstring(p,1,&len);
		if( buf ){
			if( Log_line( buf ) )
				Lua_pushnumber(p,1);
			else
				Lua_pushnil( p );
			return 1;
		}
	}
	Lua_pushnil( p );
	return 1;
}

int lua_LogClose( void* p){
	Log_close();
	Lua_pushnil( p );
	return 1;
}