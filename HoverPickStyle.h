#pragma once

#include <vtkActor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>

// Handle mouse events
class HoverPickStyle : public vtkInteractorStyleTrackballCamera {
public:
  static HoverPickStyle *New();
  vtkTypeMacro(HoverPickStyle, vtkInteractorStyleTrackballCamera);

  void SetDatasets(vtkPartitionedDataSetCollection *datasets);

  void Activate(vtkRenderer *renderer);
  void Deactivate(vtkRenderer* renderer);

  void OnMouseMove() override;

protected:
  HoverPickStyle();
  ~HoverPickStyle() override;

private:
  vtkSmartPointer<vtkPartitionedDataSetCollection> Datasets;
  vtkNew<vtkTransform> Transform;
  vtkNew<vtkTransformFilter> TransformFilter;
  vtkNew<vtkActor> SelectionActor;
  vtkNew<vtkPolyDataMapper> SelectionMapper;
};
