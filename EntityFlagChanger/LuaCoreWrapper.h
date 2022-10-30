// ReSharper disable CppClangTidyClangDiagnosticCastFunctionType
// ReSharper disable CppInconsistentNaming

#pragma once
#include <Windows.h>
#include <lua.hpp>

namespace LuaCoreWrapper
{
	const HMODULE LuaCore = GetModuleHandleW(L"LuaCore.dll");

	using fnLuaL_Register = void(_cdecl*)(lua_State*, const char* libName, const luaL_Reg*);
	const fnLuaL_Register oLuaL_Register = reinterpret_cast<fnLuaL_Register>(
		GetProcAddress(LuaCore, "luaL_register"));

	using fnLua_PushNumber = void(_cdecl*)(lua_State*, lua_Number);
	const fnLua_PushNumber oLua_PushNumber = reinterpret_cast<fnLua_PushNumber>(
		GetProcAddress(LuaCore, "lua_pushnumber"));

	using fnLuaL_PushBoolean = void(_cdecl*)(lua_State*, int);
	const fnLuaL_PushBoolean oLua_PushBoolean = reinterpret_cast<fnLuaL_PushBoolean>(
		GetProcAddress(LuaCore, "lua_pushboolean"));

	using fnLuaL_CheckInteger = lua_Integer(_cdecl*)(lua_State*, int);
	const fnLuaL_CheckInteger oLuaL_CheckInteger = reinterpret_cast<fnLuaL_CheckInteger>(
		GetProcAddress(LuaCore, "luaL_checkinteger"));
}