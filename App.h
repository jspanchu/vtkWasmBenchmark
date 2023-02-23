#pragma once

#include "HoverPickStyle.h"

#include <vtkActor.h>
#include <vtkCompositeDataDisplayAttributes.h>
#include <vtkIdList.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkVector.h>

class BenchmarkApp {
public:
  BenchmarkApp();
  ~BenchmarkApp();

  enum PickType { Area, Hover, None };
  enum LayerID { Cone = 0, Sphere, Cylinder, NumLayers };

  void ClearSelections();
  void CreateDatasets(int nx, int ny);
  void Initialize();
  void Render();
  void ResetView();
  int Run();
  void SetEdgeColor(float r, float g, float b);
  void SetLayerVisibility(LayerID layer, bool visible);
  void SetLineWidth(float width);
  void SetPickType(PickType pickType);
  void SetScrollSensitivity(float sensitivity);
  void SetPointSize(float size);
  void SetRepresentation(int representation);
  void SetSelectedBlockColor(float r, float g, float b);

protected:
  void EndPickHandler(vtkObject *, unsigned long, void *);

private:
  vtkNew<vtkIdList> BlockIdsPerLayer[NumLayers];
  vtkVector3d SelectedBlockColor;
  float ScrollSensitivity;

  vtkNew<vtkPartitionedDataSetCollection> Meshes;

  vtkNew<vtkCompositeDataDisplayAttributes> DisplayAttributes;
  vtkNew<vtkRenderWindowInteractor> Interactor;
  vtkNew<vtkRenderWindow> Window;

  vtkNew<HoverPickStyle> HoverStyle;
  vtkNew<vtkInteractorStyleSwitch> SwitchStyle;

  vtkNew<vtkActor> Actor;
  vtkNew<vtkActor> AreaSelectionActor;
};

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
EMSCRIPTEN_BINDINGS(benchmark_app_binding) {
  emscripten::class_<BenchmarkApp>("BenchmarkApp")
      .constructor<>()
      .function("initialize", &BenchmarkApp::Initialize)
      .function("run", &BenchmarkApp::Run)
      .function("clearSelections", &BenchmarkApp::ClearSelections)
      .function("createDatasets", &BenchmarkApp::CreateDatasets)
      .function("setEdgeColor", &BenchmarkApp::SetEdgeColor)
      .function("setLayerVisibility", &BenchmarkApp::SetLayerVisibility)
      .function("setLineWidth", &BenchmarkApp::SetLineWidth)
      .function("setPickType", &BenchmarkApp::SetPickType)
      .function("setScrollSensitivity", &BenchmarkApp::SetScrollSensitivity)
      .function("setPointSize", &BenchmarkApp::SetPointSize)
      .function("setRepresentation", &BenchmarkApp::SetRepresentation)
      .function("setSelectedBlockColor", &BenchmarkApp::SetSelectedBlockColor)
      .function("resetView", &BenchmarkApp::ResetView)
      .function("render", &BenchmarkApp::Render);
  emscripten::enum_<BenchmarkApp::LayerID>("LayerID")
      .value("Cone", BenchmarkApp::LayerID::Cone)
      .value("Sphere", BenchmarkApp::LayerID::Sphere)
      .value("Cylinder", BenchmarkApp::LayerID::Cylinder);
  emscripten::enum_<BenchmarkApp::PickType>("PickType")
      .value("Area", BenchmarkApp::PickType::Area)
      .value("Hover", BenchmarkApp::PickType::Hover)
      .value("None", BenchmarkApp::PickType::None);
}
#endif
