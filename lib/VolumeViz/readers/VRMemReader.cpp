// FIXME: get rid of this class, as I don't think it adds anything of
// real value..? 20021126 mortene.

/*!
  \class SoVRMemReader VolumeViz/readers/SoVRMemReader.h
  \brief FIXME: doc
  \ingroup volviz
*/

// *************************************************************************

#include <VolumeViz/readers/SoVRMemReader.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>

#include <Inventor/errors/SoDebugError.h>

#include <string.h> // memcpy()


class SoVRMemReaderP {
public:
  SoVRMemReaderP(SoVRMemReader * master) {
    this->master = master;

    this->dimensions = SbVec3s(0, 0, 0);
    this->dataType = SoVolumeData::UNSIGNED_BYTE;
  }

  SbVec3s dimensions;
  SoVolumeData::DataType dataType;

private:
  SoVRMemReader * master;
};

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

SoVRMemReader::SoVRMemReader(void)
{
  PRIVATE(this) = new SoVRMemReaderP(this);

}

SoVRMemReader::~SoVRMemReader()
{
  delete PRIVATE(this);
}


void SoVRMemReader::setUserData(void * data)
{
}

void SoVRMemReader::getDataChar(SbBox3f & size,
                                SoVolumeData::DataType & type,
                                SbVec3s & dim)
{
  type = PRIVATE(this)->dataType;
  dim = PRIVATE(this)->dimensions;

  size.setBounds(-dim[0]/2.0f, -dim[1]/2.0f, -dim[2]/2.0f,
                 dim[0]/2.0f, dim[1]/2.0f, dim[2]/2.0f);
}

void
SoVRMemReader::getSubSlice(SbBox2s & subslice, int slicenumber, void * data)
{
  CvrVoxelChunk::UnitSize vctype;
  switch (PRIVATE(this)->dataType) {
  case SoVolumeData::UNSIGNED_BYTE: vctype = CvrVoxelChunk::UINT_8; break;
  case SoVolumeData::UNSIGNED_SHORT: vctype = CvrVoxelChunk::UINT_16; break;
  case SoVolumeData::RGBA: vctype = CvrVoxelChunk::UINT_32; break;
  default: assert(FALSE); break;
  }

  // FIXME: interface of buildSubPage() should be improved to avoid
  // this roundabout way of clipping out a slice.  20021203 mortene.
  CvrVoxelChunk vc(PRIVATE(this)->dimensions, vctype, this->m_data);
  CvrVoxelChunk * output = vc.buildSubPage(2 /* Z */, slicenumber, subslice);
  (void)memcpy(data, output->getBuffer(), output->bufferSize());
  delete output;
}


void
SoVRMemReader::setData(const SbVec3s &dimensions,
                       void * data,
                       SoVolumeData::DataType type)
{
  PRIVATE(this)->dimensions = dimensions;
  this->m_data = data;
  PRIVATE(this)->dataType = type;
}
