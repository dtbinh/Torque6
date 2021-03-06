﻿//-----------------------------------------------------------------------------
// Copyright (c) 2015 Andrew Mac
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _PLUGINS_SHARED_H
#define _PLUGINS_SHARED_H

#ifndef _CONSOLE_H_
#include <console/console.h>
#endif

#ifndef _RENDERINGCOMMON_H_
#include <3d/rendering/common.h>
#endif

#ifndef _TEXTURE_OBJECT_H_
#include "graphics/TextureObject.h"
#endif

#ifndef _MATERIAL_ASSET_H_
#include "3d/material/materialAsset.h"
#endif

#ifndef _PROFILER_H_
#include "debug/profiler.h"
#endif

#ifndef NANOVG_H
#include <../common/nanovg/nanovg.h>
#endif

#ifndef _DEFERREDRENDERING_H_
#include "3d/rendering/deferredRendering.h"
#endif

// ----------------------------------------
//  Plugin Function Pointers
// ----------------------------------------
// A wrapper is created with function pointers for all the major
// engine functions. This file is shared between the engine and
// each plugin. PluginLink contains an instance of all the wrappers.
// There is a PluginLink variable in the Plugins namespace called Link.
// When the engine is initialized all the pointers in Link are defined.
// Link is defined as extern and implemented locally in the engine.
// When a plugin links against the DLL it will have access to Link and
// thus all the function pointers.
// A plugin can access engine functions with Plugins::Link.Function()

namespace Scene
{
   class SceneCamera;
}

namespace Plugins
{
   struct EngineWrapper
   {
#ifdef TORQUE_ENABLE_PROFILER
      Profiler* ProfilerLink;
#endif

      void (*mainLoop)();
      void (*resizeWindow)(int width, int height);
      void (*mouseMove)(int x, int y);
      void (*mouseButton)(bool down, bool left);
      void (*keyDown)(KeyCodes key);
      void (*keyUp)(KeyCodes key);
   };

   struct ConsoleWrapper
   {
      void (*printf)(const char *_format, ...);
      void (*warnf)(const char *_format, ...);
      void (*errorf)(const char *_format, ...);

      void (*addCommand)(const char *nsName, const char *name, VoidCallback cb, const char *usage, S32 minArgs, S32 maxArgs);

      const char* (*getData)(S32 type, void *dptr, S32 index, EnumTable *tbl, BitSet32 flag); // Defaults: *tbl = NULL, flag = 0
      Namespace* (*lookupNamespace)(const char *ns);
      bool (*classLinkNamespaces)(Namespace *parent, Namespace *child);
      void (*registerClassRep)(AbstractClassRep* in_pRep);

      S32 TypeF32;
      S32 TypeS8;
      S32 TypeS32;
      S32 TypeS32Vector;
      S32 TypeBool;
      S32 TypeBoolVector;
      S32 TypeF32Vector;
      S32 TypeString;
      S32 TypeStringTableEntryVector;
      S32 TypeCaseString;
      S32 TypeFilename;
      S32 TypeEnum;
      S32 TypeFlag;
      S32 TypeSimObjectPtr;
      S32 TypeSimObjectName;
      S32 TypeSimObjectId;
      S32 TypePoint3F;
   };

   struct SysGUIWrapper
   {
      S32 (*beginScrollArea)(const char* title, U32 x, U32 y, U32 width, U32 height);
      S32 (*endScrollArea)();
      S32 (*label)(const char* label);
      S32 (*list)(const char* script, void (*callback)(S32 id)); // Defaults: script = "", callback = NULL
      S32 (*checkBox)(const char* label, bool value);
      S32 (*slider)(const char* label, S32 value, S32 min, S32 max);
      S32 (*textInput)(const char* label, const char* text);
      S32 (*button)(const char* label, const char* script, void (*callback)(S32 id)); // Defaults: script = "", callback = NULL
      S32 (*separator)();
      S32 (*beginCollapse)(const char* label, const char* text, bool open);
      S32 (*endCollapse)();
      S32 (*colorWheel)(const char* label, ColorF color);
      S32 (*vector3)(const char* label, Point3F vec, const char* script, void (*callback)(S32 id));
      S32 (*image)(bgfx::TextureHandle*, const char* script, void (*callback)(S32 id));

      void (*addListValue)(S32 id, const char* val, const char* script, void (*callback)(S32 id)); // Defaults: script = "", callback = NULL
      const char* (*getListValue)(S32 id, S32 index);
      S32 (*getListSelected)(S32 id);
      void (*clearList)(S32 id);

