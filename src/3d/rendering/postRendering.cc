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

#include "postRendering.h"
#include "console/consoleInternal.h"
#include "graphics/dgl.h"
#include "graphics/shaders.h"
#include "graphics/core.h"
#include "3d/scene/core.h"

#include <bgfx.h>
#include <bx/fpumath.h>
#include <bx/timer.h>

namespace Rendering
{
   static PostRendering*            gPostRenderingInst = NULL;
   static U32                       gPostBufferIdx = 0;
   static bgfx::FrameBufferHandle   gPostBuffers[2] = { BGFX_INVALID_HANDLE, BGFX_INVALID_HANDLE };

   void postInit()
   {
      if (gPostRenderingInst != NULL ) return;
      gPostRenderingInst = new PostRendering();

      // Create two buffers for flip-flopping.
      const U32 samplerFlags = 0
            | BGFX_TEXTURE_RT
            | BGFX_TEXTURE_MIN_POINT
            | BGFX_TEXTURE_MAG_POINT
            | BGFX_TEXTURE_MIP_POINT
            | BGFX_TEXTURE_U_CLAMP
            | BGFX_TEXTURE_V_CLAMP;

      gPostBuffers[0] = bgfx::createFrameBuffer(Rendering::canvasWidth, Rendering::canvasHeight, bgfx::TextureFormat::BGRA8, samplerFlags);
      gPostBuffers[1] = bgfx::createFrameBuffer(Rendering::canvasWidth, Rendering::canvasHeight, bgfx::TextureFormat::BGRA8, samplerFlags);
   }

   void postDestroy()
   {
      SAFE_DELETE(gPostRenderingInst);

      if ( bgfx::isValid(gPostBuffers[0]) )
         bgfx::destroyFrameBuffer(gPostBuffers[0]);
      if ( bgfx::isValid(gPostBuffers[1]) )
         bgfx::destroyFrameBuffer(gPostBuffers[1]);
   }

   bgfx::FrameBufferHandle getPostSource()
   {
      return gPostBuffers[gPostBufferIdx];
   }

   bgfx::FrameBufferHandle getPostTarget()
   {
      U32 targetIdx = gPostBufferIdx == 0 ? 1 : 0;
      return gPostBuffers[targetIdx];
   }

   Graphics::ViewTableEntry* overridePostBegin()
   {
      return gPostRenderingInst->overrideBegin();
   }

   Graphics::ViewTableEntry* overridePostFinish()
   {
      return gPostRenderingInst->overrideFinish();
   }

   void flipPostBuffers()
   {
      gPostBufferIdx = gPostBufferIdx == 0 ? 1 : 0;
   }

   // --------------------------------

   IMPLEMENT_CONOBJECT(PostRenderFeature);

   void PostRenderFeature::onActivate()
   {
      gPostRenderingInst->addPostFeature(this);
   }

   void PostRenderFeature::onDeactivate()
   {
      gPostRenderingInst->removePostFeature(this);
   }

   // --------------------------------

   PostRendering::PostRendering()
      : mBeginEnabled(true),
        mFinishEnabled(true)
   {
      setRendering(true);

      // Shaders
      mBeginShader   = Graphics::getShader("rendering/begin_vs.sc", "rendering/begin_fs.sc");
      mFinishShader  = Graphics::getShader("rendering/finish_vs.sc", "rendering/finish_fs.sc");

      // Views
      mBeginView  = Graphics::getView("Post_Begin", 4000);
      mFinishView = Graphics::getView("Post_Finish", 5000);
   }

   PostRendering::~PostRendering()
   {
      // Unused. 
   }

   int QSORT_CALLBACK comparePostFeaturePriority(const void * a, const void * b)
   {
      return ((*(Rendering::PostRenderFeature**)a)->mPriority - (*(Rendering::PostRenderFeature**)b)->mPriority);
   }

   void PostRendering::addPostFeature(PostRenderFeature* feature)
   {
      mPostFeatureList.push_back(feature);
      qsort((void *)mPostFeatureList.address(), mPostFeatureList.size(), sizeof(Rendering::PostRenderFeature*), comparePostFeaturePriority);
   }

   void PostRendering::removePostFeature(PostRenderFeature* feature)
   {
      for (int i = 0; i < mPostFeatureList.size(); ++i)
      {
         if (mPostFeatureList[i] == feature)
         {
            mPostFeatureList.erase(i);
            return;
         }
      }
   }

   Graphics::ViewTableEntry* PostRendering::overrideBegin()
   {
      mBeginEnabled = false;
      return mBeginView;
   }

   Graphics::ViewTableEntry* PostRendering::overrideFinish() 
   {
      mFinishEnabled = false;
      return mFinishView;
   }

   void PostRendering::preRender()
   {
      // Unused.
   }

   void PostRendering::render()
   {
      // Unused.
   }

   void PostRendering::postRender()
   {
      F32 proj[16];
      bx::mtxOrtho(proj, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f);

      // Begin
      if (mBeginEnabled)
      {
         bgfx::setViewFrameBuffer(mBeginView->id, getPostTarget());
         bgfx::setViewTransform(mBeginView->id, NULL, proj);
         bgfx::setViewRect(mBeginView->id, 0, 0, Rendering::canvasWidth, Rendering::canvasHeight);
         bgfx::setTexture(0, Graphics::Shader::getTextureUniform(0), getPostSource());
         bgfx::setState(0
            | BGFX_STATE_RGB_WRITE
            | BGFX_STATE_ALPHA_WRITE
            //| BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
            );
         fullScreenQuad((F32)canvasWidth, (F32)canvasHeight);
         bgfx::submit(mBeginView->id, mBeginShader->mProgram);
         flipPostBuffers();
      }

      // Post Rendering Features
      for (S32 n = 0; n < mPostFeatureList.size(); ++n)
      {
         PostRenderFeature* feature = mPostFeatureList[n];
         feature->render();
         flipPostBuffers();
      }

      // Finish
      if (mFinishEnabled)
      {
         bgfx::setViewTransform(mFinishView->id, NULL, proj);
         bgfx::setViewRect(mFinishView->id, 0, 0, Rendering::canvasWidth, Rendering::canvasHeight);
         bgfx::setTexture(0, Graphics::Shader::getTextureUniform(0), getPostSource());
         bgfx::setState(0
            | BGFX_STATE_RGB_WRITE
            | BGFX_STATE_ALPHA_WRITE
            //| BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
            );
         fullScreenQuad((F32)canvasWidth, (F32)canvasHeight);
         bgfx::submit(mFinishView->id, mFinishShader->mProgram);
      }
   }
}
