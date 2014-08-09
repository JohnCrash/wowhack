#include "stdafx.h"
#include "callhook.h"
#include "wowentry.h"
#include "wowin.h"
#include "waypoint.h"
#include "collide.h"
#include "render.h"
#include "movement.h"
#include "scout.h"
#include "camera.h"
#include "ai.h"
#include "bot.h"
#include "player.h"
#include <strstream>
#include "hunter.h"

Hunter::Hunter( AI* ai ):Player(ai){
}

Hunter::~Hunter(){
}