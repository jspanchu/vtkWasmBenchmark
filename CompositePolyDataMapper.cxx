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
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCameraOrientationRepresentation.h"
#include "vtkCameraOrientationWidget.h"
#include "vtkCellData.h"
#include "vtkCompositeDataDisplayAttributes.h"
#include "vtkCompositeDataSet.h"
#include "vtkCompositePolyDataMapper2.h"
#include "vtkConeSource.h"
#include "vtkCylinderSource.h"
#include "vtkElevationFilter.h"
#include "vtkExtractEdges.h"
#include "vtkHardwarePicker.h"
#include "vtkHardwareSelector.h"
#include "vtkInteractorStyleRubberBandPick.h"
#include "vtkInteractorStyleSwitch.h"
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
#include "vtkRenderedAreaPicker.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkSelectionNode.h"
#include "vtkSmartPointer.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"
#include "vtkTransformFilter.h"
#include "vtkUnsignedCharArray.h"

namespace {

// Handle mouse events
class HoverPickStyle : public vtkInteractorStyleTrackballCamera {
public:
  static HoverPickStyle *New();
  vtkTypeMacro(HoverPickStyle, vtkInteractorStyleTrackballCamera);

  void SetDatasets(vtkPartitionedDataSetCollection *datasets) {
    this->Datasets = vtk::MakeSmartPointer(datasets);
  }

  void AddSlectionRepresentationsToRenderer(vtkRenderer *renderer) {
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

  void OnMouseMove() override {
    int *clickPos = this->GetInteractor()->GetEventPosition();
    vtkNew<vtkHardwarePicker> picker;
    // pick a dataset
    picker->SnapToMeshPointOff();
    picker->Pick(clickPos[0], clickPos[1], 0, this->GetDefaultRenderer());
    const auto block_idx = picker->GetFlatBlockIndex();
    if (block_idx > 0) {
      auto cylinder =
          vtkPolyData::SafeDownCast(this->Datasets->GetDataSet(block_idx));
      this->TransformFilter->SetInputData(cylinder);
      this->Transform->Identity();
      // display a rod piercing the cylinder.
      double center[3] = {};
      cylinder->GetCenter(center);
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

private:
  vtkSmartPointer<vtkPartitionedDataSetCollection> Datasets;
  vtkNew<vtkTransform> Transform;
  vtkNew<vtkTransformFilter> TransformFilter;
  vtkNew<vtkActor> SelectionActor;
  vtkNew<vtkPolyDataMapper> SelectionMapper;
};

vtkStandardNewMacro(HoverPickStyle);

constexpr int CONE_LAYER_ID = 0;
constexpr int SPHERE_LAYER_ID = 1;
constexpr int CYLINDER_LAYER_ID = 2;

std::map<int, std::vector<unsigned int>>
    blockIdsPerLayer; // <layerID, [blockId,...]>
vtkCompositeDataDisplayAttributes *cdsa = nullptr;
vtkRenderWindowInteractor *iren = nullptr;
vtkPartitionedDataSetCollection *pdset = nullptr;
HoverPickStyle *pickStyle = nullptr;
vtkInteractorStyleSwitch *switchStyle = nullptr;
vtkActor *actor = nullptr;
vtkActor *areaSelectionActor = nullptr;
double selectedBlockColor[3] = {0.952, 0.937, 0.368};
} // namespace

// Called after area picker finished.
static void EndPick(vtkObject *vtkNotUsed(caller),
                    unsigned long vtkNotUsed(eventId), void *, void *) {
  auto win = iren->GetRenderWindow();
  auto ren = win->GetRenderers()->GetFirstRenderer();
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
  cdsa->RemoveBlockColors();
  for (unsigned int i = 0; i < res->GetNumberOfNodes(); ++i) {
    auto flatIdx = res->GetNode(i)->GetProperties()->Get(
        vtkSelectionNode::COMPOSITE_INDEX());
    auto dset = pdset->GetPartition(flatIdx / 2 - 1, 0);
    cdsa->SetBlockColor(dset, selectedBlockColor);
  }
  iren->GetRenderWindow()->Render();
}

static int createDatasets(int nx, int ny) {
  if (iren == nullptr) {
    std::cerr << "A vtkRenderWindowInterator does not exist!" << std::endl;
    return 0;
  }
  auto win = iren->GetRenderWindow();
  auto ren = win->GetRenderers()->GetFirstRenderer();

  if (pdset != nullptr) {
    pdset->Delete();
  }
  pdset = vtkPartitionedDataSetCollection::New();

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
      blockIdsPerLayer[CONE_LAYER_ID].emplace_back(partitionIdx);
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
      for (int comp = 0; comp < 4; ++comp) {
        sphereColors->FillTypedComponent(comp, sphereColor[comp]);
      }
      sphere->GetCellData()->SetScalars(sphereColors);
      blockIdsPerLayer[SPHERE_LAYER_ID].emplace_back(partitionIdx);
      pdset->SetPartition(partitionIdx++, 0, sphere);
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
      blockIdsPerLayer[CYLINDER_LAYER_ID].emplace_back(partitionIdx);
      pdset->SetPartition(partitionIdx++, 0, cylinder);
    }
  }
  std::cout << pdset->GetNumberOfPartitionedDataSets() << std::endl;
  std::cout << __func__ << '(' << nx << ',' << ny << ')' << std::endl;

