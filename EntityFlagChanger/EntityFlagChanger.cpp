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
		if(const auto actor = oGetActorByIdx(idx))
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
		if (const auto actor = oGetActorByIdx(idx))
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

static int IsEntityInvisible(lua_State* L)
{
	auto idx = oLuaL_CheckInteger(L, 1);
	if (oGetActorByIdx)
	{
		if (const auto actor = oGetActorByIdx(idx))
		{
			byte invisFlag = *(byte*)(actor + 0x128);
			oLua_PushBoolean(L, (invisFlag >> 6) > 0 || (invisFlag >> 7) > 0);
			return 1;
		}
	}
	return -1;
}

static int SetEntityInvisible(lua_State* L)
{
	auto idx = oLuaL_CheckInteger(L, 1);
	if(oGetActorByIdx)
	{
		if(const auto actor = oGetActorByIdx(idx))
		{
			byte invisibleFlag = *(byte*)(actor + 0x128);
			invisibleFlag |= 1 << 6;
			//invisibleFlag |= 1 << 7; game sets both but setting 7th bit sets 8th one and unsetting 7th bit unsets 8th one so don't really need to set/unset both
			*(byte*)(actor + 0x128) = invisibleFlag;
			oLua_PushNumber(L, 1);
			return 1;
		}
	}
	oLua_PushNumber(L, -1);
	return 1;
}

static int RemoveEntityInvisible(lua_State* L)
{
	auto idx = oLuaL_CheckInteger(L, 1);
	if(oGetActorByIdx)
	{
		if(const auto actor = oGetActorByIdx(idx))
		{
			byte invisibleFlag = *(byte*)(actor + 0x128);
			invisibleFlag &= ~(1 << 6);
			//invisibleFlag &= ~(1 << 7);
			*(byte*)(actor + 0x128) = invisibleFlag;
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
		if (const auto actor = oGetActorByIdx(idx))
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
		if (const auto actor = oGetActorByIdx(idx))
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
		if(const auto actor = oGetActorByIdx(idx))
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
	constexpr struct luaL_Reg funcs[] = {
		{ "SetEntityTranslucent", SetEntityTranslucent},
		{ "HideEntityName", HideEntityName},
		{ "ShowEntityName", ShowEntityName},
		{ "IsNameHidden", IsNameHidden},
		{ "SetEntityOpaque", SetEntityOpaque},
		{ "IsEntityInvisible", IsEntityInvisible},
		{ "SetEntityInvisible", SetEntityInvisible},
		{ "RemoveEntityInvisible", RemoveEntityInvisible},
		{ NULL, NULL }
	};
	oLuaL_Register(L, "EntityFlagChanger", funcs);
	return 1;
}