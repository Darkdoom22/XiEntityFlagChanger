#include <windows.h>
#include <cstdio>
#include <cstdint>

#include <lua.hpp>

#include "Scanner.h"

const char* ActorByIdxPattern = "\x8B\x44\x24\x04\x3D\x01\x09\x00\x00\x73\x00\x8B\x04";
const char* ActorByIdxMask = "xxxxxxxxxx?xx";

const HMODULE LuaCore = GetModuleHandle(L"LuaCore.dll");

using fnLuaL_Register = void(_cdecl*)(lua_State*, const char* libName, const luaL_Reg*);
const fnLuaL_Register oLuaL_Register = (fnLuaL_Register)GetProcAddress(LuaCore, "luaL_register");

using fnLua_PushNumber = void(_cdecl*)(lua_State*, lua_Number);
const fnLua_PushNumber oLua_PushNumber = (fnLua_PushNumber)GetProcAddress(LuaCore, "lua_pushnumber");

using fnLuaL_PushBoolean = void(_cdecl*)(lua_State*, int);
const fnLuaL_PushBoolean oLua_PushBoolean = (fnLuaL_PushBoolean)GetProcAddress(LuaCore, "lua_pushboolean");

using fnLuaL_CheckInteger = lua_Number(_cdecl*)(lua_State*, int);
const fnLuaL_CheckInteger oLuaL_CheckInteger = (fnLuaL_CheckInteger)GetProcAddress(LuaCore, "luaL_checknumber");

using fnGetActorByIdx = uintptr_t(_cdecl*)(uint32_t param_1);
const fnGetActorByIdx oGetActorByIdx = (fnGetActorByIdx)FindPattern::ScanModIn((char*)ActorByIdxPattern, (char*)ActorByIdxMask, "FFXiMain.dll");

static int SetEntityTranslucent(lua_State* L)
{
	auto idx = oLuaL_CheckInteger(L, 1);
	if(oGetActorByIdx)
	{
		if(auto actor = oGetActorByIdx(idx))
		{
			byte translucentFlag = *(byte*)(actor + 0x132);
			translucentFlag |= 1 << 2;
			*(byte*)(actor + 0x132) = translucentFlag;
			oLua_PushNumber(L, 1);
			return 1;
		}
	}
	oLua_PushNumber(L, -1);
	return 1;
}

static int SetEntityOpaque(lua_State* L)
{
	auto idx = oLuaL_CheckInteger(L, 1);
	if (oGetActorByIdx)
	{
		if (auto actor = oGetActorByIdx(idx))
		{
			byte translucentFlag = *(byte*)(actor + 0x132);
			translucentFlag &= ~(1 << 2);
			*(byte*)(actor + 0x132) = translucentFlag;
			oLua_PushNumber(L, 1);
			return 1;
		}
	}
	oLua_PushNumber(L, -1);
	return 1;
}

static int IsNameHidden(lua_State* L)
{
	auto idx = oLuaL_CheckInteger(L, 1);
	if(oGetActorByIdx)
	{
		if (auto actor = oGetActorByIdx(idx))
		{
			auto nameHideFlag = *(byte*)(actor + 0x130);
			oLua_PushBoolean(L, ((nameHideFlag >> 7) & 1) > 0);
			return 1;
		}
	}
	return 1;
}

static int HideEntityName(lua_State* L)
{
	auto idx = oLuaL_CheckInteger(L, 1);
	if (oGetActorByIdx)
	{
		if (auto actor = oGetActorByIdx(idx))
		{
			auto nameHideFlag = *(byte*)(actor + 0x130);
			nameHideFlag |= 1 << 7;
			*(byte*)(actor + 0x130) = nameHideFlag;
			oLua_PushNumber(L, 1);
			return 1;
		}
	}
	oLua_PushNumber(L, -1);
	return 1;
}

static int ShowEntityName(lua_State* L)
{
	auto idx = oLuaL_CheckInteger(L, 1);
	if(oGetActorByIdx)
	{
		if(auto actor = oGetActorByIdx(idx))
		{
			auto nameHideFlag = *(byte*)(actor + 0x130);
			nameHideFlag &= ~(1 << 7);
			*(byte*)(actor + 0x130) = nameHideFlag;
			oLua_PushNumber(L, 1);
		}
	}
	oLua_PushNumber(L, -1);
	return 1;
}

extern "C" int __declspec(dllexport) luaopen_EntityFlagChanger(lua_State * L)
{
	struct luaL_Reg funcs[] = {
		{"SetEntityTranslucent", SetEntityTranslucent},
		{ "HideEntityName", HideEntityName},
		{"ShowEntityName", ShowEntityName},
		{ "IsNameHidden", IsNameHidden},
		{"SetEntityOpaque", SetEntityOpaque},
		{ NULL, NULL }
	};
	oLuaL_Register(L, "EntityFlagChanger", funcs);
	return 1;
}