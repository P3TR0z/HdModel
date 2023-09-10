//
// File: HdImage.cpp
// Created: 11 03 2023
// Project: HdSimulation06
// Author: Alexandru Petrosel
// Short description:
//
#include "include/HdImage.h"
#include "include/GeneralTypes.h"
#include "include/Mbr.h"
#include "include/Sector.h"
#include "include/FileRecord.h"
#include "include/MappingPairs.h"

HdImageT::HdImageT(HdModelT& hardDisk, unsigned char partitionNumber, ClusterT::LengthT sectorsPerCluster)
  : HdPartitionT(hardDisk)
{
  // Format()
  MbrT mbr;
  mbr.Read(hdModel);
  Set(mbr.partitions[partitionNumber].firstAsn, mbr.partitions[partitionNumber].numberOfSectors, sectorsPerCluster);
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
HdImageT::HdImageT(HdModelT& hardDisk, unsigned char partitionNumber)
  : HdPartitionT(hardDisk)
{
  // Open() 
  MbrT mbr;
  mbr.Read(hdModel);
  SectorT sector{ DataUnitT(0), hdModel.numberOfBytesPerSector };
  hdModel.ReadSector(sector, mbr.partitions[partitionNumber].firstAsn);
  FileImageT image{ sector };
  image.DeserializeSet();
  Deserialize(image);
  HdPartitionT::Set(mbr.partitions[partitionNumber].firstAsn, mbr.partitions[partitionNumber].numberOfSectors);
  mft.resize(size_t(mftLength), { { DataUnitT(0), hdModel.numberOfBytesPerSector }, numberOfSectorsPerCluster });
  bitMap.resize(size_t((numberOfClusters + BitMapT::unitLength - 1) / BitMapT::unitLength), BitMapUnitT(0));
  ReadCluster(mft.front(), lcnOfMft);
  ReadFile(mftName, image);
  image.DeserializeSet();
  mft.Deserialize(image);
  ReadFile(bitMapName, image);
  image.DeserializeSet();
  bitMap.Deserialize(image);
}
// Assumes file <constFileName> exists and isn't empty
// Always clears <fileImage> on return
void HdImageT::DestructorStubSaveFile(const char* constFileName, FileImageT& fileImage)
{
  FileRecordT fileRecord;
  mft.FindFile(constFileName, fileRecord);
  fileImage.insert(fileImage.end(), ((1 + ((fileImage.size() - 1) / numberOfBytesPerCluster)) * numberOfBytesPerCluster) - fileImage.size(), (DataUnitT)(0));
  ClusterT cluster{ { DataUnitT(0), hdModel.numberOfBytesPerSector }, numberOfSectorsPerCluster };
  long long firstLcn{ 0 };
  MappingPairsT::const_iterator i{ fileRecord.mappingPairs.begin() };
  fileImage.DeserializeSet();
  do
  {
    long long length;
    firstLcn += fileRecord.mappingPairs.GetDelta(length, i);
    long long lcn{ firstLcn };
    long long endLcn{ firstLcn + length };
    for (; endLcn > lcn; ++lcn)
    {
      cluster.Deserialize(fileImage);
      WriteCluster(lcn, cluster);
    }
  } while (0 != *i);
  fileImage.clear();
}
HdImageT::~HdImageT()
{
  FileImageT fileImage;
  mft.Serialize(fileImage);
  DestructorStubSaveFile(mftName, fileImage);
  bitMap.Serialize(fileImage);
  DestructorStubSaveFile(bitMapName, fileImage);
}
void HdImageT::Serialize(FileImageT& image) const
{
  image.Serialize(reinterpret_cast<const DataUnitT&>(numberOfSectorsPerCluster), sizeof(numberOfSectorsPerCluster));
  image.Serialize(reinterpret_cast<const DataUnitT&>(numberOfClusters), sizeof(numberOfClusters));
  image.Serialize(reinterpret_cast<const DataUnitT&>(numberOfBytesPerCluster), sizeof(numberOfBytesPerCluster));
  image.Serialize(reinterpret_cast<const DataUnitT&>(lcnOfMft), sizeof(lcnOfMft));
  image.Serialize(reinterpret_cast<const DataUnitT&>(mftLength), sizeof(mftLength));
}
void HdImageT::Deserialize(const FileImageT & image)
{
  image.Deserialize(reinterpret_cast<DataUnitT&>(numberOfSectorsPerCluster), sizeof(numberOfSectorsPerCluster));
  image.Deserialize(reinterpret_cast<DataUnitT&>(numberOfClusters), sizeof(numberOfClusters));
  image.Deserialize(reinterpret_cast<DataUnitT&>(numberOfBytesPerCluster), sizeof(numberOfBytesPerCluster));
  image.Deserialize(reinterpret_cast<DataUnitT&>(lcnOfMft), sizeof(lcnOfMft));
  image.Deserialize(reinterpret_cast<DataUnitT&>(mftLength), sizeof(mftLength));
}
HdImageT& HdImageT::Set(long long asnFirst, long long sectorsCount, ClusterT::LengthT sectorsPerCluster)
{
  HdPartitionT::Set(asnFirst, sectorsCount);
  numberOfSectorsPerCluster = sectorsPerCluster;
  numberOfClusters = numberOfSectors / sectorsPerCluster;
  numberOfBytesPerCluster = ((long long)(hdModel.numberOfBytesPerSector)) * sectorsPerCluster;
  mftLength = (numberOfClusters + 1) / 2;
  mft.resize(size_t(mftLength), { { DataUnitT(0), hdModel.numberOfBytesPerSector }, sectorsPerCluster });
  bitMap.resize(size_t((numberOfClusters + BitMapT::unitLength - 1) / BitMapT::unitLength), BitMapUnitT(0));
  return *this;
}
bool HdImageT::ReadFile(const char* fileName, FileImageT& fileImage)
{
  FileRecordT fileRecord;
  if (mft.end() == mft.FindFile(fileName, fileRecord))
    return false;
  fileImage.clear();
  if (0 == fileRecord.mappingPairs.front())
    return true;
  ClusterT cluster{ { DataUnitT(0), hdModel.numberOfBytesPerSector }, numberOfSectorsPerCluster };
  long long firstLcn{ 0 };
  MappingPairsT::const_iterator i{ fileRecord.mappingPairs.begin() };
  do
  {
    long long length;
    firstLcn += fileRecord.mappingPairs.GetDelta(length, i);
    long long lcn{ firstLcn };
    long long endLcn{ firstLcn + length };
    for (; endLcn > lcn; ++lcn)
    {
      if (!ReadCluster(cluster, lcn))
      {
        fileImage.clear();
        return false;
      }
      cluster.Serialize(fileImage);
    }
  } while (0 != *i);
  fileImage.erase(fileImage.end() - (fileImage.size() - (size_t)(fileRecord.numberOfBytes)), fileImage.end());
  return true;
}
bool HdImageT::WriteFile(const char* fileName, const FileImageT& fileImage)
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
  ClusterT cluster{ { DataUnitT(0), hdModel.numberOfBytesPerSector }, numberOfSectorsPerCluster };
  fileImage.DeserializeSet();
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
        cluster.Set(((long long)(fileImage.size())) - numberOfCompleteClusters * numberOfBytesPerCluster, hdModel.numberOfBytesPerSector);
        cluster.Deserialize(fileImage);
        if (!WriteCluster(lcn, cluster))
        {
          ClearClusters(fragments);
          return false;
        }
        fileRecord.Set(true, fileName, (long long)(fileImage.size()), fragments);
        return fileRecord.Write(*mftCluster);
      }
      cluster.Deserialize(fileImage);
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
bool HdImageT::DeleteFile(const char* fileName)
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
