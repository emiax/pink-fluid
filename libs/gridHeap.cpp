#include <gridHeap.h>
#include <cassert>
#include <iostream>

GridHeap::GridHeap(unsigned int w, unsigned int h, OrdinalGrid<float> *cg) {
  capacity = w*h;
  size = 0;

  coordinates = new GridCoordinate[capacity];
  heapIndices = new Grid<int>(w, h);
  heapIndices->setForEach([&](unsigned int i, unsigned int j) {
      return NOT_IN_HEAP;
    });
  
  comparisonGrid = cg;
}

void GridHeap::insert(GridCoordinate coord) {
  int heapIndex = heapIndices->get(coord);

  if (heapIndex == NOT_IN_HEAP) {
    assert(size <= capacity);
    coordinates[size] = coord;
    heapIndices->set(coord, size);
    percolateUp(size);
    size++;
  } else {
    percolateUp(heapIndex);
    percolateDown(heapIndex);
  }
}

GridCoordinate GridHeap::pop() {
  GridCoordinate popped = coordinates[0];
  coordinates[0] = coordinates[size-1];
  size--;

  heapIndices->set(coordinates[0], 0);
  heapIndices->set(popped, NOT_IN_HEAP);

  percolateDown(0);

  return popped;
}

void GridHeap::percolateUp(int child) {

  GridCoordinate iCoord;
  GridCoordinate parentCoord;
  int parent;

  while (child != 0) {
    parent = (child - 1) / 2;
    iCoord = coordinates[child];
    parentCoord = coordinates[parent];

    if (comp(iCoord, parentCoord)) {
      
      coordinates[child] = parentCoord;
      coordinates[parent] = iCoord;
      heapIndices->set(coordinates[child], child);
      heapIndices->set(coordinates[parent], parent);

      child = parent;
    } else {
      break;
    }
  }
}

void GridHeap::percolateDown(int parent) {

  while (true) {
    int leftChild = parent*2 + 1;
    int rightChild = parent*2 + 2;

    if (leftChild >= size) break;

    int smallestChild = leftChild;
    if (rightChild < size) {
      GridCoordinate leftCoord = coordinates[leftChild];
      GridCoordinate rightCoord = coordinates[rightChild];
      
      if (comp(rightCoord, leftCoord)) smallestChild = rightChild;
    }

    GridCoordinate childCoord = coordinates[smallestChild];
    GridCoordinate parentCoord = coordinates[parent];

    if (comp(childCoord, parentCoord)) {
      coordinates[smallestChild] = parentCoord;
      coordinates[parent] = childCoord;
      heapIndices->set(coordinates[smallestChild], smallestChild);
      heapIndices->set(coordinates[parent], parent);

      parent = smallestChild;
    } else {
      break;
    }
  }
}

bool GridHeap::comp(GridCoordinate &a, GridCoordinate &b) {
  return (comparisonGrid->get(a) < comparisonGrid->get(b));
}

bool GridHeap::empty() {
  return size == 0;
}

void GridHeap::clear() {
  size = 0;
}
