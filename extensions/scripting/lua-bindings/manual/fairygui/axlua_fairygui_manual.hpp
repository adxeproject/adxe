#ifndef SCRIPTING_AXLUA_FAIRYGUI_MANUAL_H__
#define SCRIPTING_AXLUA_FAIRYGUI_MANUAL_H__
#if defined(AX_ENABLE_EXT_FAIRYGUI)

#include "tolua++.h"

TOLUA_API int register_fairygui_module(lua_State* L);

#endif // defined(AX_ENABLE_EXT_FAIRYGUI)

#endif
