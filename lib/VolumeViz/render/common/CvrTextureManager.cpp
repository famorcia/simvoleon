/**************************************************************************\
 *
 *  This file is part of the SIM Voleon visualization library.
 *  Copyright (C) 2003-2004 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SIM Voleon with software that can not be combined with
 *  the GNU GPL, and for taking advantage of the additional benefits
 *  of our support services, please contact Systems in Motion about
 *  acquiring a SIM Voleon Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org/> for more information.
 *
 *  Systems in Motion, Teknobyen, Abels Gate 5, 7030 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <Inventor/system/gl.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/C/glue/gl.h>
#include <Inventor/SbDict.h>
#include <Inventor/errors/SoDebugError.h>

#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/render/common/CvrRGBATexture.h>
#include <VolumeViz/render/common/CvrPaletteTexture.h>
#include <VolumeViz/render/common/Cvr3DRGBATexture.h>
#include <VolumeViz/render/common/Cvr3DPaletteTexture.h>
#include <VolumeViz/render/common/Cvr2DRGBATexture.h>
#include <VolumeViz/render/common/Cvr2DPaletteTexture.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrUtil.h>

#include "CvrTextureManager.h"

static SbDict texturedict;

enum TEXTURETYPE {
  TEXTURE2D,
  TEXTURE3D,
  UNKNOWN
};

struct textureobj {
  SoVolumeData * voldata;
  uint32_t voldataid; // This will change if volume-data has been modified by the user.
  TEXTURETYPE texturetype;
  SbBox3s cutcube;
  SbBox2s cutpage;
  CvrTextureObject * object;
};

unsigned int CvrTextureManager::totaltexturesize = 0;
unsigned int CvrTextureManager::totalnumberoftexels = 0;

static textureobj * 
findTextureObject3D(const SoVolumeData * voldata, const SbBox3s cutcube)
{
  SbPList keylist;
  SbPList valuelist;
  texturedict.makePList(keylist, valuelist);

  for (int i=0;i<valuelist.getLength();++i) {
    void * ptr = valuelist[i];
    textureobj * obj = (textureobj *) ptr;
    if ((obj->voldata == voldata) &&
        (obj->voldataid == voldata->getNodeId()) &&
        (obj->cutcube == cutcube) &&
        (obj->texturetype == TEXTURE3D))
      return obj;
  }

  return NULL;
}

static textureobj * 
findTextureObject2D(const SoVolumeData * voldata, const SbBox2s cutpage)
{
  SbPList keylist;
  SbPList valuelist;
  texturedict.makePList(keylist, valuelist);

  for (int i=0;i<valuelist.getLength();++i) {
    void * ptr = valuelist[i];
    textureobj * obj = (textureobj *) ptr;
    if ((obj->voldata == voldata) &&
        (obj->voldataid == voldata->getNodeId()) &&
        (obj->cutpage == cutpage) &&
        (obj->texturetype == TEXTURE2D))
      return obj;
  }

  return NULL;

}

const CvrTextureObject *
CvrTextureManager::getTextureObject(SoGLRenderAction * action, 
                                    SoVolumeData * voldata, 
                                    SbVec3s texsize,
                                    SbBox3s cutcube)
{
  textureobj * obj = findTextureObject3D(voldata, cutcube);
    
  if (obj != NULL) {
    assert((obj->texturetype == TEXTURE3D) && "Type != TEXTURE3D. Invalid texture type!");
    obj->object->ref();
    return obj->object; 
  }
         
  CvrTextureObject * newobj = new3DTextureObject(action, voldata, texsize, cutcube);
  obj = new textureobj;
  obj->object = newobj;
  obj->voldata = voldata;
  obj->voldataid = voldata->getNodeId();
  obj->cutcube = cutcube;
  obj->texturetype = TEXTURE3D;
  texturedict.enter((unsigned long) newobj, (void *) obj);
  newobj->ref();

  return newobj;
}

const CvrTextureObject *
CvrTextureManager::getTextureObject(SoGLRenderAction * action, 
                                    SoVolumeData * voldata, 
                                    SbVec2s texsize,
                                    SbBox2s cutpage)
{ 
  assert(FALSE && "Not implemented yet for 2D textures.");
  return NULL;
}

void
CvrTextureManager::finalizeTextureObject(const CvrTextureObject * texobject)
{
  CvrTextureObject * nocast = (CvrTextureObject *) texobject; 
  if (!nocast->unref()) { // unref returned FALSE ==> 'refcounter <= 0' 
    void * ptr;
    texturedict.find((unsigned long) texobject, ptr);    
    assert(ptr && "Trying to remove a CvrTextureObject which has not been registered!");    
    GLuint tmp[1];
    tmp[0] = texobject->getOpenGLTextureId();
    glDeleteTextures(1, tmp);

    // Update total memory usage status
    const SbBool palette = ((nocast->getTypeId() == CvrPaletteTexture::getClassTypeId()));
    const SbBool tex3d = ((nocast->getTypeId() == Cvr3DRGBATexture::getClassTypeId()) ||
                          (nocast->getTypeId() == Cvr3DPaletteTexture::getClassTypeId()));

    // FIXME: This must be fixed the day we support 16 or 24 bits
    // textures. (20040625 handegar)
    const int voxelsize = palette ? 1 : 4;
    unsigned int nrtexels = 0;

    if (tex3d) {         
      SbVec3s size;
      if (palette) size = ((Cvr3DPaletteTexture *) nocast)->dimensions;
      else size = ((Cvr3DRGBATexture *) nocast)->dimensions;
      nrtexels = size[0] * size[1] * size[2];
    }
    else {
      SbVec2s size;
      if (palette) size = ((Cvr2DPaletteTexture *) nocast)->dimensions;
      else size = ((Cvr2DRGBATexture *) nocast)->dimensions;
      nrtexels = size[0] * size[1];
    }

    CvrTextureManager::totalnumberoftexels -= nrtexels;
    CvrTextureManager::totaltexturesize -= nrtexels * voxelsize;

    delete ((textureobj *) ptr);
    delete nocast;    
  }
}


CvrTextureObject *
CvrTextureManager::new3DTextureObject(SoGLRenderAction * action, 
                                      SoVolumeData * voldata, 
                                      SbVec3s texsize,
                                      SbBox3s cutcube)
{

  SbVec3s vddims;
  void * dataptr;
  SoVolumeData::DataType type;
  SbBool ok = voldata->getVolumeData(vddims, dataptr, type);
  assert(ok);

  CvrVoxelChunk::UnitSize vctype;
  switch (type) {
  case SoVolumeData::UNSIGNED_BYTE: vctype = CvrVoxelChunk::UINT_8; break;
  case SoVolumeData::UNSIGNED_SHORT: vctype = CvrVoxelChunk::UINT_16; break;
  default: assert(FALSE); break;
  }

  CvrVoxelChunk * input = new CvrVoxelChunk(vddims, vctype, dataptr);
  CvrVoxelChunk * cubechunk = input->buildSubCube(cutcube);
  delete input;

  SbBool invisible = FALSE;
  CvrTextureObject * newtexobj = NULL;
  newtexobj = cubechunk->transfer3D(action, invisible); 

  if (invisible) // Is the cubechunk 'empty'?
    return NULL;
  
  // Must clear unused texture area to prevent artifacts due to
  // floating point inaccuracies when calculating texture coords.
  SbVec3s realtexsize;
  if (newtexobj->getTypeId() == Cvr3DRGBATexture::getClassTypeId()) {
    ((Cvr3DRGBATexture *) newtexobj)->blankUnused(texsize);
    realtexsize = ((Cvr3DRGBATexture *) newtexobj)->dimensions;
  } else if (newtexobj->getTypeId() == Cvr3DPaletteTexture::getClassTypeId()) {
    ((Cvr3DPaletteTexture *) newtexobj)->blankUnused(texsize);
    realtexsize = ((Cvr3DPaletteTexture *) newtexobj)->dimensions;
  }
  
  newtexobj->setTextureCompressed(voldata->useCompressedTexture.getValue());
  transferTex3GL(action, newtexobj, realtexsize);
 
  return newtexobj;
}


CvrTextureObject *
CvrTextureManager::new2DTextureObject(SoGLRenderAction * action, 
                                      SoVolumeData * voldata, 
                                      SbVec2s texsize,
                                      SbBox2s cutpage)
{
  assert(FALSE && "Not implementet yet.");
  return NULL;
}


void
CvrTextureManager::transferTex2GL(SoGLRenderAction * action,
                                  CvrTextureObject * texobj,
                                  SbVec2s texdims)
{
  assert(FALSE && "Not implemented yet.");
}

void
CvrTextureManager::transferTex3GL(SoGLRenderAction * action,
                                  CvrTextureObject * texobj,
                                  SbVec3s texdims)
{
  
  const cc_glglue * glw = cc_glglue_instance(action->getCacheContext());

  int colorformat;
  int bitspertexel;
  GLuint texturename;
  
  int ispaletted = (texobj->getTypeId() == Cvr3DPaletteTexture::getClassTypeId());
  assert(ispaletted || texobj->getTypeId() == Cvr3DRGBATexture::getClassTypeId());

  // Standard RGBA-texture
  if (!ispaletted) {
    colorformat = 4;
    bitspertexel = 32; // 8 bits each R, G, B & A
  }
  // Paletted texture
  else {
    colorformat = GL_COLOR_INDEX8_EXT;
    bitspertexel = 8;
  }

  const int nrtexels = texdims[0] * texdims[1] * texdims[2];
  const int texmem = int(float(nrtexels) * float(bitspertexel) / 8.0f);

  // This is a debugging backdoor to test stuff with no limits on how
  // much texture memory we can use.
  static int unlimited_texmem = 1;
  if (unlimited_texmem == -1) {
    const char * envstr = coin_getenv("CVR_UNLIMITED_TEXMEM");
    if (envstr) { unlimited_texmem = atoi(envstr) > 0 ? 1 : 0; }
    else unlimited_texmem = 0;
  }
  if (!unlimited_texmem) {   
    // FIXME: A proper mechanism for setting max-texture memory usage
    // should be implemented here. (20040622 handegar)
    texturename = 0;
  }
  else {
        
    CvrTextureManager::totalnumberoftexels += nrtexels;
    CvrTextureManager::totaltexturesize += texmem;

    glGenTextures(1, &texturename);
    assert(glGetError() == GL_NO_ERROR);

    glEnable(GL_TEXTURE_3D);
    glBindTexture(GL_TEXTURE_3D, texturename);
    assert(glGetError() == GL_NO_ERROR);

    GLint wrapenum = GL_CLAMP;
    // FIXME: investigate if this is really what we want. 20021120 mortene.
    if (cc_glglue_has_texture_edge_clamp(glw)) { wrapenum = GL_CLAMP_TO_EDGE; }
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrapenum);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrapenum);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrapenum);
    assert(glGetError() == GL_NO_ERROR);

    void * imgptr = NULL;
    if (ispaletted) imgptr = ((CvrPaletteTexture *)texobj)->getIndex8Buffer();
    else imgptr = ((CvrRGBATexture *)texobj)->getRGBABuffer();

   
    int palettetype = GL_COLOR_INDEX;
#ifdef HAVE_ARB_FRAGMENT_PROGRAM
    if (cc_glglue_has_arb_fragment_program(glw)) 
      palettetype = GL_LUMINANCE;    
#endif
   
    // NOTE: Combining texture compression and GL_COLOR_INDEX doesn't
    // seem to work on NVIDIA cards (tested on GeForceFX 5600 &
    // GeForce2 MX) (20040316 handegar)   
    if (cc_glue_has_texture_compression(glw) && 
        texobj->textureCompressed() &&
        palettetype != GL_COLOR_INDEX) {
      if (colorformat == 4) colorformat = GL_COMPRESSED_RGBA_ARB;
      else colorformat = GL_COMPRESSED_INTENSITY_ARB;
    }
    
    if (!CvrUtil::dontModulateTextures()) // Is texture modulation disabled by an envvar?
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    cc_glglue_glTexImage3D(glw,
                           GL_TEXTURE_3D,
                           0,
                           colorformat,
                           texdims[0], texdims[1], texdims[2],
                           0,
                           ispaletted ? palettetype : GL_RGBA,
                           GL_UNSIGNED_BYTE,
                           imgptr);
    
    assert(glGetError() == GL_NO_ERROR);    
  }
  
  texobj->setOpenGLTextureId(texturename);
  
}

unsigned int
CvrTextureManager::totalNrOfTexels(void)
{
  return CvrTextureManager::totalnumberoftexels;
}
unsigned int
CvrTextureManager::totalTextureMemoryUsed(void)
{
  return CvrTextureManager::totaltexturesize;
}
