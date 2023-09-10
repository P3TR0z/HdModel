//
// File: HdImage.h
// Created: 11 03 2023
// Project: HdSimulation06
// Author: Alexandru Petrosel
// Short description:
//
#pragma once

#include <vector>
#include "HdPartition.h"
#include "Cluster.h"
#include "Mft.h"
#include "BitMap.h"
#include "HdModel.h"
#include "FileImage.h"
#include "Fragment.h"

class HdImageT
  : public HdPartitionT
{
  ClusterT::LengthT numberOfSectorsPerCluster{ ClusterT::LengthT(0) };
  long long numberOfClusters{ 0 };
  long long numberOfBytesPerCluster{ 0 };
  long long lcnOfMft{ 0 };
  MftT mft;  long long mftLength{ 0 };
  BitMapT bitMap;
  const char* const name{ "$HdImage" };
  const char* const mftName{ "$MFT" };
  const char* const bitMapName{ "$BitMap" };
  //
  void DestructorStubSaveFile(const char* constFileName, FileImageT& fileImage);
public:
  HdImageT(HdModelT& hardDisk, unsigned char partitionNumber, ClusterT::LengthT sectorsPerCluster);
  HdImageT(HdModelT& hardDisk, unsigned char partitonNUmber);
  ~HdImageT();
  //
  void Serialize(FileImageT& image) const;
  void Deserialize(const FileImageT& image);
  HdImageT& Set(long long lsnZero, long long sectorsCount, ClusterT::LengthT sectorsPerCluster);
  MftT& Mft()
  {
    return mft;
  }
  bool ReadCluster(ClusterT& virtualCluster, long long lcn) const
  {
    long long lsn{ lcn * numberOfSectorsPerCluster };
    for (size_t sectorIndex{ 0 }; virtualCluster.size() > sectorIndex; ++sectorIndex)
      if (!ReadSector(virtualCluster[sectorIndex], lsn + sectorIndex))
        return false;
    return true;
  }
  bool WriteCluster(long long lcn, const ClusterT& virtualCluster) const
  {
    long long lsn{ lcn * numberOfSectorsPerCluster };
    for (size_t sectorIndex{ 0 }; virtualCluster.size() > sectorIndex; ++sectorIndex)
      if (!WriteSector(lsn + sectorIndex, virtualCluster[sectorIndex]))
        return false;
    return true;
  }
  void ClearClusters(const std::vector<FragmentT>& fragments)
  {
    for (const FragmentT& fragment : fragments)
      for (long long i{ fragment.firstLcn }; fragment.firstLcn + fragment.length > i; ++i)
        bitMap.SetClusterFree(i);
  }
  long long GetVcn(long long lcn, const std::vector<FragmentT>& fragments) const
  {
    long long vcn{ 0 };
    for (const FragmentT& fragment : fragments)
    {
      if (lcn >= fragment.firstLcn)
        if (lcn < fragment.firstLcn + fragment.length)
          return (vcn + (lcn - fragment.firstLcn));
      vcn += fragment.length;
    }
    return -1;
  }
  long long GetVsn(long long lsn, const std::vector<FragmentT>& fragments) const
  {
    long long lcn{ lsn / numberOfSectorsPerCluster };
    long long vcn{ GetVcn(lcn, fragments) };
    return (-1 == vcn) ? -1 : ((lsn + (vcn - lcn) * numberOfSectorsPerCluster));  // vcn * numberOfSectorsPerCluster + (lsn % numberOfSectorsPerCluster);
  }
  long long GetLcn(long long vcn, const std::vector<FragmentT>& fragments) const
  {
    for (const FragmentT& fragment : fragments)
    {
      if (vcn < fragment.length)
        return vcn + fragment.firstLcn;
      vcn -= fragment.length;
    }
    return -1;
  }
  long long GetLsn(long long vsn, const std::vector<FragmentT>& fragments) const
  {
    long long vcn{ vsn / numberOfSectorsPerCluster };
    long long lcn{ GetLcn(vcn, fragments) };
    return (-1 == lcn) ? -1 : ((vsn + (lcn - vcn) * numberOfSectorsPerCluster));  // lcn * numberOfSectorsPerCluster + (vsn % numberOfSectorsPerCluster);
  }
  bool ReadFile(const char* fileName, FileImageT& fileImage);
  bool WriteFile(const char* fileName, const FileImageT& fileImage);
  bool DeleteFile(const char* fileName);
};
