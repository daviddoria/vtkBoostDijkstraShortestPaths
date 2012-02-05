/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkBoostDijkstraShortestPaths.cxx

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
#include "vtkBoostDijkstraShortestPaths.h"

#include "vtkBoostGraphAdapter.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkMath.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkFloatArray.h"
#include "vtkDataArray.h"
#include "vtkSelection.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkMutableDirectedGraph.h"
#include "vtkUndirectedGraph.h"
#include "vtkTree.h"

#include <boost/graph/dijkstra_shortest_path.hpp>

using namespace boost;

vtkStandardNewMacro(vtkBoostDijkstraShortestPaths);

// Constructor/Destructor
vtkBoostDijkstraShortestPaths::vtkBoostDijkstraShortestPaths()
{
  this->EdgeWeightArrayName = NULL;
  this->OriginVertexIndex = 0;
}

//----------------------------------------------------------------------------
vtkBoostDijkstraShortestPaths::~vtkBoostDijkstraShortestPaths()
{
  this->SetEdgeWeightArrayName(NULL);
}

//----------------------------------------------------------------------------
int vtkBoostDijkstraShortestPaths::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkGraph *input = vtkGraph::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  // Retrieve the edge-weight array.
  if (!this->EdgeWeightArrayName)
    {
    vtkErrorMacro("Edge-weight array name is required");
    return 0;
    }
  vtkDataArray* edgeWeightArray = input->GetEdgeData()->GetArray(this->EdgeWeightArrayName);

  // Does the edge-weight array exist at all?
  if (edgeWeightArray == NULL)
    {
    vtkErrorMacro("Could not find edge-weight array named "
                  << this->EdgeWeightArrayName);
    return 0;
    }

  // Create the mutable graph to build the tree
  vtkSmartPointer<vtkMutableDirectedGraph> temp =
    vtkSmartPointer<vtkMutableDirectedGraph>::New();

  // Initialize copying data into tree
  temp->GetFieldData()->PassData(input->GetFieldData());
  temp->GetVertexData()->PassData(input->GetVertexData());
  temp->GetPoints()->ShallowCopy(input->GetPoints());
  //FIXME - We're not copying the edge data, see FIXME note below.
  //  temp->GetEdgeData()->CopyAllocate(input->GetEdgeData());

  // Create things for Dijkstra
  std::vector<Vertex> predecessors(boost::num_vertices(g)); // To store parents
  std::vector<Weight> distances(boost::num_vertices(g)); // To store distances

  IndexMap indexMap = boost::get(boost::vertex_index, g);
  PredecessorMap predecessorMap(&predecessors[0], indexMap);
  DistanceMap distanceMap(&distances[0], indexMap);

  // Compute shortest paths from v0 to all vertices, and store the output in predecessors and distances
  boost::dijkstra_shortest_paths(g, v0, boost::predecessor_map(predecessorMap).distance_map(distanceMap));
  
  // Run the algorithm
  if (vtkDirectedGraph::SafeDownCast(input))
    {
    vtkDirectedGraph *g = vtkDirectedGraph::SafeDownCast(input);
    prim_minimum_spanning_tree(g, predecessorMap, weight_map(weight_helper).root_vertex(this->OriginVertexIndex) );
    }
  else
    {
    vtkUndirectedGraph *g = vtkUndirectedGraph::SafeDownCast(input);
    prim_minimum_spanning_tree(g, predecessorMap, weight_map(weight_helper).root_vertex(this->OriginVertexIndex) );
    }

  vtkIdType i;
  if ( temp->SetNumberOfVertices( input->GetNumberOfVertices() ) < 0 )
    { // The graph must be distributed.
    vtkErrorMacro( "Prim MST algorithm will not work on distributed graphs." );
    return 0;
    }
  for( i = 0; i < temp->GetNumberOfVertices(); i++ )
    {
    if( predecessorMap->GetValue(i) == i )
      {
      if( i == this->OriginVertexIndex )
        {
        continue;
        }
      else
        {
        vtkErrorMacro("Unexpected result: MST is a forest (collection of trees).");
        return 0;
        }
      }

    vtkEdgeType tree_e = temp->AddEdge( predecessorMap->GetValue(i), i );

    //FIXME - We're not copying the edge data from the graph to the MST because
    //  of the ambiguity associated with copying data when parallel edges between
    //  vertices in the original graph exist.
    //    temp->GetEdgeData()->CopyData(input->GetEdgeData(), e.Id, tree_e.Id);
    }

  if (this->CreateGraphVertexIdArray)
    {
    predecessorMap->SetName("predecessorMap");
    temp->GetVertexData()->AddArray(predecessorMap);
    }

  // Copy the builder graph structure into the output tree
  vtkTree *output = vtkTree::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!output->CheckedShallowCopy(temp))
    {
    vtkErrorMacro(<<"Invalid tree.");
    return 0;
    }

  predecessorMap->Delete();

  return 1;
}

//----------------------------------------------------------------------------
int vtkBoostDijkstraShortestPaths::FillInputPortInformation(
  int port, vtkInformation* info)
{
  // now add our info
  if (port == 0)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkGraph");
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkBoostDijkstraShortestPaths::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "OriginVertexIndex: " << this->OriginVertexIndex << endl;

  os << indent << "EdgeWeightArrayName: "
     << (this->EdgeWeightArrayName ? this->EdgeWeightArrayName : "(none)")
     << endl;
}
