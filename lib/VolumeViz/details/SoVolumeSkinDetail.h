#ifndef COIN_SOVOLUMESKINDETAIL_H
#define COIN_SOVOLUMESKINDETAIL_H

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

#include <Inventor/details/SoDetail.h>
#include <Inventor/details/SoSubDetail.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec3s.h>
#include <Inventor/lists/SbList.h>

#include <VolumeViz/C/basic.h>
#include <VolumeViz/details/SoVolumeDetail.h>

class SIMVOLEON_DLL_API SoVolumeSkinDetail : public SoVolumeDetail {
  typedef SoDetail inherited;

  SO_DETAIL_HEADER(SoVolumeSkinDetail);

public:
  static void initClass(void);
  SoVolumeSkinDetail(void);
  virtual ~SoVolumeSkinDetail();
 
};

#endif // !COIN_SOVOLUMESKINDETAIL_H