  pickStyle->SetDatasets(pdset);
  vtkSmartPointer<vtkCompositePolyDataMapper2> mapper =
      vtkSmartPointer<vtkCompositePolyDataMapper2>::New();
  mapper->SetCompositeDataDisplayAttributes(cdsa);
  mapper->SetScalarModeToUseCellData();
  mapper->SetInputDataObject(pdset);
  actor->SetMapper(mapper);
  return 1;
}

static void clearSelections() {
  // remove coloring from area selections.
  cdsa->RemoveBlockColors();
  std::cout << __func__ << std::endl;
  iren->GetRenderWindow()->Render();
}

static void setEdgeColor(float r, float g, float b) {
  actor->GetProperty()->SetEdgeColor(r, g, b);
}

static void setLayerVisibility(int layer, bool visible) {
  if (layer < CONE_LAYER_ID || layer > CYLINDER_LAYER_ID) {
    std::cerr << "Invalid layer " << layer << std::endl;
    return;
  }
  for (auto &id : blockIdsPerLayer[layer]) {
    std::cout << id << '\n';
    cdsa->SetBlockVisibility(pdset->GetPartition(id, 0), visible);
  }
  std::cout << __func__ << "(" << layer << ',' << visible << ")" << std::endl;
  iren->GetRenderWindow()->Render();
}

static void setLineWidth(float value) {
  actor->GetProperty()->SetLineWidth(value);
  std::cout << __func__ << "(" << value << ")" << std::endl;
  iren->GetRenderWindow()->Render();
}

static void setAreaPick(bool enabled) {
  auto win = iren->GetRenderWindow();
  auto ren = win->GetRenderers()->GetFirstRenderer();
  std::cout << __func__ << "(" << enabled << ")" << std::endl;
  if (enabled) {
    vtkNew<vtkInteractorStyleRubberBandPick> rbp;
    iren->SetInteractorStyle(rbp);
    vtkNew<vtkRenderedAreaPicker> picker;
    iren->SetPicker(picker);
    // pass pick events to the HardwareSelector
    vtkNew<vtkCallbackCommand> cbc;
    cbc->SetCallback(EndPick);
    cbc->SetClientData(ren);
    iren->AddObserver(vtkCommand::EndPickEvent, cbc);
  } else {
    iren->SetPicker(nullptr);
    iren->SetInteractorStyle(switchStyle);
    switchStyle->SetCurrentStyleToTrackballCamera();
  }
}

static void setHoverPreSelect(bool enabled) {
  auto win = iren->GetRenderWindow();
  auto ren = win->GetRenderers()->GetFirstRenderer();
  // Setup picker
  if (enabled && pickStyle != nullptr) {
    pickStyle->SetDefaultRenderer(ren);
    pickStyle->AddSlectionRepresentationsToRenderer(ren);
    iren->SetInteractorStyle(pickStyle);
  } else if (switchStyle != nullptr) {
    iren->SetInteractorStyle(switchStyle);
    switchStyle->SetCurrentStyleToTrackballCamera();
  }
  std::cout << __func__ << "(" << enabled << ")" << std::endl;
  iren->GetRenderWindow()->Render();
}

static void setPointSize(float value) {
  actor->GetProperty()->SetPointSize(value);
  std::cout << __func__ << "(" << value << ")" << std::endl;
  iren->GetRenderWindow()->Render();
}

static void setRepresentation(int representation) {
  if (representation > VTK_SURFACE) {
    // surf with edges
    actor->GetProperty()->SetEdgeVisibility(true);
  } else {
    actor->GetProperty()->SetEdgeVisibility(false);
  }
  actor->GetProperty()->SetRepresentation(representation);
  std::cout << __func__ << "(" << representation << ")" << std::endl;
  iren->GetRenderWindow()->Render();
}

static void resetView() {
  auto win = iren->GetRenderWindow();
  auto ren = win->GetRenderers()->GetFirstRenderer();
  ren->ResetCamera();
  std::cout << __func__ << std::endl;
  iren->GetRenderWindow()->Render();
}

static void setSelectedBlockColor(float r, float g, float b) {
  selectedBlockColor[0] = r;
  selectedBlockColor[1] = g;
  selectedBlockColor[2] = b;
}

static int run() {
  auto win = iren->GetRenderWindow();
  auto ren = win->GetRenderers()->GetFirstRenderer();
  std::cout << __func__ << std::endl;
  ren->AddActor(actor);

  // camera orientation widget
  vtkNew<vtkCameraOrientationWidget> camManipulator;
  camManipulator->SetParentRenderer(ren);
  camManipulator->On();
  auto rep = vtkCameraOrientationRepresentation::SafeDownCast(
      camManipulator->GetRepresentation());
  rep->AnchorToLowerLeft();

  iren->UpdateSize(600, 600);
  ren->GetActiveCamera()->Elevation(30.0);
  ren->GetActiveCamera()->Azimuth(-40.0);
  ren->GetActiveCamera()->Zoom(3.0);
  ren->GetActiveCamera()->Roll(10.0);
  ren->ResetCamera();
  win->Render();
  iren->Start();
  if (pdset != nullptr) {
    pdset->Delete();
  }
  cdsa->Delete();
  actor->Delete();
  pickStyle->Delete();
  switchStyle->Delete();
  iren->Delete();
  return 0;
}

static void initialize() {
  cdsa = vtkCompositeDataDisplayAttributes::New();
  iren = vtkRenderWindowInteractor::New();
  pickStyle = ::HoverPickStyle::New();
  switchStyle = vtkInteractorStyleSwitch::New();
  actor = vtkActor::New();
  vtkSmartPointer<vtkRenderWindow> win =
      vtkSmartPointer<vtkRenderWindow>::New();
  vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
  win->AddRenderer(ren);
  win->SetInteractor(iren);
  win->SetMultiSamples(0);
  std::cout << __func__ << std::endl;
}

#ifndef __EMSCRIPTEN__
int main(int argc, char *argv[]) {
  bool enable_pick = true;
  int nx = 32, ny = 32;
  if (argc > 3) {
    nx = std::atoi(argv[1]);
    ny = std::atoi(argv[2]);
    enable_pick = std::atoi(argv[3]);
  } else if (argc > 2) {
    nx = std::atoi(argv[1]);
    ny = std::atoi(argv[2]);
  }
  initialize();
  createDatasets(nx, ny);
  setLineWidth(1);
  // setHoverPreSelect(enable_pick);
  setAreaPick(enable_pick);
  setPointSize(1);
  setRepresentation(3);
  setEdgeColor(0.8, 0.8, 0.8);
  return run();
}
#else
#include <emscripten/bind.h>
EMSCRIPTEN_BINDINGS(module) {
  emscripten::function("initialize", &initialize);
  emscripten::function("run", &run);
  emscripten::function("clearSelections", &clearSelections);
  emscripten::function("createDatasets", &createDatasets);
  emscripten::function("setEdgeColor", &setEdgeColor);
  emscripten::function("setLayerVisibility", &setLayerVisibility);
  emscripten::function("setLineWidth", &setLineWidth);
  emscripten::function("setAreaPick", &setAreaPick);
  emscripten::function("setHoverPreSelect", &setHoverPreSelect);
  emscripten::function("setPointSize", &setPointSize);
  emscripten::function("setRepresentation", &setRepresentation);
  emscripten::function("setSelectedBlockColor", &setSelectedBlockColor);
  emscripten::function("resetView", &resetView);
}
#endif
