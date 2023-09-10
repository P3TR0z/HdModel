#include "include/Sector.h"
#include <iostream>
int main()
{
  SectorT sector{DataUnitT(13), 512};
  DataUnitT a[10];
  sector.DeserializeSet();
  sector.Deserialize(reinterpret_cast<DataUnitT&>(a), sizeof(a)/sizeof(*a));
  for (size_t i = 0; i < 10; i++)
    std::cout << (int)(a[i]) << std::endl;
}
