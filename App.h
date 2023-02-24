#pragma once

#include "HoverPickStyle.h"

#include <vtkActor.h>
#include <vtkCameraOrientationRepresentation.h>
#include <vtkCameraOrientationWidget.h>
#include <vtkCompositeDataDisplayAttributes.h>
#include <vtkIdList.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkVector.h>

class CameraState {
public:
  double viewUp[3];
  double position[3];
  double focalPoint[3];
  double viewAngle;
};

class BenchmarkApp {
public:
  BenchmarkApp();
  ~BenchmarkApp();

  enum PickType { Area, Hover, None };
  enum LayerID { Cone = 0, Sphere, Cylinder, NumLayers };

  void ClearSelections();
  int CreateDatasets(int nx, int ny);
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
  CameraState GetCameraState();
  void SetCameraState(CameraState &state);
  void SetShowCameraManipulator(bool show);

protected:
  void EndPickHandler(vtkObject *, unsigned long, void *);
  void EndRenderHandler(vtkObject *, unsigned long, void *);

private:
  vtkNew<vtkIdList> BlockIdsPerLayer[NumLayers];
  vtkVector3d SelectedBlockColor;
  float ScrollSensitivity;

  vtkNew<vtkCameraOrientationWidget> CamManipulator;

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
      .function("render", &BenchmarkApp::Render)
      .function("getCameraState", &BenchmarkApp::GetCameraState)
      .function("setCameraState", &BenchmarkApp::SetCameraState)
      .function("setShowCameraManipulator", &BenchmarkApp::SetShowCameraManipulator);
  emscripten::enum_<BenchmarkApp::LayerID>("LayerID")
      .value("Cone", BenchmarkApp::LayerID::Cone)
      .value("Sphere", BenchmarkApp::LayerID::Sphere)
      .value("Cylinder", BenchmarkApp::LayerID::Cylinder);
  emscripten::enum_<BenchmarkApp::PickType>("PickType")
      .value("Area", BenchmarkApp::PickType::Area)
      .value("Hover", BenchmarkApp::PickType::Hover)
      .value("None", BenchmarkApp::PickType::None);
  emscripten::value_object<CameraState>("CameraState")
      .field("viewUp", &CameraState::viewUp)
      .field("position", &CameraState::position)
      .field("focalPoint", &CameraState::focalPoint)
      .field("viewAngle", &CameraState::viewAngle);
  // Register std::array<int, 2> because CameraState::viewUp and others are
  // interpreted as such
  emscripten::value_array<std::array<double, 3>>("array_double_3")
      .element(emscripten::index<0>())
      .element(emscripten::index<1>())
      .element(emscripten::index<2>());
}
#endif
