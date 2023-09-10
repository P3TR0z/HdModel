//
// File: BitMap.h
// Created: 11 03 2023
// Project: HdSimulation06
// Author: Alexandru Petrosel
// Short description:
//
#pragma once

#include <vector>
#include "FileImage.h"

using BitMapUnitT = unsigned long long;

class BitMapT
  : public std::vector<BitMapUnitT>
{
public:
  static const size_t unitLength{ 8 * sizeof(BitMapUnitT) };
  BitMapT() = default;
  BitMapT(long long numberOfClusters)
    : vector(size_t((numberOfClusters + unitLength - 1) / unitLength), BitMapUnitT(0))
  {
  }
  void Serialize(FileImageT& image) const
  {
    for (auto& bitMapUnit : *this)
      image.Serialize(reinterpret_cast<const DataUnitT&>(bitMapUnit), sizeof(bitMapUnit));
  }
  void Deserialize(const FileImageT& image)
  {
    for (auto& bitMapUnit : *this)
      image.Deserialize(reinterpret_cast<DataUnitT&>(bitMapUnit), sizeof(bitMapUnit));
  }
  // returns <cluster's <bit map unit> index> and sets 'mask' to the value of <cluster's mask inside <bit map unit>>
  size_t GetCluster(BitMapUnitT& mask, long long lcn) const
  {
    size_t bitMapIndex{ size_t(lcn / unitLength) };
    mask = (BitMapUnitT(1)) << (lcn - ((long long)(bitMapIndex)) * unitLength);
    return bitMapIndex;
  }
  bool IsClusterFree(size_t bitMapIndex, BitMapUnitT mask) const
  {
    return 0 == (mask & at(bitMapIndex));
  }
  bool IsClusterFree(long long lcn) const
  {
    BitMapUnitT mask;
    size_t bitMapIndex{ GetCluster(mask, lcn) };
    return IsClusterFree(bitMapIndex, mask);
  }
  void SetClusterFree(size_t bitMapIndex, BitMapUnitT mask)
  {
    at(bitMapIndex) &= ~mask;
  }
  void SetClusterFree(long long lcn)
  {
    BitMapUnitT mask;
    size_t bitMapIndex{ GetCluster(mask, lcn) };
    SetClusterFree(bitMapIndex, mask);
  }
  void SetClusterNotFree(size_t bitMapIndex, BitMapUnitT mask)
  {
    at(bitMapIndex) |= mask;
  }
  void SetClusterNotFree(long long lcn)
  {
    BitMapUnitT mask;
    size_t bitMapIndex{ GetCluster(mask, lcn) };
    SetClusterNotFree(bitMapIndex, mask);
  }
};