      void                 (*setElementHidden)(S32 id, bool val);
      char*                (*getLabelValue)(S32 id);
      void                 (*setLabelValue)(S32 id, const char* val);
      char*                (*getTextValue)(S32 id);
      void                 (*setTextValue)(S32 id, const char* val);
      S32                  (*getIntValue)(S32 id);
      void                 (*setIntValue)(S32 id, S32 val);
      bool                 (*getBoolValue)(S32 id);
      void                 (*setBoolValue)(S32 id, bool val);
      ColorF               (*getColorValue)(S32 id);
      void                 (*setColorValue)(S32 id, ColorF val);
      Point3F              (*getVector3Value)(S32 id);
      void                 (*setVector3Value)(S32 id, Point3F val);
      bgfx::TextureHandle  (*getImageValue)(S32 id);
      void                 (*setImageValue)(S32 id, bgfx::TextureHandle val);

      void  (*alignLeft)(S32 id);
      void  (*alignRight)(S32 id);
      void  (*alignTop)(S32 id);
      void  (*alignBottom)(S32 id);

      void (*clearScrollArea)(S32 id);
      void (*seek)(S32 id);
      void (*clearSeek)();
   };

   struct NanoVGWrapper
   {
      void (*nvgSave)(NVGcontext* ctx);
      void (*nvgRestore)(NVGcontext* ctx);
      void (*nvgReset)(NVGcontext* ctx);

      NVGcolor (*nvgRGBA)(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
      NVGcolor (*nvgRGBAf)(float r, float g, float b, float a);

      void (*nvgBeginPath)(NVGcontext* ctx);
      void (*nvgMoveTo)(NVGcontext* ctx, float x, float y);
      void (*nvgBezierTo)(NVGcontext* ctx, float c1x, float c1y, float c2x, float c2y, float x, float y);

      void (*nvgCircle)(NVGcontext* ctx, float cx, float cy, float r);
      void (*nvgRect)(NVGcontext* ctx, float x, float y, float w, float h);
      void (*nvgRoundedRect)(NVGcontext* ctx, float x, float y, float w, float h, float r);

      NVGpaint (*nvgLinearGradient)(NVGcontext* ctx, float sx, float sy, float ex, float ey, NVGcolor icol, NVGcolor ocol);

      void (*nvgFill)(NVGcontext* ctx);
      void (*nvgFillColor)(NVGcontext* ctx, NVGcolor color);
      void (*nvgFillPaint)(NVGcontext* ctx, NVGpaint paint);

      void (*nvgStroke)(NVGcontext* ctx);
      void (*nvgStrokeColor)(NVGcontext* ctx, NVGcolor color);
      void (*nvgStrokeWidth)(NVGcontext* ctx, float size);

      void (*nvgFontFace)(NVGcontext* ctx, const char* font);
      void (*nvgFontSize)(NVGcontext* ctx, float size);
      float (*nvgText)(NVGcontext* ctx, float x, float y, const char* string, const char* end);
      void (*nvgTextAlign)(NVGcontext* ctx, int align);

      NVGpaint (*nvgImagePattern)(NVGcontext* ctx, float ox, float oy, float ex, float ey, float angle, int image, float alpha);
      void (*nvgImageSize)(NVGcontext* ctx, int image, int* w, int* h);
   };

   struct SceneWrapper
   {
      Scene::SceneCamera* (*getActiveCamera)();
      void (*pushActiveCamera)(const char *);
      void (*popActiveCamera)();
      void (*addCamera)(const char* name, Scene::SceneCamera* cam);
      Scene::SceneCamera* (*getCamera)(const char *);
      SimGroup* (*getEntityGroup)();
      Scene::SceneEntity* (*raycast)(Point3F start, Point3F end);

      Point3F* directionalLightDir;
      ColorF*  directionalLightColor;
      ColorF*  directionalLightAmbient;
      void (*setDirectionalLight)(Point3F direction, ColorF color, ColorF ambient);

      void (*addEntity)(Scene::SceneEntity* entity, const char* name); // Defaults: name = "SceneEntity"
      void (*removeEntity)(Scene::SceneEntity* entity);

      MaterialAsset* (*getMaterialAsset)(const char* id);
      MeshAsset* (*getMeshAsset)(const char* id);

      void (*refresh)();
   };

   struct PhysicsWrapper
   {
      void (*pause)();
      void (*resume)();
   };

