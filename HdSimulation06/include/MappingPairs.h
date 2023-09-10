//
// File: MappingPairs.h
// Created: 11 03 2023
// Project: HdSimulation06
// Author: Alexandru Petrosel
// Short description:
//
#pragma once

#include <vector>
#include "GeneralTypes.h"
#include "FileImage.h"
#include "Fragment.h"

class MappingPairsT
  : public std::vector<DataUnitT>
{
public:
  MappingPairsT()
    : vector(DataUnitT(0))
  {
  }
  void Serialize(FileImageT& image) const
  {
    for (auto& mappingPairsUnit : *this)
      image.Serialize(mappingPairsUnit);
  }
  void Deserialize(const FileImageT& image)
  {
    clear();
    while (true)
    {
      push_back((DataUnitT)(0));
      image.Deserialize(back());
      if (0 == back())
        return;
      unsigned char deltaSize;
      unsigned char numberOfBytes{ GetSizes(deltaSize, back()) };
      numberOfBytes += deltaSize;
      insert(end(), size_t(numberOfBytes), DataUnitT(0));
      iterator mappingPair{ end() - numberOfBytes };
      for (; mappingPair != end(); mappingPair++)
        image.Deserialize(*mappingPair);
    }
  }
  MappingPairsT& Set(const vector<FragmentT>& fragments)
  {
    long long lcn{ 0 };   // also used as <delta>
    clear();
    for (const FragmentT& fragment : fragments)
    {
      size_t i{ size() };
      push_back(DataUnitT(0));
      // using 'lcn' as <delta>
      lcn = fragment.firstLcn - lcn;
      if (0 == lcn)
        if (0 == fragment.length)
          return *this;
      at(i) = (0x0f & PushValue(fragment.length)) | (0xf0 & (PushValue(lcn) << 4));
      // end <using 'lcn' as <delta>>
      lcn = fragment.firstLcn;
    }
    push_back(DataUnitT(0));
    return *this;
  }
  // returns <length size> and sets 'deltaSize' to the value of <delta size>
  unsigned char GetSizes(unsigned char& deltaSize, DataUnitT firstByte) const
  {
    deltaSize = 0x0f & (firstByte >> 4);
    return 0x0f & firstByte;
  }
  long long GetValue(unsigned char valueSize, const const_iterator& rightBeforeValue) const
  {
    if (0 == valueSize)
      return 0;
    long long value{ char(rightBeforeValue[valueSize]) };
    while (0 < --valueSize)
      value = (value << 8) | (rightBeforeValue[valueSize]);  // value = value * 256 + rightBeforeValue[valueSize]
    return value;
  }
  // returns <delta> and sets 'length' to the value of <length>
  long long GetDelta(long long& length, unsigned char deltaSize, unsigned char lengthSize, const const_iterator& mappingPair) const
  {
    length = GetValue(lengthSize, mappingPair);
    return GetValue(deltaSize, lengthSize + mappingPair);
  }
  // returns <delta>, sets 'length' to the value of <length> and sets 'mappingPair' to the <next record in mappingPairs>
  long long GetDelta(long long& length, const_iterator& mappingPair) const
  {
    unsigned char deltaSize;
    unsigned char lengthSize{ GetSizes(deltaSize, *mappingPair) };
    const_iterator prevMappingPair{ mappingPair };
    mappingPair += (size_t(1)) + lengthSize + deltaSize;
    return GetDelta(length, deltaSize, lengthSize, prevMappingPair);
  }
  // returns the minimum number of bytes needed to store 'value'
  unsigned char GetValueSize(long long value) const
  {
    if (0 == value)
      return 1;
    if (0 > value)
      value = ~value;
    char* valueByte{ reinterpret_cast<char*>(&value) };
    unsigned char i;
    for (i = sizeof(value) - 1; 0 == valueByte[i]; --i);
    return i + ((0 < valueByte[i]) ? 1 : 2);
  }
  // returns the number of bytes used to push 'value'
  unsigned char PushValue(long long value)
  {
    unsigned char size{ GetValueSize(value) };
    for (size_t i = 0; size > i; ++i)
      push_back((reinterpret_cast<unsigned char*>(&value))[i]);
    return size;
  }
};
