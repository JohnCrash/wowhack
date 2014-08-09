#include "stdafx.h"
#include "callhook.h"
#include "nethook.h"
#include "wowentry.h"
#include "dllentry.h"
#include "Winsock2.h"

extern const char* sWow;
const char* sWS2 = "WS2_32";
const char* sWS2DLL = "WS2_32.dll";

typedef int (WSAAPI *t_send)(SOCKET s,const char* buf,int len,int flags);
typedef int (WSAAPI *t_recv)(SOCKET s,char* buf,int len,int flags);

PROC pfnSend;
PROC pfnRecv;

int WSAAPI mypfnSend(SOCKET s,const char* buf,int len,int flags){
	int ret = ((t_send)pfnSend)(s,buf,len,flags);
	printf("send:%d{",len);
	for( int i = 0;i < len;i++ ){
		printf("%02X,",(unsigned char)buf[i]);
	}
	printf("}\n");
	return ret;
}

int WSAAPI mypfnRecv(SOCKET s,char* buf,int len,int flags){
	int ret = ((t_send)pfnRecv)(s,buf,len,flags);
	printf("recv:%d{",len);
	for( int i = 0;i < len;i++ ){
		printf("%02X,",(unsigned char)buf[i]);
	}
	printf("}\n");
	return ret;
}

void HookNetwork(){
	/*
	HMODULE hmodCaller = GetModuleHandle(sWow);
	
	pfnSend = GetProcAddress(GetModuleHandle(sWS2),"send");
	if( pfnSend )
		RelpaceEntry(sWS2DLL,pfnSend,(PROC)mypfnSend,hmodCaller);
	else
		printf("Can't find send function in %s\n",sWS2DLL );
	*/
	/*
	pfnRecv = GetProcAddress(GetModuleHandle(sWS2),"recv");
	if( pfnRecv )
		RelpaceEntry(sWS2DLL,pfnRecv,(PROC)mypfnRecv,hmodCaller);
	else
		printf("Can't find recv function in %s\n",sWS2DLL );
	*/
}
