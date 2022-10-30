// ReSharper disable CppClangTidyPerformanceNoIntToPtr
// ReSharper disable CppInconsistentNaming
// ReSharper disable CppClangTidyBugproneNarrowingConversions
// ReSharper disable CppClangTidyClangDiagnosticFloatConversion
#include <Windows.h>
#include <cstdint>

#include "Scanner.h"
#include "LuaCoreWrapper.h"

constexpr uint32_t InvisibleFlagOffset = 0x128;
constexpr uint32_t NameHideFlagOffset = 0x130;
constexpr uint32_t TranslucentFlagOffset = 0x132;

const char* ActorByIdxPattern = "\x8B\x44\x24\x04\x3D\x01\x09\x00\x00\x73\x00\x8B\x04";
const char* ActorByIdxMask = "xxxxxxxxxx?xx";

using fnGetActorByIdx = uintptr_t(__cdecl*)(uint32_t param_1);
const fnGetActorByIdx oGetActorByIdx = reinterpret_cast<fnGetActorByIdx>(
	FindPattern::ScanModIn((char*)ActorByIdxPattern, (char*)ActorByIdxMask, "FFXiMain.dll"));

uintptr_t GetActorByIdx(uint32_t idx)
{
	if (!oGetActorByIdx)
		return 0;

	return oGetActorByIdx(idx);
}

static int SetEntityTranslucent(lua_State* L)
{
	const auto idx = LuaCoreWrapper::oLuaL_CheckInteger(L, 1);

	if(const auto actor = GetActorByIdx(idx))
	{
		byte translucentFlag = *reinterpret_cast<byte*>(actor + TranslucentFlagOffset);
		translucentFlag |= 1 << 2;
		*reinterpret_cast<byte*>(actor + TranslucentFlagOffset) = translucentFlag;
		LuaCoreWrapper::oLua_PushNumber(L, 1);
		return 1;
	}
	
	LuaCoreWrapper::oLua_PushNumber(L, -1);
	return 1;
}

static int SetEntityOpaque(lua_State* L)
{
	const auto idx = LuaCoreWrapper::oLuaL_CheckInteger(L, 1);

	if (const auto actor = GetActorByIdx(idx))
	{
		auto translucentFlag = *reinterpret_cast<byte*>(actor + TranslucentFlagOffset);
		translucentFlag &= ~(1 << 2);
		*reinterpret_cast<byte*>(actor + TranslucentFlagOffset) = translucentFlag;
		LuaCoreWrapper::oLua_PushNumber(L, 1);
		return 1;
	}
	
	LuaCoreWrapper::oLua_PushNumber(L, -1);
	return 1;
}

static int IsEntityInvisible(lua_State* L)
{
	const auto idx = LuaCoreWrapper::oLuaL_CheckInteger(L, 1);

	if (const auto actor = GetActorByIdx(idx))
	{
		const auto invisFlag = *reinterpret_cast<byte*>(actor + InvisibleFlagOffset);
		LuaCoreWrapper::oLua_PushBoolean(L, (invisFlag >> 6) > 0 || (invisFlag >> 7) > 0);
		return 1;
	}
	
	return -1;
}

static int RemoveEntityInvisible(lua_State* L)
{
	const auto idx = LuaCoreWrapper::oLuaL_CheckInteger(L, 1);

	if (const auto actor = GetActorByIdx(idx))
	{
		auto invisibleFlag = *reinterpret_cast<byte*>(actor + InvisibleFlagOffset);
		invisibleFlag &= ~(1 << 6);
		//invisibleFlag &= ~(1 << 7);
		*reinterpret_cast<byte*>(actor + InvisibleFlagOffset) = invisibleFlag;
		LuaCoreWrapper::oLua_PushNumber(L, 1);
		return 1;
	}
	
	LuaCoreWrapper::oLua_PushNumber(L, -1);
	return 1;
}

static int SetEntityInvisible(lua_State* L)
{
	const auto idx = LuaCoreWrapper::oLuaL_CheckInteger(L, 1);

	if(const auto actor = GetActorByIdx(idx))
	{
		auto invisibleFlag = *reinterpret_cast<byte*>(actor + InvisibleFlagOffset);
		invisibleFlag |= 1 << 6;
		//invisibleFlag |= 1 << 7; game sets both but setting 7th bit sets 8th one and unsetting 7th bit unsets 8th one so don't really need to set/unset both
		*reinterpret_cast<byte*>(actor + InvisibleFlagOffset) = invisibleFlag;
		LuaCoreWrapper::oLua_PushNumber(L, 1);
		return 1;
	}
	
	LuaCoreWrapper::oLua_PushNumber(L, -1);
	return 1;
}

static int IsNameHidden(lua_State* L)
{
	const auto idx = LuaCoreWrapper::oLuaL_CheckInteger(L, 1);

	if (const auto actor = GetActorByIdx(idx))
	{
		const auto nameHideFlag = *reinterpret_cast<byte*>(actor + NameHideFlagOffset);
		LuaCoreWrapper::oLua_PushBoolean(L, ((nameHideFlag >> 7) & 1) > 0);
		return 1;
	}
	
	return 1;
}

static int HideEntityName(lua_State* L)
{
	const auto idx = LuaCoreWrapper::oLuaL_CheckInteger(L, 1);

	if (const auto actor = GetActorByIdx(idx))
	{
		auto nameHideFlag = *reinterpret_cast<byte*>(actor + NameHideFlagOffset);
		nameHideFlag |= 1 << 7;
		*reinterpret_cast<byte*>(actor + NameHideFlagOffset) = nameHideFlag;
		LuaCoreWrapper::oLua_PushNumber(L, 1);
		return 1;
	}
	
	LuaCoreWrapper::oLua_PushNumber(L, -1);
	return 1;
}

static int ShowEntityName(lua_State* L)
{
	const auto idx = LuaCoreWrapper::oLuaL_CheckInteger(L, 1);

	if(const auto actor = GetActorByIdx(idx))
	{
		auto nameHideFlag = *reinterpret_cast<byte*>(actor + NameHideFlagOffset);
		nameHideFlag &= ~(1 << 7);
		*reinterpret_cast<byte*>(actor + NameHideFlagOffset) = nameHideFlag;
		LuaCoreWrapper::oLua_PushNumber(L, 1);
		return 1;
	}
	
	LuaCoreWrapper::oLua_PushNumber(L, -1);
	return 1;
}

extern "C" int __declspec(dllexport) luaopen_EntityFlagChanger(lua_State * L)
{
	constexpr struct luaL_Reg funcs[] = {
		{ "SetEntityTranslucent", SetEntityTranslucent},
		{ "SetEntityOpaque", SetEntityOpaque},
		{ "IsNameHidden", IsNameHidden},
		{ "HideEntityName", HideEntityName},
		{ "ShowEntityName", ShowEntityName},
		{ "IsEntityInvisible", IsEntityInvisible},//Checks purely for the invisible buff flags, probably not the only way to hide entities
		{ "SetEntityInvisible", SetEntityInvisible},
		{ "RemoveEntityInvisible", RemoveEntityInvisible},
		{ NULL, NULL }
	};
	LuaCoreWrapper::oLuaL_Register(L, "EntityFlagChanger", funcs);
	return 1;
}