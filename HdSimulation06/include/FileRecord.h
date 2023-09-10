//
// File: FileRecord.h
// Created: 11 03 2023
// Project: HdSimulation06
// Author: Alexandru Petrosel
// Short description:
//
#pragma once

#include <vector>
#include "FileName.h"
#include "MappingPairs.h"
#include "Fragment.h"
#include "FileImage.h"
#include "Cluster.h"

struct FileRecordT
{
  bool recordInUse{ false };
  FileNameT name{ size_t(0) };
  long long numberOfBytes{ 0 };
  MappingPairsT mappingPairs;
  FileRecordT() = default;
  FileRecordT(bool inUse, const char* fileName, long long bytesCount, std::vector<FragmentT>& fragments)
    : recordInUse{ inUse }
    , name{ fileName }
    , numberOfBytes{ bytesCount }
  {
    mappingPairs.Set(fragments);
  }
  void Serialize(FileImageT& image) const
  {
    image.Serialize((DataUnitT)(recordInUse));
    name.Serialize(image);
    image.Serialize(reinterpret_cast<const DataUnitT&>(numberOfBytes), sizeof(numberOfBytes));
    mappingPairs.Serialize(image);
  }
  void Deserialize(const FileImageT& image) 
  {
    image.Deserialize(reinterpret_cast<DataUnitT&>(recordInUse));
    name.Deserialize(image);
    image.Deserialize(reinterpret_cast<DataUnitT&>(numberOfBytes), sizeof(numberOfBytes));
    mappingPairs.Deserialize(image);
  }
  FileRecordT& Set(bool inUse, const char* fileName, long long bytesCount, std::vector<FragmentT>& fragments)
  {
    recordInUse = inUse;
    name = fileName;
    numberOfBytes = bytesCount;
    mappingPairs.Set(fragments);
    return *this;
  }
  bool Read(const ClusterT& cluster)
  {
    FileImageT image;
    cluster.Serialize(image);
    image.DeserializeSet();
    Deserialize(image);
    return true;
  }
  // assumes 'cluster' has enough room to store <'*this' serialized>
  bool Write(ClusterT& cluster) const
  {
    FileImageT image;
    Serialize(image);
    image.insert(image.end(), (cluster.size() - 1) * cluster.front().size() + cluster.back().size() - image.size(), DataUnitT(0));
    image.DeserializeSet();
    cluster.Deserialize(image);
    return true;
  }
};