   struct RenderingWrapper
   {
      bool*    canvasSizeChanged;
      U32*     canvasHeight; 
      U32*     canvasWidth;
      F32*     viewMatrix;
      F32*     projectionMatrix;

      Point2I (*worldToScreen)(Point3F worldPos);
      Point3F (*screenToWorld)(Point2I screenPos);
      Rendering::RenderData* (*createRenderData)();

      Rendering::DeferredRendering* (*getDeferredRendering)();
   };

   struct GraphicsWrapper
   {
      bgfx::VertexDecl* PosUVNormalVertex;
      bgfx::VertexDecl* PosUVColorVertex;

      bgfx::IndexBufferHandle* cubeIB;
      bgfx::VertexBufferHandle* cubeVB;

      TextureObject* (*loadTexture)(const char* pTextureKey, TextureHandle::TextureHandleType type, U32 flags, bool checkOnly, bool force16Bit );
      bgfx::UniformHandle (*getTextureUniform)(U32 slot);
      bgfx::UniformHandle (*getUniformVec4)(const char* name, U32 count);
      bgfx::UniformHandle (*getUniformMat4)(const char* name, U32 count);
      Graphics::Shader* (*getShader)(const char* vertex_shader_path, const char* fragment_shader_path, bool defaultPath); // Defaults: defaultPath = true
      Graphics::ShaderAsset* (*getShaderAsset)(const char* id);

      void (*fullScreenQuad)(F32 _textureWidth, F32 _textureHeight, F32 _x); // Defaults: _x = 0.0f
      void (*screenSpaceQuad)(F32 _x, F32 _y, F32 _width, F32 _height, F32 _targetWidth, F32 _targetHeight);
      void (*dglScreenQuad)(U32 _x, U32 _y, U32 _width, U32 _height);
      void (*drawLine3D)(Point3F start, Point3F end, ColorI color, F32 lineWidth);
      void (*drawBox3D)(Box3F box, ColorI color, F32 lineWidth);
      NVGcontext* (*dglGetNVGContext)();

      Graphics::ViewTableEntry* (*getView)(const char* name, S16 priority);
   };

   struct AssetDatabaseWrapper
   {
      S32 (*findAssetType)( AssetQuery* pAssetQuery, const char* pAssetType, const bool assetQueryAsSource ); // Defaults: assetQueryAsSource = false
   };

   struct BGFXWrapper
   {
      void (*dbgTextClear)(uint8_t _attr, bool _small); // Defaults: _attr = 0, _small = false
	   void (*dbgTextPrintf)(uint16_t _x, uint16_t _y, uint8_t _attr, const char* _format, ...);

      void (*setViewClear)(uint8_t _id, uint16_t _flags, uint32_t _rgba, float _depth, uint8_t _stencil); // Defaults: _depth = 1.0, _stencil = 0
      void (*setViewRect)(uint8_t _id, uint16_t _x, uint16_t _y, uint16_t _width, uint16_t _height);
      void (*setViewTransform)(uint8_t _id, const void* _view, const void* _projL, uint8_t _flags, const void* _projR); // Defaults: _flags = BGFX_VIEW_STEREO, _projR = NULL

      uint32_t (*setTransform)(const void* _mtx, uint16_t _num);
      void (*setTexture)(uint8_t _stage, bgfx::UniformHandle _sampler, bgfx::TextureHandle _handle, uint32_t _flags); // Defaults: _flags = UINT32_MAX
      void (*setState)(uint64_t _state, uint32_t _rgba); // Defaults: _rgba = 0
      void (*setUniform)(bgfx::UniformHandle _handle, const void* _value, uint16_t _num); // Defaults: _num = 1

      uint32_t (*touch)(uint8_t _id);
      uint32_t (*submit)(uint8_t _id, bgfx::ProgramHandle _handle, int32_t _depth); // Defaults: _depth = 0

      const bgfx::Memory* (*makeRef)(const void* _data, uint32_t _size, bgfx::ReleaseFn _releaseFn, void* _userData); // Defaults: _releaseFn = NULL, _userData = NULL

   	bgfx::IndexBufferHandle (*createIndexBuffer)(const bgfx::Memory* _mem, uint16_t _flags); // Defaults: _flags = BGFX_BUFFER_NONE
	   void (*destroyIndexBuffer)(bgfx::IndexBufferHandle _handle);

