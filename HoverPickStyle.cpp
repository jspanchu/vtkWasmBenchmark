#include "HoverPickStyle.h"

#include <vtkHardwarePicker.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(HoverPickStyle);

HoverPickStyle::HoverPickStyle() = default;
HoverPickStyle::~HoverPickStyle() = default;

void HoverPickStyle::SetDatasets(vtkPartitionedDataSetCollection *datasets) {
  this->Datasets = vtk::MakeSmartPointer(datasets);
}

void HoverPickStyle::Activate(vtkRenderer *renderer) {
  vtkNew<vtkPolyData> empty;
  this->TransformFilter->SetTransform(this->Transform);
  this->TransformFilter->SetInputData(empty);
  this->SelectionMapper->SetInputConnection(
      this->TransformFilter->GetOutputPort());
  // this->SelectionMapper->SetScalarVisibility(false);
  this->SelectionActor->SetMapper(this->SelectionMapper);
  this->SelectionActor->GetProperty()->SetEdgeColor(1.0, 1.0, 1.0);
  this->SelectionActor->GetProperty()->SetEdgeVisibility(true);
  this->SelectionActor->GetProperty()->SetLineWidth(2);
  this->SelectionActor->GetProperty()->SetVertexVisibility(true);
  this->SelectionActor->GetProperty()->SetVertexColor(255 / 255., 117 / 255.,
                                                      24 / 255.);
  this->SelectionActor->GetProperty()->RenderPointsAsSpheresOn();
  this->SelectionActor->GetProperty()->SetPointSize(8);
  this->SelectionActor->SetPickable(false);
  renderer->AddActor(this->SelectionActor);
}

void HoverPickStyle::Deactivate(vtkRenderer *renderer) {
  renderer->RemoveActor(this->SelectionActor);
}

void HoverPickStyle::OnMouseMove() {
  int *clickPos = this->GetInteractor()->GetEventPosition();
  vtkNew<vtkHardwarePicker> picker;
  // pick a dataset
  picker->SnapToMeshPointOff();
  picker->Pick(clickPos[0], clickPos[1], 0, this->GetDefaultRenderer());
  const auto block_idx = picker->GetFlatBlockIndex();
  // something was picked, enlarge it and show it with edges and points
  // rendererd as spheres.
  if (block_idx > 0) {
    auto mesh =
        vtkPolyData::SafeDownCast(this->Datasets->GetDataSet(block_idx));
    this->TransformFilter->SetInputData(mesh);
    this->Transform->Identity();
    // display a rod piercing the mesh.
    double center[3] = {};
    mesh->GetCenter(center);
    this->Transform->Translate(center);
    this->Transform->Scale(1.2, 1.2, 1.2);
    this->Transform->Translate(-center[0], -center[1], -center[2]);
    this->SelectionActor->SetVisibility(true);
  } else {
    this->SelectionActor->SetVisibility(false);
  }
  this->GetInteractor()->Render();
  this->Superclass::OnMouseMove();
}
