//-----------------------------------------------------------------------------
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


#include "deferredRendering.h"
#include "console/consoleInternal.h"
#include "graphics/shaders.h"
#include "graphics/dgl.h"
#include "3d/scene/core.h"

#include <bgfx.h>
#include <bx/fpumath.h>
#include <bx/timer.h>

namespace Rendering
{
   static DeferredRendering* gDeferredRenderingInst = NULL;

   DeferredRendering* getDeferredRendering()
   {
      return gDeferredRenderingInst;
   }

   void deferredInit()
   {
      if (gDeferredRenderingInst != NULL ) return;
      gDeferredRenderingInst = new DeferredRendering();
   }

   void deferredDestroy()
   {
      SAFE_DELETE(gDeferredRenderingInst);
   }

   DeferredRendering::DeferredRendering()
   {
      mGBufferTextures[0].idx = bgfx::invalidHandle;
      mGBufferTextures[1].idx = bgfx::invalidHandle;
      mGBufferTextures[2].idx = bgfx::invalidHandle;
      mGBufferTextures[3].idx = bgfx::invalidHandle;
      mGBuffer.idx            = bgfx::invalidHandle; 
      mLightBuffer.idx        = bgfx::invalidHandle; 
      mFinalBuffer.idx        = bgfx::invalidHandle;

      mCombineShader = Graphics::getShader("rendering/combine_vs.sc", "rendering/combine_fs.sc");

      // Load Ambient Cubemap ( TEMP )
      ambientCubemap.idx = bgfx::invalidHandle;
      TextureObject* ambientCubemapTex = TextureManager::loadTexture("pisa_lod.dds", TextureHandle::BitmapKeepTexture, false);
      if ( ambientCubemapTex != NULL )
         ambientCubemap = ambientCubemapTex->getBGFXTexture();
      u_ambientCube = Graphics::Shader::getUniform("u_ambientCube", bgfx::UniformType::Int1);

      ambientIrrCubemap.idx = bgfx::invalidHandle;
      TextureObject* ambientIrrCubemapTex = TextureManager::loadTexture("pisa_irr.dds", TextureHandle::BitmapKeepTexture, false);
      if ( ambientIrrCubemapTex != NULL )
         ambientIrrCubemap = ambientIrrCubemapTex->getBGFXTexture();
      u_ambientIrrCube = Graphics::Shader::getUniform("u_ambientIrrCube", bgfx::UniformType::Int1);

      // Get Views
      mDeferredGeometryView   = Graphics::getView("DeferredGeometry", 1000);
      mDeferredLightView      = Graphics::getView("DeferredLight", 1500);
      mRenderLayer0View       = Graphics::getView("RenderLayer0");

      initBuffers();

      setRendering(true);
   }

   DeferredRendering::~DeferredRendering()
   {
      destroyBuffers();
   }

   void DeferredRendering::initBuffers()
   {
      destroyBuffers();

      const U32 samplerFlags = 0
            | BGFX_TEXTURE_RT
            | BGFX_TEXTURE_MIN_POINT
            | BGFX_TEXTURE_MAG_POINT
            | BGFX_TEXTURE_MIP_POINT
            | BGFX_TEXTURE_U_CLAMP
            | BGFX_TEXTURE_V_CLAMP;

      // G-Buffer
      mGBufferTextures[0] = bgfx::createTexture2D(canvasWidth, canvasHeight, 1, bgfx::TextureFormat::BGRA8, samplerFlags);
      mGBufferTextures[1] = Rendering::getNormalTexture();
      mGBufferTextures[2] = Rendering::getMatInfoTexture();
      mGBufferTextures[3] = Rendering::getDepthTexture();
      mGBuffer = bgfx::createFrameBuffer(BX_COUNTOF(mGBufferTextures), mGBufferTextures, false);

      // Light Buffer
      mLightBuffer = bgfx::createFrameBuffer(canvasWidth, canvasHeight, bgfx::TextureFormat::BGRA8);

      // Final Buffer
      bgfx::TextureHandle fbtextures[] =
      {
         Rendering::getColorTexture(),
         bgfx::createTexture2D(canvasWidth, canvasHeight, 1, bgfx::TextureFormat::D16, BGFX_TEXTURE_RT_BUFFER_ONLY)
      };
      mFinalBuffer = bgfx::createFrameBuffer(BX_COUNTOF(fbtextures), fbtextures);
   }

