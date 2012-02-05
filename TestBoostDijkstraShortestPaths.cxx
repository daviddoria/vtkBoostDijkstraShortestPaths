#include <vtkDataSetAttributes.h>
#include <vtkDoubleArray.h>
#include <vtkMutableUndirectedGraph.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>

#include "vtkBoostDijkstraShortestPaths.h"

int TestBoostDijkstraShortestPaths(int, char *[])
{
  // Create a graph
  vtkSmartPointer<vtkMutableUndirectedGraph> g =
    vtkSmartPointer<vtkMutableUndirectedGraph>::New();

  /*    v0
         .
        / \ 2
       /   \
      /     . v2
    5/       \
    /         \ 4
   /           \
  v1----------- v3
      3
  */

  // Create vertices
  vtkIdType v1 = g->AddVertex();
  vtkIdType v2 = g->AddVertex();
  vtkIdType v3 = g->AddVertex();

  // Create edges
  g->AddEdge(v0, v1);
  g->AddEdge(v1, v3);
  g->AddEdge(v0, v2);
  g->AddEdge(v2, v3);

  // Create the edge weight array
  vtkSmartPointer<vtkDoubleArray> weights =
    vtkSmartPointer<vtkDoubleArray>::New();
  weights->SetNumberOfComponents(1);
  weights->SetName("Weights");

  // Set the edge weights
  weights->InsertNextValue(5.0);
  weights->InsertNextValue(3.0);
  weights->InsertNextValue(2.0);
  weights->InsertNextValue(4.0);

  // Compute the shortest paths
  vtkSmartPointer<vtkBoostDijkstraShortestPaths> filter =
    vtkSmartPointer<vtkBoostDijkstraShortestPaths>::New();
  filter->SetInputConnection(g->GetProducerPort());
  filter->Update();

  if(filter->GetOutput()->GetNumberOfVertices() != 3)
    {
    std::cout << "Size of largest connected component: " << filter->GetOutput()->GetNumberOfVertices()
              << " (Should have been 3)." << std::endl;
    return EXIT_FAILURE;
    }

  if(results[i] == EXIT_SUCCESS)
    {
    std::cout << "Test " << i << " passed." << std::endl;
    }
  else
    {
    std::cout << "Test " << i << " failed!" << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
