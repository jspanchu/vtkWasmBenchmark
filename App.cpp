#include "App.h"

#include <vtkAbstractPropPicker.h>
#include <vtkCamera.h>
#include <vtkCameraOrientationRepresentation.h>
#include <vtkCameraOrientationWidget.h>
#include <vtkCellData.h>
#include <vtkCompositeDataDisplayAttributes.h>
#include <vtkCompositePolyDataMapper2.h>
#include <vtkConeSource.h>
#include <vtkCylinderSource.h>
#include <vtkHardwareSelector.h>
#include <vtkInteractorStyle.h>
#include <vtkInteractorStyleRubberBandPick.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkProperty.h>
#include <vtkRenderedAreaPicker.h>
#include <vtkRendererCollection.h>
#include <vtkSelectionNode.h>
#include <vtkSphereSource.h>

#include <iostream>

BenchmarkApp::BenchmarkApp() { std::cout << __func__ << std::endl; }

BenchmarkApp::~BenchmarkApp() { std::cout << __func__ << std::endl; }

void BenchmarkApp::ClearSelections() {
  std::cout << __func__ << std::endl;
  // remove coloring from area selections.
  this->DisplayAttributes->RemoveBlockColors();
}

void BenchmarkApp::CreateDatasets(int nx, int ny) {
  std::cout << __func__ << '(' << nx << ',' << ny << ')' << std::endl;

  // clear previous meshes.
  this->DisplayAttributes->RemoveBlockColors();
  this->DisplayAttributes->RemoveBlockVisibilities();
  this->Meshes->SetNumberOfPartitionedDataSets(0);

  vtkNew<vtkConeSource> coneSource;
  vtkNew<vtkSphereSource> sphereSource;
  vtkNew<vtkCylinderSource> cylinderSource;

  unsigned char coneColor[4] = {190, 33, 40, 255};
  unsigned char sphereColor[4] = {36, 152, 71, 255};
  unsigned char cylinderColor[4] = {6, 78, 140, 255};

  double x = 0, y = 0, z = 0.;
  int dimensions[2] = {nx, ny};
  double spacings[3] = {50.0, 50.0, 50.0};
  int partitionIdx = 0;
  for (int i = 0, x = 0; i < dimensions[0]; ++i, x += spacings[0]) {
    for (int j = 0, y = 0; j < dimensions[1]; ++j, y += spacings[1]) {
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
      for (int comp = 0; comp < 4; ++comp) {
        coneColors->FillTypedComponent(comp, coneColor[comp]);
      }
      cone->GetCellData()->SetScalars(coneColors);
      this->BlockIdsPerLayer[LayerID::Cone]->InsertNextId(partitionIdx);
      this->Meshes->SetPartition(partitionIdx++, 0, cone);
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
      for (int comp = 0; comp < 4; ++comp) {
        sphereColors->FillTypedComponent(comp, sphereColor[comp]);
      }
      sphere->GetCellData()->SetScalars(sphereColors);
      this->BlockIdsPerLayer[LayerID::Sphere]->InsertNextId(partitionIdx);
      this->Meshes->SetPartition(partitionIdx++, 0, sphere);
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
      for (int comp = 0; comp < 4; ++comp) {
        cylinderColors->FillTypedComponent(comp, cylinderColor[comp]);
      }
      cylinder->GetCellData()->SetScalars(cylinderColors);
      this->BlockIdsPerLayer[LayerID::Cylinder]->InsertNextId(partitionIdx);
      this->Meshes->SetPartition(partitionIdx++, 0, cylinder);
    }
  }
  std::cout << "Created " << this->Meshes->GetNumberOfPartitionedDataSets()
            << " objects" << std::endl;

  this->HoverStyle->SetDatasets(this->Meshes);
  vtkSmartPointer<vtkCompositePolyDataMapper2> mapper =
      vtkSmartPointer<vtkCompositePolyDataMapper2>::New();
  mapper->SetCompositeDataDisplayAttributes(this->DisplayAttributes);
  mapper->SetScalarModeToUseCellData();
  mapper->SetInputDataObject(this->Meshes);
  this->Actor->SetMapper(mapper);
}

