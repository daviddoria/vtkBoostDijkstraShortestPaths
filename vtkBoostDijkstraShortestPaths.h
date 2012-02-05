/*=========================================================================
  
Program:   Visualization Toolkit
Module:    vtkBoostDijkstraShortestPaths.h

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
  -------------------------------------------------------------------------*/
// .NAME vtkBoostDijkstraShortestPaths - Computes the shortest path from a
//    specified vertex to all other vertices.
//
// .SECTION Description
//
// This VTK class uses the Boost Dijkstra Shortest Paths
// algorithm to compute the shortest path from a specified vertex
// to all other vertices in a graph.
//
// .SECTION See Also
// vtkGraph vtkBoostGraphAdapter

#ifndef __vtkBoostDijkstraShortestPaths_h
#define __vtkBoostDijkstraShortestPaths_h

#include "vtkStdString.h" // For string type
#include "vtkVariant.h" // For variant type

#include <boost/graph/adjacency_list.hpp>

#include "vtkGraphAlgorithm.h"

class VTK_INFOVIS_EXPORT vtkBoostDijkstraShortestPaths : public vtkGraphAlgorithm
{
public:
  static vtkBoostDijkstraShortestPaths *New();
  vtkTypeMacro(vtkBoostDijkstraShortestPaths, vtkGraphAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Set the name of the edge-weight input array, which must name an
  // array that is part of the edge data of the input graph and
  // contains numeric data. If the edge-weight array is not of type
  // vtkDoubleArray, the array will be copied into a temporary
  // vtkDoubleArray.
  vtkSetStringMacro(EdgeWeightArrayName);
  
  // Description:
  // Set the vertex from which to find shortest paths to
  // all vertices.
  vtkSetMacro(OriginVertexIndex, vtkIdType);

  // Description:
  // This method returns the vertices on the path from the source
  // to a specified destination.
  std::vector<unsigned int> GetPath(vtkIdType destination);

protected:
  vtkBoostDijkstraShortestPaths();
  ~vtkBoostDijkstraShortestPaths();

  int RequestData(
    vtkInformation *,
    vtkInformationVector **,
    vtkInformationVector *);

  int FillInputPortInformation(
    int port, vtkInformation* info);

private:
  char* EdgeWeightArrayName;
  vtkIdType OriginVertexIndex;

  typedef double WeightType;
  typedef boost::property<boost::edge_weight_t, WeightType> WeightProperty;

  typedef boost::adjacency_list < boost::listS, boost::vecS, boost::directedS,
    boost::no_property, WeightProperty > Graph;

  typedef boost::graph_traits < Graph >::vertex_descriptor Vertex;

  typedef boost::property_map < Graph, boost::vertex_index_t >::type IndexMap;

  typedef boost::iterator_property_map < Vertex*, IndexMap, Vertex, Vertex& > PredecessorMap;
  typedef boost::iterator_property_map < Weight*, IndexMap, Weight, Weight& > DistanceMap;

  struct PathType
  {
    Vertex Source;
    Vertex Desintation;
    WeightType Distance;
  };

  vtkBoostDijkstraShortestPaths(const vtkBoostDijkstraShortestPaths&);  // Not implemented.
  void operator=(const vtkBoostDijkstraShortestPaths&);  // Not implemented.
};

#endif
