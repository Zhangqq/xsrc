/**************************************************************************
 * 
 * Copyright 2003 Tungsten Graphics, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

#include "i915_context.h"
#include "imports.h"
#include "intel_tex.h"
#include "intel_tris.h"
#include "tnl/t_context.h"
#include "tnl/t_pipeline.h"
#include "tnl/t_vertex.h"

#include "swrast/swrast.h"
#include "swrast_setup/swrast_setup.h"
#include "tnl/tnl.h"
#include "array_cache/acache.h"

#include "utils.h"
#include "i915_reg.h"

/***************************************
 * Mesa's Driver Functions
 ***************************************/

static const char * const card_extensions[] =
{
   "GL_ARB_depth_texture",           /* New: alanh 06-Jan-2005 */
   "GL_ARB_fragment_program",
   "GL_ARB_shadow",                  /* New: alanh 06-Jan-2005 */
   "GL_EXT_shadow_funcs",            /* New: alanh 06-Jan-2005 */
   "GL_SGIX_depth_texture",	/* ARB extn won't work if not enabled */
   NULL
};

/* Override intel default.
 */
static void i915InvalidateState( GLcontext *ctx, GLuint new_state )
{
   _swrast_InvalidateState( ctx, new_state );
   _swsetup_InvalidateState( ctx, new_state );
   _ac_InvalidateState( ctx, new_state );
   _tnl_InvalidateState( ctx, new_state );
   _tnl_invalidate_vertex_state( ctx, new_state );
   INTEL_CONTEXT(ctx)->NewGLState |= new_state;

   /* Todo: gather state values under which tracked parameters become
    * invalidated, add callbacks for things like
    * ProgramLocalParameters, etc.
    */
   {
      struct i915_fragment_program *p = 
	 (struct i915_fragment_program *)ctx->FragmentProgram.Current;
      if (p->nr_params)
	 p->params_uptodate = 0;
   }

}


static void i915InitDriverFunctions( struct dd_function_table *functions )
{
   intelInitDriverFunctions( functions );
   i915InitStateFunctions( functions );
   i915InitTextureFuncs( functions );
   i915InitFragProgFuncs( functions );
   functions->UpdateState = i915InvalidateState;
}



GLboolean i915CreateContext( const __GLcontextModes *mesaVis,
			    __DRIcontextPrivate *driContextPriv,
			    void *sharedContextPrivate)
{
   struct dd_function_table functions;
   i915ContextPtr i915 = (i915ContextPtr) CALLOC_STRUCT(i915_context);
   intelContextPtr intel = &i915->intel;
   GLcontext *ctx = &intel->ctx;

   if (!i915) return GL_FALSE;

   i915InitVtbl( i915 );

   i915InitDriverFunctions( &functions );

   if (!intelInitContext( intel, mesaVis, driContextPriv,
			  sharedContextPrivate, &functions )) {
      FREE(i915);
      return GL_FALSE;
   }

   ctx->Const.MaxTextureUnits = I915_TEX_UNITS;
   ctx->Const.MaxTextureImageUnits = I915_TEX_UNITS;
   ctx->Const.MaxTextureCoordUnits = I915_TEX_UNITS;

   intel->nr_heaps = 1;
   intel->texture_heaps[0] = 
      driCreateTextureHeap( 0, intel,
			    intel->intelScreen->textureSize,
			    12,
			    I830_NR_TEX_REGIONS,
			    intel->sarea->texList,
			    & intel->sarea->texAge,
			    & intel->swapped,
			    sizeof( struct i915_texture_object ),
			    (destroy_texture_object_t *)intelDestroyTexObj );

   /* FIXME: driCalculateMaxTextureLevels assumes that mipmaps are
    * tightly packed, but they're not in Intel graphics
    * hardware.
    */
   ctx->Const.MaxTextureUnits = 1;
   driCalculateMaxTextureLevels( intel->texture_heaps,
				 intel->nr_heaps,
				 &intel->ctx.Const,
				 4,
				 11, /* max 2D texture size is 2048x2048 */
				 8,  /* 3D texture */
				 11, /* cube texture. */
				 11, /* rect texture */
				 12,
				 GL_FALSE );
   ctx->Const.MaxTextureUnits = I915_TEX_UNITS;

   /* GL_ARB_fragment_program limits - don't think Mesa actually
    * validates programs against these, and in any case one ARB
    * instruction can translate to more than one HW instruction, so
    * we'll still have to check and fallback each time.
    */
   
   ctx->Const.MaxFragmentProgramTemps = I915_MAX_TEMPORARY;
   ctx->Const.MaxFragmentProgramAttribs = 11; /* 8 tex, 2 color, fog */
   ctx->Const.MaxFragmentProgramLocalParams = I915_MAX_CONSTANT;
   ctx->Const.MaxFragmentProgramEnvParams = I915_MAX_CONSTANT;
   ctx->Const.MaxFragmentProgramAluInstructions = I915_MAX_ALU_INSN;
   ctx->Const.MaxFragmentProgramTexInstructions = I915_MAX_TEX_INSN;
   ctx->Const.MaxFragmentProgramInstructions = (I915_MAX_ALU_INSN + 
						I915_MAX_TEX_INSN);
   ctx->Const.MaxFragmentProgramTexIndirections = I915_MAX_TEX_INDIRECT;
   ctx->Const.MaxFragmentProgramAddressRegs = 0; /* I don't think we have one */


   driInitExtensions( ctx, card_extensions, GL_FALSE );


   _tnl_init_vertices( ctx, ctx->Const.MaxArrayLockSize + 12, 
		       36 * sizeof(GLfloat) );

   intel->verts = TNL_CONTEXT(ctx)->clipspace.vertex_buf;

   i915InitState( i915 );

   return GL_TRUE;
}

