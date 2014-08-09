#pragma once

//取得视角矢量
extern void DLLEXPORT GetCameraDirection( float* v );
//转动视角x水平角度，y垂直角度单位度
extern void DLLEXPORT TurnCamera( float x,float y );
//使得角色的方向和视角一致
extern void DLLEXPORT FaceCamera();

int lua_TurnCamera( void* p );