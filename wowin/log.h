#pragma once

extern void Log( const char* str );

extern bool Log_open( const char* name );
extern bool Log_line( const char* buf );
extern void Log_close();

extern int lua_LogOpen( void* p);
extern int lua_LogLine( void* p);
extern int lua_LogClose( void* p);