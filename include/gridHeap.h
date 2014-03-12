#pragma once

#include <grid.h>
#include <ordinalGrid.h>
#include <functional>

class GridHeap
{
public:

  GridHeap(unsigned int w, unsigned int h, unsigned int d, OrdinalGrid<float> *cg);
  ~GridHeap();

  void insert(GridCoordinate coord);
  GridCoordinate pop();

  bool empty();
  void clear();

private:
  void percolateUp(int);
  void percolateDown(int);
  bool comp(GridCoordinate &a, GridCoordinate &b);

  static constexpr int NOT_IN_HEAP = -1;

  GridCoordinate *coordinates;
  OrdinalGrid<float> *comparisonGrid;
  Grid<int> *heapIndices;
  unsigned int capacity, size;

};