   void DeferredRendering::destroyBuffers()
   {
      // Destroy Frame Buffers
      if ( bgfx::isValid(mGBuffer) )
         bgfx::destroyFrameBuffer(mGBuffer);
      if ( bgfx::isValid(mLightBuffer) )
         bgfx::destroyFrameBuffer(mLightBuffer);

      // Destroy G-Buffer Color/Lighting Textures
      if ( bgfx::isValid(mGBufferTextures[0]) )
         bgfx::destroyTexture(mGBufferTextures[0]);
   }

   void DeferredRendering::preRender()
   {
      // G-Buffer
      bgfx::setClearColor(0, UINT32_C(0x00000000) );

      bgfx::setViewClear(mDeferredGeometryView->id
         , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
         , 1.0f
         , 0
         , 0
         , 0
         , 0
      );
      bgfx::setViewRect(mDeferredGeometryView->id, 0, 0, canvasWidth, canvasHeight);
      bgfx::setViewFrameBuffer(mDeferredGeometryView->id, mGBuffer);
      bgfx::setViewTransform(mDeferredGeometryView->id, viewMatrix, projectionMatrix);
      bgfx::touch(mDeferredGeometryView->id);

      // Light Buffer
      bgfx::setViewClear(mDeferredLightView->id
         , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
         , 1.0f
         , 0
         , 0
      );
      bgfx::setViewRect(mDeferredLightView->id, 0, 0, canvasWidth, canvasHeight);
      bgfx::setViewFrameBuffer(mDeferredLightView->id, mLightBuffer);
      bgfx::setViewTransform(mDeferredLightView->id, viewMatrix, projectionMatrix);
      bgfx::touch(mDeferredLightView->id);

      // Temp hack.
      bgfx::setViewFrameBuffer(mRenderLayer0View->id, mFinalBuffer);
   }

   void DeferredRendering::render()
   {
      //
   }

   void DeferredRendering::postRender()
   {
      // This projection matrix is used because its a full screen quad.
      F32 proj[16];
      bx::mtxOrtho(proj, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f);
      bgfx::setViewTransform(mRenderLayer0View->id, NULL, proj);
      bgfx::setViewRect(mRenderLayer0View->id, 0, 0, canvasWidth, canvasHeight);

      // Combine Color + Light
      bgfx::setTexture(0, Graphics::Shader::getTextureUniform(0), mGBuffer, 0);                     // Albedo
      bgfx::setTexture(1, Graphics::Shader::getTextureUniform(1), Rendering::getNormalTexture());  // Normals
      bgfx::setTexture(2, Graphics::Shader::getTextureUniform(2), mGBuffer, 2);                     // Material Info
      bgfx::setTexture(3, Graphics::Shader::getTextureUniform(3), Rendering::getDepthTexture());   // Depth Buffer
      bgfx::setTexture(4, Graphics::Shader::getTextureUniform(4), mLightBuffer, 0);                 // Light Buffer

      // Real Time Ambient
      //bgfx::setTexture(5, Graphics::Shader::getTextureUniform(5), Rendering::getDirectLightVolume());
      //bgfx::setTexture(6, Graphics::Shader::getTextureUniform(6), Rendering::getSurfaceNormalVolume());

      // Ambient Cubemap, Ambient Irradience Cubemap
      bgfx::setTexture(5, u_ambientCube, ambientCubemap);
      bgfx::setTexture(6, u_ambientIrrCube, ambientIrrCubemap);

      bgfx::setState(0
         | BGFX_STATE_RGB_WRITE
         | BGFX_STATE_ALPHA_WRITE
         );

      fullScreenQuad((F32)canvasWidth, (F32)canvasHeight);

      bgfx::submit(mRenderLayer0View->id, mCombineShader->mProgram);
   }
}
