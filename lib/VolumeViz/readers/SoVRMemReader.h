#ifndef COIN_SOVRMEMREADER_H
#define COIN_SOVRMEMREADER_H

#include <VolumeViz/readers/SoVolumeReader.h>
#include <Inventor/SbBox3f.h>

class SoVRMemReader : public SoVolumeReader{
public:
  SoVRMemReader(void);
  virtual ~SoVRMemReader();

  void setUserData(void * data);

  void getDataChar(SbBox3f & size, SoVolumeData::DataType & type,
                   SbVec3s & dim);

  virtual void getSubSlice(SbBox2s & subslice, int slicenumber, void * data);

  void setData(const SbVec3s &dimensions, void * data,
               SoVolumeData::DataType type = SoVolumeData::UNSIGNED_BYTE);

private:
  friend class SoVRMemReaderP;
  class SoVRMemReaderP * pimpl;
};

#endif // !COIN_SOVOLUMEREADER_H