      bgfx::DynamicIndexBufferHandle (*createDynamicIndexBuffer)(const bgfx::Memory* _mem, uint16_t _flags); // Defaults: _flags = BGFX_BUFFER_NONE
      void (*updateDynamicIndexBuffer)(bgfx::DynamicIndexBufferHandle _handle, uint32_t _startIndex, const bgfx::Memory* _mem);
      void (*destroyDynamicIndexBuffer)(bgfx::DynamicIndexBufferHandle _handle);

	   bgfx::VertexBufferHandle (*createVertexBuffer)(const bgfx::Memory* _mem, const bgfx::VertexDecl& _decl, uint16_t _flags); // Defaults: _flags = BGFX_BUFFER_NONE
	   void (*destroyVertexBuffer)(bgfx::VertexBufferHandle _handle);

      bgfx::DynamicVertexBufferHandle (*createDynamicVertexBuffer)(const bgfx::Memory* _mem, const bgfx::VertexDecl& _decl, uint16_t _flags); // Defaults: _flags = BGFX_BUFFER_NONE
      void (*updateDynamicVertexBuffer)(bgfx::DynamicVertexBufferHandle _handle, uint32_t _startVertex, const bgfx::Memory* _mem);
      void (*destroyDynamicVertexBuffer)(bgfx::DynamicVertexBufferHandle _handle);

      bgfx::FrameBufferHandle (*createFrameBuffer)(uint8_t _num, bgfx::TextureHandle* _handles, bool _destroyTextures); // Defaults: _destroyTextures = false
      void (*destroyFrameBuffer)(bgfx::FrameBufferHandle _handle);

      bgfx::TextureHandle (*createTexture2D)(uint16_t _width, uint16_t _height, uint8_t _numMips, bgfx::TextureFormat::Enum _format, uint32_t _flags, const bgfx::Memory* _mem); // Defaults: _flags = BGFX_TEXTURE_NONE, _mem = NULL
      void (*updateTexture2D)(bgfx::TextureHandle _handle, uint8_t _mip, uint16_t _x, uint16_t _y, uint16_t _width, uint16_t _height, const bgfx::Memory* _mem, uint16_t _pitch); // Defaults: _pitch = UINT16_MAX
      void (*destroyTexture)(bgfx::TextureHandle _handle);

      void (*setViewFrameBuffer)(uint8_t _id, bgfx::FrameBufferHandle _handle);

      const bgfx::Memory* (*alloc)(uint32_t _size);
      const bgfx::Memory* (*copy)(const void* _data, uint32_t _size);
   };

   class PluginAPI
   {
      public:
         char pluginName[256];
   };

   struct PluginAPIRequest
   {
      char pluginName[256];
      void (*requestCallback)(PluginAPI* api);
   };

   struct PluginLink
   {
      EngineWrapper           Engine;
      ConsoleWrapper          Con;
      SysGUIWrapper           SysGUI;
      NanoVGWrapper           NanoVG;
      SceneWrapper            Scene;
      PhysicsWrapper          Physics;
      RenderingWrapper        Rendering;
      GraphicsWrapper         Graphics;
      AssetDatabaseWrapper    AssetDatabaseLink;
      BGFXWrapper             bgfx;
      
      _StringTable*           StringTableLink;
      ResManager*             ResourceManager;

      void (*addPluginAPI)(PluginAPI* api);
      void (*requestPluginAPI)(const char* name, void (*requestCallback)(PluginAPI* api));
   };

