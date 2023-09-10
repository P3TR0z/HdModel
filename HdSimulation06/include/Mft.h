//
// File: Mft.h
// Created: 11 03 2023
// Project: HdSimulation06
// Author: Alexandru Petrosel
// Short description:
//
#pragma once

#include <vector>
#include "Cluster.h"
#include "Sector.h"
#include "FileImage.h"
#include "FileRecord.h"

class MftT
  : public std::vector<ClusterT>
{
public:
  MftT() = default;
  MftT(long long numberOfClusters, ClusterT::LengthT numberOfSectorsPerCluster, SectorT::LengthT numberOfBytesPerSector)
    : vector(size_t(numberOfClusters), { { DataUnitT(0), numberOfBytesPerSector }, numberOfSectorsPerCluster })
  {
  }
  void Serialize(FileImageT& image) const
  {
    for (auto& cluster : *this)
      cluster.Serialize(image);
  }
  void Deserialize(const FileImageT& image)
  {
    for (auto& cluster : *this)
      cluster.Deserialize(image);
  }
  iterator FindFreeRecord()
  {
    FileRecordT mftRecord;
    iterator mftCluster{ begin() };
    for (; end() != mftCluster; ++mftCluster)
    {
      mftRecord.Read(*mftCluster);
      if (!mftRecord.recordInUse)
        return mftCluster;
    }
    return end();
  }
  iterator FindFile(const char* fileName, /* output */ FileRecordT& mftRecord)
  {
    iterator mftCluster{ begin() };
    for (; end() != mftCluster; ++mftCluster)
    {
      mftRecord.Read(*mftCluster);
      if (mftRecord.recordInUse)
        if (mftRecord.name.IsEqual(fileName))
          return mftCluster;
    }
    return end();
  }
}; // deber hd image (cum sa mearga, modificat in alt fisier daca modific cv)
