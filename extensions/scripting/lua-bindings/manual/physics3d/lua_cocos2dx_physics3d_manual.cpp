/****************************************************************************
 Copyright (c) 2014-2016 Chukong Technologies Inc.
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 https://axis-project.github.io/

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#include "platform/CCPlatformConfig.h"
#include "base/ccConfig.h"
#if CC_USE_3D_PHYSICS && CC_ENABLE_BULLET_INTEGRATION
#    include "scripting/lua-bindings/manual/physics3d/lua_cocos2dx_physics3d_manual.h"
#    include "scripting/lua-bindings/auto/lua_cocos2dx_physics3d_auto.hpp"
#    include "scripting/lua-bindings/manual/tolua_fix.h"
#    include "scripting/lua-bindings/manual/LuaBasicConversions.h"
#    include "scripting/lua-bindings/manual/CCLuaEngine.h"
#    include "physics3d/CCPhysics3D.h"

bool luaval_to_Physics3DRigidBodyDes(lua_State* L,
                                     int lo,
                                     axis::Physics3DRigidBodyDes* outValue,
                                     const char* funcName)
{
    if (nullptr == L || nullptr == outValue)
        return false;

    bool ok = true;

    tolua_Error tolua_err;
    if (!tolua_istable(L, lo, 0, &tolua_err))
    {
#    if COCOS2D_DEBUG >= 1
        luaval_to_native_err(L, "#ferror:", &tolua_err, funcName);
#    endif
        ok = false;
    }

    if (ok)
    {
        lua_pushstring(L, "mass");
        lua_gettable(L, lo);
        outValue->mass = lua_isnil(L, -1) ? 0.0f : (float)lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_pushstring(L, "localInertia");
        lua_gettable(L, lo);
        if (!lua_istable(L, -1))
        {
            outValue->localInertia = axis::Vec3(0.0, 0.0, 0.0);
        }
        else
        {
            luaval_to_vec3(L, lua_gettop(L), &outValue->localInertia);
        }
        lua_pop(L, 1);

        lua_pushstring(L, "shape");
        lua_gettable(L, lo);
        if (!tolua_isusertype(L, -1, "cc.Physics3DShape", 0, &tolua_err))
        {
            outValue->shape = nullptr;
        }
        else
        {
            outValue->shape = static_cast<axis::Physics3DShape*>(tolua_tousertype(L, lua_gettop(L), nullptr));
        }
        lua_pop(L, 1);

        lua_pushstring(L, "originalTransform");
        lua_gettable(L, lo);
        if (!lua_istable(L, -1))
        {
            outValue->originalTransform = axis::Mat4();
        }
        else
        {
            luaval_to_mat4(L, lua_gettop(L), &outValue->originalTransform);
        }
        lua_pop(L, 1);

        lua_pushstring(L, "disableSleep");
        lua_gettable(L, lo);
        outValue->disableSleep = lua_isnil(L, -1) ? false : lua_toboolean(L, -1);
        lua_pop(L, 1);
    }
    return ok;
}

bool luaval_to_Physics3DWorld_HitResult(lua_State* L,
                                        int lo,
                                        axis::Physics3DWorld::HitResult* outValue,
                                        const char* funcName)
{
    if (nullptr == L || nullptr == outValue)
        return false;

    bool ok = true;

    tolua_Error tolua_err;
    if (!tolua_istable(L, lo, 0, &tolua_err))
    {
#    if COCOS2D_DEBUG >= 1
        luaval_to_native_err(L, "#ferror:", &tolua_err, funcName);
#    endif
        ok = false;
    }

    if (ok)
    {
        lua_pushstring(L, "hitPosition");
        lua_gettable(L, lo);
        if (!lua_istable(L, -1))
        {
            outValue->hitPosition = axis::Vec3();
        }
        else
        {
            luaval_to_vec3(L, lua_gettop(L), &(outValue->hitPosition));
        }
        lua_pop(L, 1);

        lua_pushstring(L, "hitNormal");
        lua_gettable(L, lo);
        if (!lua_istable(L, -1))
        {
            outValue->hitNormal = axis::Vec3();
        }
        else
        {
            luaval_to_vec3(L, lua_gettop(L), &(outValue->hitNormal));
        }
        lua_pop(L, 1);

        lua_pushstring(L, "hitObj");
        lua_gettable(L, lo);
        if (!tolua_isusertype(L, -1, "cc.Physics3DObject", 0, &tolua_err))
        {
            outValue->hitObj = nullptr;
        }
        else
        {
            outValue->hitObj = static_cast<axis::Physics3DObject*>(tolua_tousertype(L, lua_gettop(L), nullptr));
        }
        lua_pop(L, 1);
    }
    return true;
}

void Physics3DWorld_HitResult_to_luaval(lua_State* L, const axis::Physics3DWorld::HitResult& hitResult)
{
    if (nullptr == L)
        return;

    lua_newtable(L);
    lua_pushstring(L, "hitPosition");
    vec3_to_luaval(L, hitResult.hitPosition);
    lua_rawset(L, -3);
    lua_pushstring(L, "hitNormal");
    vec3_to_luaval(L, hitResult.hitNormal);
    lua_rawset(L, -3);
    lua_pushstring(L, "hitObj");
    if (nullptr == hitResult.hitObj)
    {
        lua_pushnil(L);
    }
    else
    {
        object_to_luaval<axis::Physics3DObject>(L, "cc.Physics3DObject", hitResult.hitObj);
    }
    lua_rawset(L, -3);
}

int lua_cocos2dx_physics3d_PhysicsMeshRenderer_create(lua_State* L)
{
    int argc = 0;
    bool ok  = true;

#    if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
#    endif

#    if COCOS2D_DEBUG >= 1
    if (!tolua_isusertable(L, 1, "cc.PhysicsMeshRenderer", 0, &tolua_err))
        goto tolua_lerror;
#    endif

    argc = lua_gettop(L) - 1;

    if (argc == 2)
    {
        std::string arg0;
        axis::Physics3DRigidBodyDes arg1;
        ok &= luaval_to_std_string(L, 2, &arg0, "cc.PhysicsMeshRenderer:create");
        ok &= luaval_to_Physics3DRigidBodyDes(L, 3, &arg1, "cc.PhysicsMeshRenderer:create");
        if (!ok)
        {
            tolua_error(L, "invalid arguments in function 'lua_cocos2dx_physics3d_PhysicsMeshRenderer_create'", nullptr);
            return 0;
        }
        axis::PhysicsMeshRenderer* ret = axis::PhysicsMeshRenderer::create(arg0, &arg1);
        object_to_luaval<axis::PhysicsMeshRenderer>(L, "cc.PhysicsMeshRenderer", (axis::PhysicsMeshRenderer*)ret);
        return 1;
    }
    if (argc == 3)
    {
        std::string arg0;
        axis::Physics3DRigidBodyDes arg1;
        axis::Vec3 arg2;
        ok &= luaval_to_std_string(L, 2, &arg0, "cc.PhysicsMeshRenderer:create");
        ok &= luaval_to_Physics3DRigidBodyDes(L, 3, &arg1, "cc.PhysicsMeshRenderer:create");
        ok &= luaval_to_vec3(L, 4, &arg2, "cc.PhysicsMeshRenderer:create");
        if (!ok)
        {
            tolua_error(L, "invalid arguments in function 'lua_cocos2dx_physics3d_PhysicsMeshRenderer_create'", nullptr);
            return 0;
        }
        axis::PhysicsMeshRenderer* ret = axis::PhysicsMeshRenderer::create(arg0, &arg1, arg2);
        object_to_luaval<axis::PhysicsMeshRenderer>(L, "cc.PhysicsMeshRenderer", (axis::PhysicsMeshRenderer*)ret);
        return 1;
    }
    if (argc == 4)
    {
        std::string arg0;
        axis::Physics3DRigidBodyDes arg1;
        axis::Vec3 arg2;
        axis::Quaternion arg3;
        ok &= luaval_to_std_string(L, 2, &arg0, "cc.PhysicsMeshRenderer:create");
        ok &= luaval_to_Physics3DRigidBodyDes(L, 3, &arg1, "cc.PhysicsMeshRenderer:create");
        ok &= luaval_to_vec3(L, 4, &arg2, "cc.PhysicsMeshRenderer:create");
        ok &= luaval_to_quaternion(L, 5, &arg3);
        if (!ok)
        {
            tolua_error(L, "invalid arguments in function 'lua_cocos2dx_physics3d_PhysicsMeshRenderer_create'", nullptr);
            return 0;
        }
        axis::PhysicsMeshRenderer* ret = axis::PhysicsMeshRenderer::create(arg0, &arg1, arg2, arg3);
        object_to_luaval<axis::PhysicsMeshRenderer>(L, "cc.PhysicsMeshRenderer", (axis::PhysicsMeshRenderer*)ret);
        return 1;
    }
    luaL_error(L, "%s has wrong number of arguments: %d, was expecting %d\n ", "cc.PhysicsMeshRenderer:create", argc, 2);
    return 0;
#    if COCOS2D_DEBUG >= 1
tolua_lerror:
    tolua_error(L, "#ferror in function 'lua_cocos2dx_physics3d_PhysicsMeshRenderer_create'.", &tolua_err);
#    endif
    return 0;
}

void extendPhysicsMeshRenderer(lua_State* L)
{
    lua_pushstring(L, "cc.PhysicsMeshRenderer");
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (lua_istable(L, -1))
    {
        tolua_function(L, "create", lua_cocos2dx_physics3d_PhysicsMeshRenderer_create);
    }
    lua_pop(L, 1);
}

int lua_cocos2dx_physics3d_Physics3DRigidBody_create(lua_State* L)
{
    int argc = 0;
    bool ok  = true;

#    if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
#    endif

#    if COCOS2D_DEBUG >= 1
    if (!tolua_isusertable(L, 1, "cc.Physics3DRigidBody", 0, &tolua_err))
        goto tolua_lerror;
#    endif

    argc = lua_gettop(L) - 1;

    if (argc == 1)
    {
        axis::Physics3DRigidBodyDes arg0;
        ok &= luaval_to_Physics3DRigidBodyDes(L, 2, &arg0, "cc.Physics3DRigidBody:create");
        if (!ok)
        {
            tolua_error(L, "invalid arguments in function 'lua_cocos2dx_physics3d_Physics3DRigidBody_create'", nullptr);
            return 0;
        }
        axis::Physics3DRigidBody* ret = axis::Physics3DRigidBody::create(&arg0);
        object_to_luaval<axis::Physics3DRigidBody>(L, "cc.Physics3DRigidBody", (axis::Physics3DRigidBody*)ret);
        return 1;
    }
    luaL_error(L, "%s has wrong number of arguments: %d, was expecting %d\n ", "cc.Physics3DRigidBody:create", argc, 1);
    return 0;
#    if COCOS2D_DEBUG >= 1
tolua_lerror:
    tolua_error(L, "#ferror in function 'lua_cocos2dx_physics3d_Physics3DRigidBody_create'.", &tolua_err);
#    endif
    return 0;
}

void extendPhysics3DRigidBody(lua_State* L)
{
    lua_pushstring(L, "cc.Physics3DRigidBody");
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (lua_istable(L, -1))
    {
        tolua_function(L, "create", lua_cocos2dx_physics3d_Physics3DRigidBody_create);
    }
    lua_pop(L, 1);
}

int lua_cocos2dx_physics3d_Physics3DComponent_create(lua_State* L)
{
    int argc = 0;
    bool ok  = true;
#    if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
#    endif

#    if COCOS2D_DEBUG >= 1
    if (!tolua_isusertable(L, 1, "cc.Physics3DComponent", 0, &tolua_err))
        goto tolua_lerror;
#    endif

    argc = lua_gettop(L) - 1;

    do
    {
        if (argc == 1)
        {
            axis::Physics3DObject* arg0;
            ok &= luaval_to_object<axis::Physics3DObject>(L, 2, "cc.Physics3DObject", &arg0);
            if (!ok)
            {
                break;
            }
            axis::Physics3DComponent* ret = axis::Physics3DComponent::create(arg0);
            object_to_luaval<axis::Physics3DComponent>(L, "cc.Physics3DComponent",
                                                          (axis::Physics3DComponent*)ret);
            return 1;
        }
    } while (0);
    ok = true;
    do
    {
        if (argc == 2)
        {
            axis::Physics3DObject* arg0;
            ok &= luaval_to_object<axis::Physics3DObject>(L, 2, "cc.Physics3DObject", &arg0);
            if (!ok)
            {
                break;
            }
            axis::Vec3 arg1;
            ok &= luaval_to_vec3(L, 3, &arg1, "cc.Physics3DComponent:create");
            if (!ok)
            {
                break;
            }
            axis::Physics3DComponent* ret = axis::Physics3DComponent::create(arg0, arg1);
            object_to_luaval<axis::Physics3DComponent>(L, "cc.Physics3DComponent",
                                                          (axis::Physics3DComponent*)ret);
            return 1;
        }
    } while (0);
    ok = true;
    do
    {
        if (argc == 3)
        {
            axis::Physics3DObject* arg0;
            ok &= luaval_to_object<axis::Physics3DObject>(L, 2, "cc.Physics3DObject", &arg0);
            if (!ok)
            {
                break;
            }
            axis::Vec3 arg1;
            ok &= luaval_to_vec3(L, 3, &arg1, "cc.Physics3DComponent:create");
            if (!ok)
            {
                break;
            }
            axis::Quaternion arg2;
            ok &= luaval_to_quaternion(L, 4, &arg2);
            if (!ok)
            {
                break;
            }
            axis::Physics3DComponent* ret = axis::Physics3DComponent::create(arg0, arg1, arg2);
            object_to_luaval<axis::Physics3DComponent>(L, "cc.Physics3DComponent",
                                                          (axis::Physics3DComponent*)ret);
            return 1;
        }
    } while (0);
    ok = true;
    do
    {
        if (argc == 0)
        {
            axis::Physics3DComponent* ret = axis::Physics3DComponent::create();
            object_to_luaval<axis::Physics3DComponent>(L, "cc.Physics3DComponent",
                                                          (axis::Physics3DComponent*)ret);
            return 1;
        }
    } while (0);
    ok = true;
    luaL_error(L, "%s has wrong number of arguments: %d, was expecting %d", "cc.Physics3DComponent:create", argc, 0);
    return 0;
#    if COCOS2D_DEBUG >= 1
tolua_lerror:
    tolua_error(L, "#ferror in function 'lua_cocos2dx_physics3d_Physics3DComponent_create'.", &tolua_err);
#    endif
    return 0;
}

void extendPhysics3DComponent(lua_State* L)
{
    lua_pushstring(L, "cc.Physics3DComponent");
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (lua_istable(L, -1))
    {
        tolua_function(L, "create", lua_cocos2dx_physics3d_Physics3DComponent_create);
    }
    lua_pop(L, 1);
}

int lua_cocos2dx_physics3d_Physics3DWorld_rayCast(lua_State* L)
{
    int argc                      = 0;
    axis::Physics3DWorld* cobj = nullptr;
    bool ok                       = true;

#    if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
#    endif

#    if COCOS2D_DEBUG >= 1
    if (!tolua_isusertype(L, 1, "cc.Physics3DWorld", 0, &tolua_err))
        goto tolua_lerror;
#    endif

    cobj = (axis::Physics3DWorld*)tolua_tousertype(L, 1, 0);

#    if COCOS2D_DEBUG >= 1
    if (!cobj)
    {
        tolua_error(L, "invalid 'cobj' in function 'lua_cocos2dx_physics3d_Physics3DWorld_rayCast'", nullptr);
        return 0;
    }
#    endif

    argc = lua_gettop(L) - 1;
    if (argc == 3)
    {
        axis::Vec3 arg0;
        axis::Vec3 arg1;
        axis::Physics3DWorld::HitResult arg2;

        ok &= luaval_to_vec3(L, 2, &arg0, "cc.Physics3DWorld:rayCast");

        ok &= luaval_to_vec3(L, 3, &arg1, "cc.Physics3DWorld:rayCast");

        ok &= luaval_to_Physics3DWorld_HitResult(L, 4, &arg2, "cc.Physics3DWorld:rayCast");

        if (!ok)
        {
            tolua_error(L, "invalid arguments in function 'lua_cocos2dx_physics3d_Physics3DWorld_rayCast'", nullptr);
            return 0;
        }
        bool ret = cobj->rayCast(arg0, arg1, &arg2);
        tolua_pushboolean(L, (bool)ret);
        Physics3DWorld_HitResult_to_luaval(L, arg2);
        return 2;
    }
    luaL_error(L, "%s has wrong number of arguments: %d, was expecting %d \n", "cc.Physics3DWorld:rayCast", argc, 3);
    return 0;

#    if COCOS2D_DEBUG >= 1
tolua_lerror:
    tolua_error(L, "#ferror in function 'lua_cocos2dx_physics3d_Physics3DWorld_rayCast'.", &tolua_err);
#    endif

    return 0;
}

void extendPhysics3DWorld(lua_State* L)
{
    lua_pushstring(L, "cc.Physics3DWorld");
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (lua_istable(L, -1))
    {
        tolua_function(L, "rayCast", lua_cocos2dx_physics3d_Physics3DWorld_rayCast);
    }
    lua_pop(L, 1);
}

int lua_cocos2dx_physics3d_Physics3DShape_createMesh(lua_State* L)
{
    int argc = 0;
    bool ok  = true;

#    if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
#    endif

#    if COCOS2D_DEBUG >= 1
    if (!tolua_isusertable(L, 1, "cc.Physics3DShape", 0, &tolua_err))
        goto tolua_lerror;
#    endif

    argc = lua_gettop(L) - 1;

    if (argc == 2)
    {
        std::vector<Vec3> arg0;
        int arg1;
        ok &= luaval_to_std_vector_vec3(L, 2, &arg0, "cc.Physics3DShape:createMesh");
        ok &= luaval_to_int32(L, 3, (int*)&arg1, "cc.Physics3DShape:createMesh");
        if (!ok)
        {
            tolua_error(L, "invalid arguments in function 'lua_cocos2dx_physics3d_Physics3DShape_createMesh'", nullptr);
            return 0;
        }
        axis::Physics3DShape* ret = axis::Physics3DShape::createMesh(&arg0[0], arg1);
        object_to_luaval<axis::Physics3DShape>(L, "cc.Physics3DShape", (axis::Physics3DShape*)ret);
        return 1;
    }
    luaL_error(L, "%s has wrong number of arguments: %d, was expecting %d\n ", "cc.Physics3DShape:createMesh", argc, 2);
    return 0;
#    if COCOS2D_DEBUG >= 1
tolua_lerror:
    tolua_error(L, "#ferror in function 'lua_cocos2dx_physics3d_Physics3DShape_createMesh'.", &tolua_err);
#    endif
    return 0;
}

int lua_cocos2dx_physics3d_Physics3DShape_createHeightfield(lua_State* L)
{
    int argc = 0;
    bool ok  = true;

#    if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
#    endif

#    if COCOS2D_DEBUG >= 1
    if (!tolua_isusertable(L, 1, "cc.Physics3DShape", 0, &tolua_err))
        goto tolua_lerror;
#    endif

    argc = lua_gettop(L) - 1;

    if (argc == 8)
    {
        int arg0;
        int arg1;
        std::vector<float> arg2;
        double arg3;
        double arg4;
        double arg5;
        bool arg6;
        bool arg7;
        ok &= luaval_to_int32(L, 2, (int*)&arg0, "cc.Physics3DShape:createHeightfield");
        ok &= luaval_to_int32(L, 3, (int*)&arg1, "cc.Physics3DShape:createHeightfield");
        ok &= luaval_to_std_vector_float(L, 4, &arg2, "cc.Physics3DShape:createHeightfield");
        ok &= luaval_to_number(L, 5, &arg3, "cc.Physics3DShape:createHeightfield");
        ok &= luaval_to_number(L, 6, &arg4, "cc.Physics3DShape:createHeightfield");
        ok &= luaval_to_number(L, 7, &arg5, "cc.Physics3DShape:createHeightfield");
        ok &= luaval_to_boolean(L, 8, &arg6, "cc.Physics3DShape:createHeightfield");
        ok &= luaval_to_boolean(L, 9, &arg7, "cc.Physics3DShape:createHeightfield");
        if (!ok)
        {
            tolua_error(L, "invalid arguments in function 'lua_cocos2dx_physics3d_Physics3DShape_createHeightfield'",
                        nullptr);
            return 0;
        }
        axis::Physics3DShape* ret = axis::Physics3DShape::createHeightfield(arg0, arg1, &arg2[0], (float)arg3,
                                                                                  (float)arg4, (float)arg5, arg6, arg7);
        object_to_luaval<axis::Physics3DShape>(L, "cc.Physics3DShape", (axis::Physics3DShape*)ret);
        return 1;
    }
    if (argc == 9)
    {
        int arg0;
        int arg1;
        std::vector<float> arg2;
        double arg3;
        double arg4;
        double arg5;
        bool arg6;
        bool arg7;
        bool arg8;
        ok &= luaval_to_int32(L, 2, (int*)&arg0, "cc.Physics3DShape:createHeightfield");
        ok &= luaval_to_int32(L, 3, (int*)&arg1, "cc.Physics3DShape:createHeightfield");
        ok &= luaval_to_std_vector_float(L, 4, &arg2, "cc.Physics3DShape:createHeightfield");
        ok &= luaval_to_number(L, 5, &arg3, "cc.Physics3DShape:createHeightfield");
        ok &= luaval_to_number(L, 6, &arg4, "cc.Physics3DShape:createHeightfield");
        ok &= luaval_to_number(L, 7, &arg5, "cc.Physics3DShape:createHeightfield");
        ok &= luaval_to_boolean(L, 8, &arg6, "cc.Physics3DShape:createHeightfield");
        ok &= luaval_to_boolean(L, 9, &arg7, "cc.Physics3DShape:createHeightfield");
        ok &= luaval_to_boolean(L, 10, &arg8, "cc.Physics3DShape:createHeightfield");
        if (!ok)
        {
            tolua_error(L, "invalid arguments in function 'lua_cocos2dx_physics3d_Physics3DShape_createHeightfield'",
                        nullptr);
            return 0;
        }
        axis::Physics3DShape* ret = axis::Physics3DShape::createHeightfield(
            arg0, arg1, &arg2[0], (float)arg3, (float)arg4, (float)arg5, arg6, arg7, arg8);
        object_to_luaval<axis::Physics3DShape>(L, "cc.Physics3DShape", (axis::Physics3DShape*)ret);
        return 1;
    }
    luaL_error(L, "%s has wrong number of arguments: %d, was expecting %d\n ", "cc.Physics3DShape:createHeightfield",
               argc, 8);
    return 0;
#    if COCOS2D_DEBUG >= 1
tolua_lerror:
    tolua_error(L, "#ferror in function 'lua_cocos2dx_physics3d_Physics3DShape_createHeightfield'.", &tolua_err);
#    endif
    return 0;
}

int lua_cocos2dx_physics3d_Physics3DShape_createCompoundShape(lua_State* L)
{
    int argc = 0;
    bool ok  = true;

    tolua_Error tolua_err;

#    if COCOS2D_DEBUG >= 1
    if (!tolua_isusertable(L, 1, "cc.Physics3DShape", 0, &tolua_err))
        goto tolua_lerror;
#    endif

    argc = lua_gettop(L) - 1;

    if (argc == 1)
    {

        std::vector<std::pair<axis::Physics3DShape*, axis::Mat4>> shapes;
        if (!tolua_istable(L, 2, 0, &tolua_err))
        {
#    if COCOS2D_DEBUG >= 1
            luaval_to_native_err(L, "#ferror:", &tolua_err, "cc.Physics3DShape:createCompoundShape");
#    endif
            ok = false;
        }

        if (ok)
        {
            size_t len                     = lua_objlen(L, 2);
            axis::Physics3DShape* shape = nullptr;
            axis::Mat4 mat;
            for (size_t i = 0; i < len; i++)
            {
                lua_pushnumber(L, i + 1);
                lua_gettable(L, 2);
                if (lua_istable(L, -1))
                {
                    lua_pushnumber(L, 1);
                    lua_gettable(L, -2);
                    luaval_to_object(L, lua_gettop(L), "cc.Physics3DShape", &shape);
                    lua_pop(L, 1);

                    lua_pushnumber(L, 2);
                    lua_gettable(L, -2);
                    luaval_to_mat4(L, lua_gettop(L), &mat);
                    lua_pop(L, 1);

                    shapes.push_back(std::make_pair(shape, mat));
                }
                lua_pop(L, 1);
            }
        }

        axis::Physics3DShape* ret = axis::Physics3DShape::createCompoundShape(shapes);
        object_to_luaval<axis::Physics3DShape>(L, "cc.Physics3DShape", (axis::Physics3DShape*)ret);
        return 1;
    }
    luaL_error(L, "%s has wrong number of arguments: %d, was expecting %d\n ", "cc.Physics3DShape:createCompoundShape",
               argc, 1);
    return 0;
#    if COCOS2D_DEBUG >= 1
tolua_lerror:
    tolua_error(L, "#ferror in function 'lua_cocos2dx_physics3d_Physics3DShape_createCompoundShape'.", &tolua_err);
#    endif
    return 0;
}

void extendPhysics3DShape(lua_State* L)
{
    lua_pushstring(L, "cc.Physics3DShape");
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (lua_istable(L, -1))
    {
        tolua_function(L, "createMesh", lua_cocos2dx_physics3d_Physics3DShape_createMesh);
        tolua_function(L, "createHeightfield", lua_cocos2dx_physics3d_Physics3DShape_createHeightfield);
        tolua_function(L, "createCompoundShape", lua_cocos2dx_physics3d_Physics3DShape_createCompoundShape);
    }
    lua_pop(L, 1);
}

void CollisionPoint_to_luaval(lua_State* L, const axis::Physics3DCollisionInfo::CollisionPoint& collisionPoint)
{
    if (nullptr == L)
        return;

    lua_newtable(L);
    lua_pushstring(L, "localPositionOnA");
    vec3_to_luaval(L, collisionPoint.localPositionOnA);
    lua_rawset(L, -3);
    lua_pushstring(L, "worldPositionOnA");
    vec3_to_luaval(L, collisionPoint.worldPositionOnA);
    lua_rawset(L, -3);
    lua_pushstring(L, "localPositionOnB");
    vec3_to_luaval(L, collisionPoint.localPositionOnB);
    lua_rawset(L, -3);
    lua_pushstring(L, "worldPositionOnB");
    vec3_to_luaval(L, collisionPoint.worldPositionOnB);
    lua_rawset(L, -3);
    lua_pushstring(L, "worldNormalOnB");
    vec3_to_luaval(L, collisionPoint.worldNormalOnB);
    lua_rawset(L, -3);
}

int lua_cocos2dx_physics3d_Physics3DObject_setCollisionCallback(lua_State* L)
{
    int argc                       = 0;
    axis::Physics3DObject* cobj = nullptr;

#    if COCOS2D_DEBUG >= 1
    tolua_Error tolua_err;
#    endif

#    if COCOS2D_DEBUG >= 1
    if (!tolua_isusertype(L, 1, "cc.Physics3DObject", 0, &tolua_err))
        goto tolua_lerror;
#    endif

    cobj = (axis::Physics3DObject*)tolua_tousertype(L, 1, 0);

#    if COCOS2D_DEBUG >= 1
    if (!cobj)
    {
        tolua_error(L, "invalid 'cobj' in function 'lua_cocos2dx_physics3d_Physics3DObject_setCollisionCallback'",
                    nullptr);
        return 0;
    }
#    endif

    argc = lua_gettop(L) - 1;
    if (argc == 1)
    {
#    if COCOS2D_DEBUG >= 1
        if (!toluafix_isfunction(L, 2, "LUA_FUNCTION", 0, &tolua_err))
        {
            goto tolua_lerror;
        }
#    endif
        LUA_FUNCTION handler = toluafix_ref_function(L, 2, 0);
        cobj->setCollisionCallback([=](const axis::Physics3DCollisionInfo& ci) {
            auto stack = LuaEngine::getInstance()->getLuaStack();
            auto Ls    = stack->getLuaState();
            lua_newtable(Ls);
            lua_pushstring(Ls, "objA");
            if (nullptr == ci.objA)
            {
                lua_pushnil(Ls);
            }
            else
            {
                object_to_luaval(Ls, "cc.Physics3DObject", ci.objA);
            }
            lua_rawset(Ls, -3);
            lua_pushstring(Ls, "objB");
            if (nullptr == ci.objB)
            {
                lua_pushnil(Ls);
            }
            else
            {
                object_to_luaval(Ls, "cc.Physics3DObject", ci.objB);
            }
            lua_rawset(Ls, -3);
            lua_pushstring(Ls, "collisionPointList");
            if (ci.collisionPointList.empty())
            {
                lua_pushnil(Ls);
            }
            else
            {
                int vecIndex = 1;
                lua_newtable(Ls);
                for (const auto& value : ci.collisionPointList)
                {
                    lua_pushnumber(Ls, vecIndex);
                    CollisionPoint_to_luaval(Ls, value);
                    lua_rawset(Ls, -3);
                    ++vecIndex;
                }
            }
            lua_rawset(Ls, -3);
            stack->executeFunctionByHandler(handler, 1);
        });

        ScriptHandlerMgr::getInstance()->addCustomHandler((void*)cobj, handler);
        return 0;
    }
    luaL_error(L, "%s has wrong number of arguments: %d, was expecting %d \n",
               "cc.Physics3DObject:setCollisionCallback", argc, 1);
    return 0;

#    if COCOS2D_DEBUG >= 1
tolua_lerror:
    tolua_error(L, "#ferror in function 'lua_cocos2dx_physics3d_Physics3DObject_setCollisionCallback'.", &tolua_err);
#    endif

    return 0;
}

void extendPhysics3DObject(lua_State* L)
{
    lua_pushstring(L, "cc.Physics3DObject");
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (lua_istable(L, -1))
    {
        tolua_function(L, "setCollisionCallback", lua_cocos2dx_physics3d_Physics3DObject_setCollisionCallback);
    }
    lua_pop(L, 1);
}

int register_all_physics3d_manual(lua_State* L)
{
    if (nullptr == L)
        return 0;

    extendPhysicsMeshRenderer(L);
    extendPhysics3DRigidBody(L);
    extendPhysics3DComponent(L);
    extendPhysics3DWorld(L);
    extendPhysics3DShape(L);
    extendPhysics3DObject(L);

    return 1;
}

int register_physics3d_module(lua_State* L)
{
    lua_getglobal(L, "_G");
    if (lua_istable(L, -1))  // stack:...,_G,
    {
        register_all_cocos2dx_physics3d(L);
        register_all_physics3d_manual(L);
    }
    lua_pop(L, 1);
    return 1;
}

#endif