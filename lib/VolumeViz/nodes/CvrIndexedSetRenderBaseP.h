#ifndef CVR_INDEXEDSETRENDERBASEP_H
#define CVR_INDEXEDSETRENDERBASEP_H

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

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoCoordinateElement.h>
#include <Inventor/nodes/SoIndexedShape.h>
#include <Inventor/fields/SoMFInt32.h>

#include <VolumeViz/render/3D/Cvr3DTexCube.h>

// Internal class

class CvrIndexedSetRenderBaseP {

public:
  virtual SbBool getVertexData(SoState *state, 
                               const SoCoordinateElement *&coords, 
                               const SbVec3f *&normals, 
                               const int32_t *&cindices, 
                               const int32_t *&nindices, 
                               const int32_t *&tindices,
                               const int32_t *&mindices, 
                               int &numcindices, 
                               const SbBool needNormals, 
                               SbBool &normalCacheUsed) = 0;

  void GLRender(SoGLRenderAction * action, 
                const float offset,
                const SbBool clipGeometry);
  
  enum SetType { FACESET, TRIANGLESTRIPSET };

  Cvr3DTexCube * cube;
  const CvrCLUT * clut;
  uint32_t parentnodeid;
  SoIndexedShape * clipgeometryshape; 
  enum SetType type;

protected:
  SoIndexedShape * master; 

};

#endif // CVR_INDEXEDSETRENDERBASEP_H
