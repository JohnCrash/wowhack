#pragma once

typedef void (*t_Think)();
typedef void (*t_ChatEvent)(const char* sender,const char* msg,const char* channel);
typedef int (*t_Init)( AI* );
typedef void (*t_Release)();

void PrintCooldown();
void PrintSkill();