#ifndef COIN_CVRTEXTUREOBJECT_H
#define COIN_CVRTEXTUREOBJECT_H

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

#include <Inventor/SoType.h>
#include <Inventor/SbVec3s.h>
#include <Inventor/system/gl.h>

class SoGLRenderAction;
class SoVolumeData;
class SbBox3s;

// *************************************************************************

class CvrTextureObject {
public:
  static const CvrTextureObject * create(const SoGLRenderAction * action,
                                         const SbVec3s & texsize,
                                         const SbBox3s & cutcube);

  static void initClass(void);

  virtual SoType getTypeId(void) const = 0;
  static SoType getClassTypeId(void);

  uint32_t getRefCount(void) const;
  void ref(void) const;
  void unref(void) const;

  const SbVec3s & getDimensions(void) const;

  GLuint getGLTexture(const SoGLRenderAction * action) const;

  // FIXME: these need to go -- a CvrTextureObject can have several GL
  // textures bound to it, some compressed, some not. 20040716 mortene.
  SbBool textureCompressed() const;
  void setTextureCompressed(SbBool flag);

  virtual SbBool isPaletted(void) const = 0;

  virtual void blankUnused(const SbVec3s & texsize) const = 0;

protected:
  // Constructor and destructor is protected as only the texture
  // manager is allowed to create and remove TextureObjects.
  CvrTextureObject(const SbVec3s & size);
  virtual ~CvrTextureObject();

private:
  SbBool findGLTexture(const SoGLRenderAction * action, GLuint & texid) const;
  static CvrTextureObject * new3DTextureObject(const SoGLRenderAction * action,
                                               const SoVolumeData * voldata,
                                               const SbVec3s & texsize,
                                               const SbBox3s & cutcube);

  static SoType classTypeId;
  SbBool iscompressed;
  SbVec3s dimensions;
  uint32_t refcounter;
};

#endif // !COIN_CVRTEXTUREOBJECT_H
