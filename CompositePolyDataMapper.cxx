/*=========================================================================

  Program:   Visualization Toolkit
  Module:    app.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkCompositeDataDisplayAttributes.h"
#include "vtkCompositeDataSet.h"
#include "vtkCompositePolyDataMapper2.h"
#include "vtkElevationFilter.h"
#include "vtkExtractEdges.h"
#include "vtkHardwarePicker.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPartitionedDataSetCollection.h"
#include "vtkPointDataToCellData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkTransform.h"
#include "vtkTransformFilter.h"
#include <vtkCellData.h>
#include <vtkConeSource.h>
#include <vtkCylinderSource.h>
#include <vtkSphereSource.h>
#include <vtkUnsignedCharArray.h>

namespace
{

constexpr float SELECTED_COMPOSITE_OPACITY = 0.5;
constexpr float SELECTED_BLOCK_OPACITY = 1.0;

// Handle mouse events
class HoverPickStyle : public vtkInteractorStyleTrackballCamera
{
public:
  static HoverPickStyle* New();
  vtkTypeMacro(HoverPickStyle, vtkInteractorStyleTrackballCamera);

  void SetDatasets(vtkPartitionedDataSetCollection* datasets)
  {
    this->Datasets = vtk::MakeSmartPointer(datasets);
  }

  void AddSlectionRepresentationsToRenderer(vtkRenderer* renderer)
  {
    vtkNew<vtkPolyData> empty;
    this->TransformFilter->SetTransform(this->Transform);
    this->TransformFilter->SetInputData(empty);
    this->SelectionMapper->SetInputConnection(this->TransformFilter->GetOutputPort());
    // this->SelectionMapper->SetScalarVisibility(false);
    this->SelectionActor->SetMapper(this->SelectionMapper);
    this->SelectionActor->GetProperty()->SetEdgeColor(1.0, 1.0, 1.0);
    this->SelectionActor->GetProperty()->SetEdgeVisibility(true);
    this->SelectionActor->GetProperty()->SetLineWidth(2);
    this->SelectionActor->GetProperty()->SetVertexVisibility(true);
    this->SelectionActor->GetProperty()->SetVertexColor(255/255., 117/255., 24/255.);
    this->SelectionActor->GetProperty()->RenderPointsAsSpheresOn();
    this->SelectionActor->GetProperty()->SetPointSize(8);
    this->SelectionActor->SetPickable(false);
    renderer->AddActor(this->SelectionActor);
  }

  void OnMouseMove() override
  {
    int* clickPos = this->GetInteractor()->GetEventPosition();
    vtkNew<vtkHardwarePicker> picker;
    // pick a dataset
    picker->SnapToMeshPointOff();
    picker->Pick(clickPos[0], clickPos[1], 0, this->GetDefaultRenderer());
    const auto block_idx = picker->GetFlatBlockIndex();
    if (block_idx > 0)
    {
      auto cylinder = vtkPolyData::SafeDownCast(this->Datasets->GetDataSet(block_idx));
      this->TransformFilter->SetInputData(cylinder);
      this->Transform->Identity();
      // display a rod piercing the cylinder.
      double center[3] = {};
      cylinder->GetCenter(center);
      this->Transform->Translate(center);
      this->Transform->Scale(1.2, 1.2, 1.2);
      this->Transform->Translate(-center[0], -center[1], -center[2]);
      this->SelectionActor->SetVisibility(true);
    }
    else
    {
      this->SelectionActor->SetVisibility(false);
    }
    this->GetInteractor()->Render();
    this->Superclass::OnMouseMove();
  }

private:
  vtkSmartPointer<vtkPartitionedDataSetCollection> Datasets;
  vtkNew<vtkTransform> Transform;
  vtkNew<vtkTransformFilter> TransformFilter;
  vtkNew<vtkActor> SelectionActor;
  vtkNew<vtkPolyDataMapper> SelectionMapper;
};

vtkStandardNewMacro(HoverPickStyle);
}

int main(int argc, char* argv[])
{
  vtkSmartPointer<vtkRenderWindow> win = vtkSmartPointer<vtkRenderWindow>::New();
  vtkSmartPointer<vtkRenderWindowInteractor> iren =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
  win->AddRenderer(ren);
  win->SetInteractor(iren);
  win->SetMultiSamples(0);

  vtkNew<vtkPartitionedDataSetCollection> pdset;
  vtkNew<vtkConeSource> coneSource;
  vtkNew<vtkSphereSource> sphereSource;
  vtkNew<vtkCylinderSource> cylinderSource;

  unsigned char coneColor[4] = { 190, 33, 40, 255 };
  unsigned char sphereColor[4] = { 36, 152, 71, 255 };
  unsigned char cylinderColor[4] = { 6, 78, 140, 255 };

  double x = 0, y = 0, z = 0.;
  int dimensions[2] = { 32, 32 };
  double spacings[3] = { 50.0, 50.0, 50.0 };
  int partitionIdx = 0;
  for (int i = 0, x = 0; i < dimensions[0]; ++i, x += spacings[0])
  {
    for (int j = 0, y = 0; j < dimensions[1]; ++j, y += spacings[1])
    {
      z = 0;
      coneSource->SetCenter(x, y, z);
      coneSource->SetRadius(spacings[0] / 3);
      coneSource->SetHeight(spacings[1] - 2.0);
      coneSource->SetResolution(16);
      coneSource->Update();
      vtkNew<vtkPolyData> cone;
      cone->DeepCopy(coneSource->GetOutput());
      vtkNew<vtkUnsignedCharArray> coneColors;
      coneColors->SetNumberOfComponents(4);
      coneColors->SetNumberOfTuples(cone->GetNumberOfCells());
      for (int comp = 0; comp < 4; ++comp)
      {
        coneColors->FillTypedComponent(comp, coneColor[comp]);
      }
      cone->GetCellData()->SetScalars(coneColors);
      pdset->SetPartition(partitionIdx++, 0, cone);
      z += spacings[2];

      sphereSource->SetCenter(x, y, z);
      sphereSource->SetRadius(spacings[0] / 3);
      sphereSource->SetPhiResolution(16);
      sphereSource->SetThetaResolution(16);
      sphereSource->Update();
      vtkNew<vtkPolyData> sphere;
      sphere->DeepCopy(sphereSource->GetOutput());
      vtkNew<vtkUnsignedCharArray> sphereColors;
      sphereColors->SetNumberOfComponents(4);
      sphereColors->SetNumberOfTuples(sphere->GetNumberOfCells());
      for (int comp = 0; comp < 4; ++comp)
      {
        sphereColors->FillTypedComponent(comp, sphereColor[comp]);
      }
      sphere->GetCellData()->SetScalars(sphereColors);
      pdset->SetPartition(partitionIdx++, 1, sphere);
      z += spacings[2];

      cylinderSource->SetCenter(x, y, z);
      cylinderSource->SetRadius(spacings[0] / 3);
      cylinderSource->SetHeight(spacings[1] - 2.0);
      cylinderSource->SetResolution(16);
      cylinderSource->Update();
      vtkNew<vtkPolyData> cylinder;
      cylinder->DeepCopy(cylinderSource->GetOutput());
      vtkNew<vtkUnsignedCharArray> cylinderColors;
      cylinderColors->SetNumberOfComponents(4);
      cylinderColors->SetNumberOfTuples(cylinder->GetNumberOfCells());
      for (int comp = 0; comp < 4; ++comp)
      {
        cylinderColors->FillTypedComponent(comp, cylinderColor[comp]);
      }
      cylinder->GetCellData()->SetScalars(cylinderColors);
      pdset->SetPartition(partitionIdx++, 1, cylinder);
      pdset->SetPartition(partitionIdx++, 2, cylinder);
    }
  }

  vtkSmartPointer<vtkCompositePolyDataMapper2> mapper =
    vtkSmartPointer<vtkCompositePolyDataMapper2>::New();
  vtkNew<vtkCompositeDataDisplayAttributes> cdsa;
  mapper->SetCompositeDataDisplayAttributes(cdsa.GetPointer());
  mapper->SetScalarModeToUseCellData();
  mapper->SetInputDataObject(pdset);

  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);
  actor->GetProperty()->SetEdgeColor(0.8, 0.8, 0.8);
  actor->GetProperty()->SetEdgeVisibility(true);
  actor->GetProperty()->SetLineWidth(1);
  ren->AddActor(actor);

  ren->GetActiveCamera()->Elevation(30.0);
  ren->GetActiveCamera()->Azimuth(-40.0);
  ren->GetActiveCamera()->Zoom(3.0);
  ren->GetActiveCamera()->Roll(10.0);
  ren->ResetCamera();

  // Setup picker
  vtkNew<::HoverPickStyle> style;
  style->SetDefaultRenderer(ren);
  style->SetDatasets(pdset);
  style->AddSlectionRepresentationsToRenderer(ren);
  iren->SetInteractorStyle(style);
  win->SetSize(1920, 1080);
  win->Render();
  iren->Start();
  return 0;
}
