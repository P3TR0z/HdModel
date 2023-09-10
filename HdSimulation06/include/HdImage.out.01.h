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
#include "Mbr.h"
#include "Sector.h"
#include "FileRecord.h"
#include "Fragment.h"
#include "MappingPairs.h"

class HdImageT
  : public HdPartitionT
{
public:
  ClusterT::LengthT numberOfSectorsPerCluster{ ClusterT::LengthT(0) };
  long long numberOfClusters{ 0 };
  long long numberOfBytesPerCluster{ 0 };
  long long lcnOfMft{ 0 };
  MftT mft;
  long long mftLength{ 0 };
  BitMapT bitMap;
  const char* const name{ "$HdImage" };
  const char* const mftName{ "$MFT" };
  const char* const bitMapName{ "$BitMap" };
  HdImageT() = default;
  HdImageT(HdModelT& hardDisk, long long lsnZero, long long sectorsCount, ClusterT::LengthT sectorsPerCluster)
    : HdPartitionT(hardDisk, lsnZero, sectorsCount)
    , numberOfSectorsPerCluster{ sectorsPerCluster }
    , numberOfClusters{ sectorsCount / sectorsPerCluster }
    , numberOfBytesPerCluster{ ((long long)(hardDisk.numberOfBytesPerSector)) * sectorsPerCluster }
    , mftLength{ (1 + (sectorsCount / sectorsPerCluster)) / 2 }
    , mft{ size_t((1 + (sectorsCount / sectorsPerCluster)) / 2), sectorsPerCluster, hardDisk.numberOfBytesPerSector }
    , bitMap{ sectorsCount / sectorsPerCluster }
  {
  }
  void Serialize(FileImageT& image) const
  {
    image.Serialize(reinterpret_cast<const DataUnitT&>(numberOfSectorsPerCluster), sizeof(numberOfSectorsPerCluster));
    image.Serialize(reinterpret_cast<const DataUnitT&>(numberOfClusters), sizeof(numberOfClusters));
    image.Serialize(reinterpret_cast<const DataUnitT&>(numberOfBytesPerCluster), sizeof(numberOfBytesPerCluster));
    image.Serialize(reinterpret_cast<const DataUnitT&>(lcnOfMft), sizeof(lcnOfMft));
    image.Serialize(reinterpret_cast<const DataUnitT&>(mftLength), sizeof(mftLength));
  }
  void Deserialize(FileImageT::const_iterator& dataUnit)
  {
    FileImageT::Deserialize(reinterpret_cast<DataUnitT&>(numberOfSectorsPerCluster), sizeof(numberOfSectorsPerCluster), dataUnit);
    FileImageT::Deserialize(reinterpret_cast<DataUnitT&>(numberOfClusters), sizeof(numberOfClusters), dataUnit);
    FileImageT::Deserialize(reinterpret_cast<DataUnitT&>(numberOfBytesPerCluster), sizeof(numberOfBytesPerCluster), dataUnit);
    FileImageT::Deserialize(reinterpret_cast<DataUnitT&>(lcnOfMft), sizeof(lcnOfMft), dataUnit);
    FileImageT::Deserialize(reinterpret_cast<DataUnitT&>(mftLength), sizeof(mftLength), dataUnit);
  }
  HdImageT& Set(HdModelT& hardDisk, long long lsnZero, long long sectorsCount, ClusterT::LengthT sectorsPerCluster)
  {
    HdPartitionT::Set(hardDisk, lsnZero, sectorsCount);
    numberOfSectorsPerCluster = sectorsPerCluster;
    numberOfClusters = numberOfSectors / sectorsPerCluster;
    numberOfBytesPerCluster = ((long long)(hardDisk.numberOfBytesPerSector)) * sectorsPerCluster;
    mftLength = (numberOfClusters + 1) / 2;
    mft.resize(size_t(mftLength), { { DataUnitT(0), hardDisk.numberOfBytesPerSector }, sectorsPerCluster });
    bitMap.resize(size_t((numberOfClusters + BitMapT::unitLength - 1) / BitMapT::unitLength), BitMapUnitT(0));
    return *this;
  }
  void Format(HdModelT& hardDisk, unsigned char partitionNumber, ClusterT::LengthT sectorsPerCluster)
  {
    MbrT mbr;
    mbr.Read(hardDisk);
    Set(hardDisk, mbr.partitions[partitionNumber].firstAsn, mbr.partitions[partitionNumber].numberOfSectors, sectorsPerCluster);
    FileImageT image;
    Serialize(image);
    lcnOfMft = (image.size() + numberOfBytesPerCluster - 1) / numberOfBytesPerCluster;
    image.clear();
    bitMap.Serialize(image);
    lcnOfMft += (image.size() + numberOfBytesPerCluster - 1) / numberOfBytesPerCluster;
    for (long long lcn{ 0 }; lcnOfMft > lcn; ++lcn)
      bitMap.SetClusterNotFree(lcn);
    image.clear();
    mft.Serialize(image);
    WriteFile(mftName, image);
    for (long long lcn{ 0 }; lcnOfMft > lcn; ++lcn)
      bitMap.SetClusterFree(lcn);
    image.clear();
    Serialize(image);
    WriteFile(name, image);
    image.clear();
    bitMap.Serialize(image);
    WriteFile(bitMapName, image);
  }
  void Open(HdModelT& hardDisk, unsigned char partitionNumber)
  {
    MbrT mbr;
    mbr.Read(hardDisk);
    SectorT sector{ DataUnitT(0), hardDisk.numberOfBytesPerSector };
    hardDisk.ReadSector(sector, mbr.partitions[partitionNumber].firstAsn);
    FileImageT image{ sector };
    FileImageT::iterator i{ image.begin() };
    Deserialize(i);
    HdPartitionT::Set(hardDisk, mbr.partitions[partitionNumber].firstAsn, mbr.partitions[partitionNumber].numberOfSectors);
    mft.resize(size_t(mftLength), { { DataUnitT(0), hdModel->numberOfBytesPerSector }, numberOfSectorsPerCluster });
    bitMap.resize(size_t((numberOfClusters + BitMapT::unitLength - 1) / BitMapT::unitLength), BitMapUnitT(0));
    ReadCluster(mft.front(), lcnOfMft);
    ReadFile(mftName, image);
    i = image.begin();
    mft.Deserialize(i);
    ReadFile(bitMapName, image);
    i = image.begin();
    bitMap.Deserialize(i);
  }
  void Close()
  {
    FileRecordT fileRecord;
    mft.FindFile(mftName, fileRecord);
    FileImageT fileImage;
    mft.Serialize(fileImage);
    CloseStubSaveFile(fileImage, fileRecord.mappingPairs);
    mft.FindFile(bitMapName, fileRecord);
    fileImage.clear();
    bitMap.Serialize(fileImage);
    CloseStubSaveFile(fileImage, fileRecord.mappingPairs);
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
  bool ReadFile(const char* fileName, FileImageT& fileImage)
  {
    FileRecordT fileRecord;
    if (mft.end() == mft.FindFile(fileName, fileRecord))
      return false;
    fileImage.clear();
    if (0 == fileRecord.mappingPairs.front())
      return true;
    ClusterT cluster{ { DataUnitT(0), hdModel->numberOfBytesPerSector }, numberOfSectorsPerCluster };
    long long firstLcn{ 0 };
    MappingPairsT::const_iterator i{ fileRecord.mappingPairs.begin() };
    while (true)
    {
      long long length;
      firstLcn += fileRecord.mappingPairs.GetDelta(length, i);
      long long lcn{ firstLcn };
      long long endLcn{ firstLcn + length };
      if (0 == *i)
      {
        for (--endLcn; endLcn > lcn; ++lcn)
          if (!ReadFileStubReadCluster(fileImage, lcn, cluster))
            return false;
        cluster.Set(fileRecord.numberOfBytes - ((long long)(fileImage.size())), hdModel->numberOfBytesPerSector);
        return ReadFileStubReadCluster(fileImage, lcn, cluster);
      }
      for (; endLcn > lcn; ++lcn)
        if (!ReadFileStubReadCluster(fileImage, lcn, cluster))
          return false;
    }
  }
  bool WriteFile(const char* fileName, const FileImageT& fileImage)
  {
    MftT::iterator mftCluster{ mft.FindFreeRecord() };
    if (mft.end() == mftCluster)
      return false;
    FileRecordT fileRecord;
    std::vector<FragmentT> fragments;
    if (0 == fileImage.size())
    {
      fileRecord.Set(true, fileName, (long long)(0), fragments);
      return fileRecord.Write(*mftCluster);
    }
    long long lcn{ 0 };
    long long lcnPrev{ -1 };
    long long numberOfCompleteClusters{ (((long long)(fileImage.size())) - 1) / numberOfBytesPerCluster };
    long long numberOfClustersWritten{ 0 };
    ClusterT cluster{ { DataUnitT(0), hdModel->numberOfBytesPerSector }, numberOfSectorsPerCluster };
    FileImageT::const_iterator i{ fileImage.begin() };
    while (numberOfClusters > lcn)
    {
      BitMapUnitT mask;
      size_t bitMapIndex{ bitMap.GetCluster(mask, lcn) };
      if (bitMap.IsClusterFree(bitMapIndex, mask))
      {
        bitMap.SetClusterNotFree(bitMapIndex, mask);
        if (lcnPrev == lcn)
          fragments.back().length++;
        else
          fragments.push_back({ lcn, 1 });
        lcnPrev = lcn + 1;
        if (numberOfCompleteClusters == numberOfClustersWritten)
        {
          cluster.Set(((long long)(fileImage.size())) - numberOfCompleteClusters * numberOfBytesPerCluster, hdModel->numberOfBytesPerSector);
          cluster.Deserialize(i);
          if (!WriteCluster(lcn, cluster))
          {
            ClearClusters(fragments);
            return false;
          }
          fileRecord.Set(true, fileName, (long long)(fileImage.size()), fragments);
          return fileRecord.Write(*mftCluster);
        }
        cluster.Deserialize(i);
        if (!WriteCluster(lcn, cluster))
        {
          ClearClusters(fragments);
          return false;
        }
        ++numberOfClustersWritten;
        lcn = lcnPrev;
      }
      else
        ++lcn;
    }
    ClearClusters(fragments);
    return false;
  }
  bool DeleteFile(const char* fileName)
  {
    FileRecordT fileRecord;
    MftT::iterator mftCluster{ mft.FindFile(fileName, fileRecord) };
    if (mft.end() == mftCluster)
      return true;
    long long firstLcn{ 0 };
    MappingPairsT::const_iterator i{ fileRecord.mappingPairs.begin() };
    while (0 != *i)
    {
      long long length;
      firstLcn += fileRecord.mappingPairs.GetDelta(length, i);
      ClearClusters({ { firstLcn, length } });
    }
    fileRecord.recordInUse = false;
    return fileRecord.Write(*mftCluster);
  }
private:
  void CloseStubSaveFile(const FileImageT& fileImage, const MappingPairsT& mappingPairs) const
  {
    FileImageT::const_iterator dataUnit{ fileImage.begin() };
    ClusterT cluster{ { DataUnitT(0), hdModel->numberOfBytesPerSector }, numberOfSectorsPerCluster };
    long long firstLcn{ 0 };
    MappingPairsT::const_iterator i{ mappingPairs.begin() };
    while (true)
    {
      long long length;
      firstLcn += mappingPairs.GetDelta(length, i);
      long long lcn{ firstLcn };
      long long endLcn{ firstLcn + length };
      if (0 == *i)
      {
        for (--endLcn; endLcn > lcn; ++lcn)
        {
          cluster.Deserialize(dataUnit);
          WriteCluster(lcn, cluster);
        }
        cluster.Set(fileImage.end() - dataUnit, hdModel->numberOfBytesPerSector);
        cluster.Deserialize(dataUnit);
        WriteCluster(lcn, cluster);
        return;
      }
      for (; endLcn > lcn; ++lcn)
      {
        cluster.Deserialize(dataUnit);
        WriteCluster(lcn, cluster);
      }
    }
  }
  bool ReadFileStubReadCluster(FileImageT& fileImage, long long lcn, ClusterT& cluster) const
  {
    if (ReadCluster(cluster, lcn))
    {
      cluster.Serialize(fileImage);
      return true;
    }
    fileImage.clear();
    return false;
  }
};
