/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

/*
FIXME

  LUTs for all the predefined enums must be implemented. 

  The different ColorMapTypes must be implemented. Currently, only RGBA
  is supported.
*/


#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <memory.h>
#include <Inventor/elements/SoGLCacheContextElement.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H
#include <GL/gl.h>


// *************************************************************************

SO_NODE_SOURCE(SoTransferFunction);

// *************************************************************************

class SoTransferFunctionP{
public:
  SoTransferFunctionP(SoTransferFunction * master) {
    this->master = master;
  }

  int unpack(const void * data, int numBits, int index);
  void pack(void * data, int numBits, int index, int val);

private:
  SoTransferFunction * master;
};


#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

/*!
  Constructor.
*/
SoTransferFunction::SoTransferFunction(void)
{
  PRIVATE(this) = new SoTransferFunctionP(this);

  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, NONE);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, GREY);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, GRAY);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, TEMPERATURE);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, PHYSICS);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, STANDARD);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, GLOW);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, BLUE_RED);
  SO_NODE_DEFINE_ENUM_VALUE(PredefColorMap, SEISMIC);

  SO_NODE_DEFINE_ENUM_VALUE(ColorMapType, ALPHA);
  SO_NODE_DEFINE_ENUM_VALUE(ColorMapType, LUM_ALPHA);
  SO_NODE_DEFINE_ENUM_VALUE(ColorMapType, RGBA);

  SO_NODE_ADD_FIELD(shift, (0));
  SO_NODE_ADD_FIELD(offset, (0));
  SO_NODE_ADD_FIELD(predefColorMap, (GREY));
  SO_NODE_ADD_FIELD(colorMapType, (RGBA));
}//Constructor




/*!
  Destructor.
*/
SoTransferFunction::~SoTransferFunction()
{
  delete PRIVATE(this);
}


// Doc from parent class.
void
SoTransferFunction::initClass(void)
{
  static int first = 0;
  if (first == 1) return;
  first = 1;

  SO_NODE_INIT_CLASS(SoTransferFunction, 
                     SoVolumeRendering, 
                     "TransferFunction");
  SoTransferFunctionElement::initClass();
}// initClass




/*!
  Coin method.
*/
void
SoTransferFunction::GLRender(SoGLRenderAction * action)
{
  SoTransferFunctionElement::setTransferFunction(action->getState(), 
                                                 this, 
                                                 this);
}// GLRender


/*
  Transfers input to output, according to specified parameters. 

*/
void
SoTransferFunction::transfer(const void * input, 
                             SoVolumeRendering::DataType inputDataType,
                             SbVec2s &size,
                             void *& output,
                             int &outputFormat,
                             float *& palette,
                             int &paletteFormat,
                             int &paletteSize)
{
  // Handling RGBA inputdata. Just forwarding to output
  if (inputDataType == SoVolumeRendering::RGBA) {
    outputFormat = GL_RGBA;
    palette = NULL;
    paletteFormat = 0;
    paletteSize = 0;

    output = new int[size[0]*size[1]];
    memcpy(output, input, size[0]*size[1]*sizeof(int));
  }//if

  // Handling paletted inputdata
  else {
    int i;

    int numBits;
    switch(inputDataType) {
      case SoVolumeRendering::UNSIGNED_BYTE: 
          numBits = 8; 
          break;

       case SoVolumeRendering::UNSIGNED_SHORT:
          numBits = 16;
          break;
    }// switch
    int numEntries = 1 << numBits;

    // Counting number of references to each palette entry     
    int * palCount = new int[numEntries];
    memset(palCount, 0, sizeof(int)*numEntries);
    for (i = 0; i < size[0]*size[1]; i++) {

      // FIXME: Test if the index is out of bounds. 08282002 torbjorv.
      int idx = 
        unsigned short((PRIVATE(this)->unpack(input, numBits, i) << shift.getValue()) + 
        offset.getValue());

      palCount[idx]++;
    }// for

    // Creating remap-table and counting number of entries in new palette
    int * remap = new int[numEntries];
    int newIdx = 0;
    for (i = 0; i < numEntries; i++) {
      if (palCount[i] != 0) {
        remap[i] = newIdx;
        newIdx++;
      }// if
      else
        remap[i] = -1;
    }// for

    // Calculating the new palette's size
    paletteSize = 2;
    while (paletteSize < newIdx)
      paletteSize <<= 1;


    // Building new palette
    palette = new float [paletteSize*4];
    memset(palette, 0, sizeof(float)*paletteSize*4);
    const float * oldPal = this->colorMap.getValues(0);
    int tmp = 0;
    for (i = 0; i < numEntries; i++) {
      if (palCount[i] != 0) {
        memcpy(&palette[tmp*4], &oldPal[i*4], sizeof(float)*4);
        tmp++;
      }// if
    }// for


    // Deciding outputformat
    int newNumBits = 8;
    outputFormat = UNSIGNED_BYTE;
    if (paletteSize > 256) {
      newNumBits = 16;
      outputFormat = UNSIGNED_SHORT;
    }// if

    // Rebuilding texturedata
    unsigned char * newTexture = 
      new unsigned char[size[0] * size[1] * newNumBits / 8];
    memset(newTexture, 0, size[0] * size[1] * newNumBits / 8);
    for (i = 0; i < size[0]*size[1]; i++) {
      int idx = 
        unsigned short((PRIVATE(this)->unpack(input, numBits, i) << shift.getValue()) + 
        offset.getValue());

      idx = remap[idx];
      PRIVATE(this)->pack(newTexture, newNumBits, i, idx);
    }// for
    output = newTexture;
    paletteFormat = GL_RGBA;

    delete [] palCount;
    delete [] remap;
  }//else
}// transfer




/*
  Handles packed data for 1, 2, 4, 8 and 16 bits. Assumes 16-bit 
  data are stored in little-endian format (that is the x86-way, 
  right?). And MSB is the leftmost of the bitstream...? Think so. 
*/
int 
SoTransferFunctionP::unpack(const void * data, int numBits, int index)
{
  if (numBits == 8)
    return (int)((char*)data)[index];

  if (numBits == 16)
    return (int)((short*)data)[index];


  // Handling 1, 2 and 4 bit formats
  int bitIndex = numBits*index;
  int byteIndex = bitIndex/8;
  int localBitIndex = 8 - bitIndex%8 - numBits;

  char val = ((char*)data)[byteIndex];
  val >>= localBitIndex;
  val &= (1 << numBits) - 1;

  return val;
}// unpack



/*
  Saves the index specified in val with the specified number of bits, 
  at the specified index (not elementindex, not byteindex) in data. 
*/
void 
SoTransferFunctionP::pack(void * data, int numBits, int index, int val)
{
  if (val >= (1 << numBits))
    val = (1 << numBits) - 1;

  if (numBits == 8) {
    ((char*)data)[index] = (char)val;
  }// if
  else 
  if (numBits == 16) {
    ((short*)data)[index] = (short)val;
  }// else
  else {
    int bitIndex = numBits*index;
    int byteIndex = bitIndex/8;
    int localBitIndex = 8 - bitIndex%8 - numBits;

    char byte = ((char*)data)[byteIndex];
    char mask = (((1 << numBits) - 1) << localBitIndex) ^ -1;
    byte &= mask;
    val <<= localBitIndex;
    byte |= val;
    ((char*)data)[byteIndex] = byte;
  }// else 
}// pack


// FIXME: Implement this function torbjorv 08282002
void SoTransferFunction::reMap(int min, int max) {}