   extern DLL_PUBLIC Plugins::PluginLink Link;
   extern DLL_PUBLIC Vector<AbstractClassRep*> _pluginConsoleClasses;
   extern DLL_PUBLIC Vector<PluginAPI*> _pluginAPIs;
   extern DLL_PUBLIC Vector<PluginAPIRequest> _pluginAPIRequests;
}

// ----------------------------------------
//  Plugin Macros
// ----------------------------------------
//   Engine Side:
//
//     - PLUGIN_FUNC_PTR:
//         Defines a function pointer that can optionally be filled by a plugin.
//         Example: PLUGIN_FUNC_PTR(interpolateTick, F32 delta)
//
//     - IMPLEMENT_PLUGIN_CONOBJECT(className):
//         Plugin equivilant of IMPLEMENT_CONOBJECT
//
//     - DECLARE_PLUGIN_CONOBJECT(className):
//         Plugin equivilant of DECLARE_CONOBJECT
//
//   Plugin Side:
//
//     - PLUGIN_FUNC:
//         Opposite to PLUGIN_FUNC_PTR. Defines the function on the plugin side.
//         Example: PLUGIN_FUNC(interpolateTick, F32 delta)

#define PLUGIN_FUNC_PTR(name, ...) \
   typedef void (*name##Func)(__VA_ARGS__); \
   name##Func _##name;

#ifdef __GNUC__
   #define PLUGIN_FUNC(name, ...) \
   extern "C" { __attribute__ ((dllexport)) void name##(__VA_ARGS__); }
#else
   #define PLUGIN_FUNC(name, ...) \
   extern "C" { __declspec(dllexport) void name##(__VA_ARGS__); }
#endif

#define IMPLEMENT_PLUGIN_CONOBJECT(className)                                                                       \
    AbstractClassRep* className::getClassRep() const { return &className::dynClassRep; }                            \
    AbstractClassRep* className::getStaticClassRep() { return &dynClassRep; }                                       \
    AbstractClassRep* className::getParentStaticClassRep() { return Parent::getStaticClassRep(); }                  \
    AbstractClassRep* className::getContainerChildStaticClassRep() { return NULL; }                                 \
    AbstractClassRep::WriteCustomTamlSchema className::getStaticWriteCustomTamlSchema() { return NULL; }            \
    className::className##Rep className::dynClassRep(#className, 0, -1, 0, className::getParentStaticClassRep())

#define DECLARE_PLUGIN_CONOBJECT(className)                                                                                         \
    class className##Rep;                                                                                                           \
    static className##Rep dynClassRep;                                                                                              \
    static AbstractClassRep* getParentStaticClassRep();                                                                             \
    static AbstractClassRep* getContainerChildStaticClassRep();                                                                     \
    static AbstractClassRep* getStaticClassRep();                                                                                   \
    static AbstractClassRep::WriteCustomTamlSchema getStaticWriteCustomTamlSchema();                                                \
    virtual AbstractClassRep* getClassRep() const;                                                                                  \
    class className##Rep : public AbstractClassRep {                                                                                \
      public:                                                                                                                       \
         className##Rep(const char *name, S32 netClassGroupMask, S32 netClassType, S32 netEventDir, AbstractClassRep *parent) {     \
            mClassName = name;                                                                                                      \
            for (U32 i = 0; i < NetClassGroupsCount; i++) mClassId[i] = -1;                                                         \
            mClassType = netClassType;                                                                                              \
            mClassGroupMask = netClassGroupMask;                                                                                    \
            mNetEventDir = netEventDir;                                                                                             \
            parentClass = parent;                                                                                                   \
            Plugins::_pluginConsoleClasses.push_back(this);                                                                         \
      }                                                                                                                             \
      void registerClass() {                                                                                                        \
         Plugins::Link.Con.registerClassRep( this );                                                                                \
         mNamespace = Plugins::Link.Con.lookupNamespace( Plugins::Link.StringTableLink->insert( getClassName() ) );                 \
         mNamespace->mClassRep = this;                                                                                              \
         sg_tempFieldList.setSize(0);                                                                                               \
         init();                                                                                                                    \
         if ( sg_tempFieldList.size() != 0 )                                                                                        \
            mFieldList = sg_tempFieldList;                                                                                          \
         sg_tempFieldList.clear();                                                                                                  \
      }                                                                                                                             \
      virtual AbstractClassRep* getContainerChildClass(const bool recurse) {                                                        \
         AbstractClassRep* pChildren = className::getContainerChildStaticClassRep();                                                \
         if (!recurse || pChildren != NULL) return pChildren;                                                                       \
         AbstractClassRep* pParent = className::getParentStaticClassRep();                                                          \
         if (pParent == NULL) return NULL;                                                                                          \
         return pParent->getContainerChildClass(recurse);                                                                           \
      }                                                                                                                             \
      virtual WriteCustomTamlSchema getCustomTamlSchema(void) { return className::getStaticWriteCustomTamlSchema(); }               \
      void init() const {                                                                                                           \
         AbstractClassRep *parent = className::getParentStaticClassRep();                                                           \
         AbstractClassRep *child = className::getStaticClassRep();                                                                  \
         if (parent && child) Plugins::Link.Con.classLinkNamespaces(parent->getNameSpace(), child->getNameSpace());                 \
         className::initPersistFields();                                                                                            \
         className::consoleInit();                                                                                                  \
      }                                                                                                                             \
      ConsoleObject* create() const { return new className; }                                                                       \
   };                                                                                           

#endif