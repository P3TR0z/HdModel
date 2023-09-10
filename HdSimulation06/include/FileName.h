//
// File: FileName.h
// Created: 11 03 2023
// Project: HdSimulation06
// Author: Alexandru Petrosel
// Short description:
//
#pragma once

#include <vector>
#include "FileImage.h"

struct FileNameT
  : std::vector<char>
{
  FileNameT(size_t nameLength)
    : vector(nameLength)
  {
  }
  FileNameT(const char* name)
  {
    for (const char* c{ name }; 0 != *c; ++c)
      push_back(*c);
  }
  void Serialize(FileImageT& image) const
  {
    size_t length{ size() };
    image.Serialize(reinterpret_cast<DataUnitT&>(length), sizeof(length));
    for (auto& c : *this)
      image.Serialize((DataUnitT)(c));
  }
  void Deserialize(const FileImageT& image)
  {
    size_t length;
    image.Deserialize(reinterpret_cast<DataUnitT&>(length), sizeof(length));
    resize(length);
    for (auto& c : *this)
      image.Deserialize(reinterpret_cast<DataUnitT&>(c));
  }
  bool IsEqual(const char* name)
  {
    size_t i;
    for (i = 0; size() > i; ++i)
    {
      if (0 == name[i])
        return false;
      if (at(i) != name[i])
        return false;
    }
    return 0 == name[i];
  }
};