void BenchmarkApp::Initialize() {
  std::cout << __func__ << std::endl;
  // create the default renderer
  vtkNew<vtkRenderer> ren;
  this->Window->AddRenderer(ren);
  this->Window->SetInteractor(this->Interactor);
  this->Window->SetMultiSamples(0);
}

void BenchmarkApp::Render() {
  std::cout << __func__ << std::endl;
  this->Window->Render();
}

void BenchmarkApp::ResetView() {
  std::cout << __func__ << std::endl;
  auto ren = this->Window->GetRenderers()->GetFirstRenderer();
  if (ren != nullptr) {
    ren->ResetCamera();
  }
}

int BenchmarkApp::Run() {
  std::cout << __func__ << std::endl;
  auto ren = this->Window->GetRenderers()->GetFirstRenderer();
  if (!ren->HasViewProp(this->Actor)) {
    ren->AddActor(this->Actor);
  }

  // camera orientation widget
  vtkNew<vtkCameraOrientationWidget> camManipulator;
  camManipulator->SetParentRenderer(ren);
  camManipulator->On();
  auto rep = vtkCameraOrientationRepresentation::SafeDownCast(
      camManipulator->GetRepresentation());
  rep->AnchorToLowerLeft();

  this->Interactor->UpdateSize(600, 600);
  ren->GetActiveCamera()->Elevation(30.0);
  ren->GetActiveCamera()->Azimuth(-40.0);
  ren->GetActiveCamera()->Zoom(3.0);
  ren->GetActiveCamera()->Roll(10.0);
  ren->ResetCamera();
  this->Window->Render();
  this->Interactor->Start();
  return 0;
}

void BenchmarkApp::SetEdgeColor(float r, float g, float b) {
  std::cout << __func__ << "(" << r << ',' << g << ',' << b << ")" << std::endl;
  this->Actor->GetProperty()->SetEdgeColor(r, g, b);
}

void BenchmarkApp::SetPickType(PickType pickType) {
  std::cout << __func__ << "("
            << (pickType == Area
                    ? "Area"
                    : (pickType == PickType::Hover ? "Hover" : "None"))
            << ")" << std::endl;
  auto ren = this->Window->GetRenderers()->GetFirstRenderer();
  // Setup picker
  if (pickType == PickType::Area) {
    vtkNew<vtkInteractorStyleRubberBandPick> rbp;
    this->Interactor->SetInteractorStyle(rbp);
    vtkNew<vtkRenderedAreaPicker> picker;
    this->Interactor->SetPicker(picker);
    // pass pick events to the HardwareSelector
    auto ren = this->Window->GetRenderers()->GetFirstRenderer();
    this->Interactor->AddObserver(vtkCommand::EndPickEvent, this,
                                  &BenchmarkApp::EndPickHandler);
    rbp->SetMouseWheelMotionFactor(this->ScrollSensitivity);
  } else if (pickType == PickType::Hover) {
    this->Interactor->SetPicker(nullptr);
    this->HoverStyle->SetDefaultRenderer(ren);
    this->HoverStyle->Activate(ren);
    this->Interactor->SetInteractorStyle(this->HoverStyle);
    this->HoverStyle->SetMouseWheelMotionFactor(this->ScrollSensitivity);
  } else if (pickType == PickType::None) {
    this->HoverStyle->Deactivate(ren);
    this->Interactor->SetPicker(nullptr);
    this->Interactor->SetInteractorStyle(this->SwitchStyle);
    this->SwitchStyle->SetCurrentStyleToTrackballCamera();
    this->SwitchStyle->GetCurrentStyle()->SetMouseWheelMotionFactor(
        this->ScrollSensitivity);
  }
}

void BenchmarkApp::SetScrollSensitivity(float sensitivity) {
  std::cout << __func__ << "(" << sensitivity << ")" << std::endl;
  if (auto iStyle = vtkInteractorStyle::SafeDownCast(
          this->Interactor->GetInteractorStyle())) {
    if (auto switchStyle = vtkInteractorStyleSwitch::SafeDownCast(iStyle)) {
      switchStyle->GetCurrentStyle()->SetMouseWheelMotionFactor(sensitivity);
    } else {
      iStyle->SetMouseWheelMotionFactor(sensitivity);
    }
  }
  this->ScrollSensitivity = sensitivity;
}

void BenchmarkApp::SetLayerVisibility(LayerID layer, bool visible) {
  std::cout << __func__ << "(" << layer << ',' << visible << ")" << std::endl;
  if (layer < LayerID::Cone || layer >= LayerID::NumLayers) {
    std::cerr << "Invalid layer " << layer << std::endl;
    return;
  }
  auto &blkIds = this->BlockIdsPerLayer[layer];
  for (auto it = blkIds->begin(); it != blkIds->end(); ++it) {
    const auto &mesh = this->Meshes->GetPartition(*it, 0);
    this->DisplayAttributes->SetBlockVisibility(mesh, visible);
  }
}

void BenchmarkApp::SetLineWidth(float width) {
  std::cout << __func__ << "(" << width << ")" << std::endl;
  this->Actor->GetProperty()->SetLineWidth(width);
}

void BenchmarkApp::SetPointSize(float size) {
  std::cout << __func__ << "(" << size << ")" << std::endl;
  this->Actor->GetProperty()->SetPointSize(size);
}

void BenchmarkApp::SetRepresentation(int representation) {
  std::cout << __func__ << "(" << representation << ")" << std::endl;
  if (representation > VTK_SURFACE) {
    // surf with edges
    this->Actor->GetProperty()->SetEdgeVisibility(true);
  } else {
    this->Actor->GetProperty()->SetEdgeVisibility(false);
  }
  this->Actor->GetProperty()->SetRepresentation(representation);
}

void BenchmarkApp::SetSelectedBlockColor(float r, float g, float b) {
  std::cout << __func__ << "(" << r << ',' << g << ',' << b << ")" << std::endl;
  this->SelectedBlockColor.Set(r, g, b);
}

// Called after area picker finished.
void BenchmarkApp::EndPickHandler(vtkObject *, unsigned long, void *) {
  auto ren = this->Window->GetRenderers()->GetFirstRenderer();

  vtkNew<vtkHardwareSelector> sel;
  sel->SetFieldAssociation(vtkDataObject::FIELD_ASSOCIATION_CELLS);
  sel->SetRenderer(ren);

  double x0 = ren->GetPickX1();
  double y0 = ren->GetPickY1();
  double x1 = ren->GetPickX2();
  double y1 = ren->GetPickY2();

  sel->SetArea(static_cast<int>(x0), static_cast<int>(y0), static_cast<int>(x1),
               static_cast<int>(y1));
  vtkSmartPointer<vtkSelection> res;
  res.TakeReference(sel->Select());
  if (!res) {
    cerr << "Selection not supported." << endl;
    return;
  }

  vtkSelectionNode *cellids = res->GetNode(0);
  this->DisplayAttributes->RemoveBlockColors();
  for (unsigned int i = 0; i < res->GetNumberOfNodes(); ++i) {
    auto flatIdx = res->GetNode(i)->GetProperties()->Get(
        vtkSelectionNode::COMPOSITE_INDEX());
    auto dset = this->Meshes->GetPartition(flatIdx / 2 - 1, 0);
    this->DisplayAttributes->SetBlockColor(dset,
                                           this->SelectedBlockColor.GetData());
  }
  this->Window->Render();
}
