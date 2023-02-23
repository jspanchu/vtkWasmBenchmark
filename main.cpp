#ifndef __EMSCRIPTEN__
// This main function lets you run the benchmark as a native desktop
// application.
#include "App.h"

#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
  auto pickType = BenchmarkApp::PickType::None;
  int nx = 32, ny = 32;
  int lw = 1, ps = 1;     // lineWidth, pointSize;
  int representation = 3; // Surface with edges.

  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "-nx") {
      nx = std::atoi(argv[i + 1]);
    } else if (std::string(argv[i]) == "-ny") {
      ny = std::atoi(argv[i + 1]);
    } else if (std::string(argv[i]) == "-r") {
      representation = std::atoi(argv[i + 1]);
    } else if (std::string(argv[i]) == "-lw") {
      lw = std::atoi(argv[i + 1]);
    } else if (std::string(argv[i]) == "-ps") {
      ps = std::atoi(argv[i + 1]);
    } else if (std::string(argv[i]) == "--area-pick") {
      pickType = BenchmarkApp::PickType::Area;
    } else if (std::string(argv[i]) == "--hover-preselect") {
      pickType = BenchmarkApp::PickType::Hover;
    } else if (std::string(argv[i]) == "--help" ||
               std::string(argv[i]) == "-h") {
      std::cout
          << "Usage: ./bench OPTIONS"
          << "\n"
          << "\t-nx <number of objects along X direction> \n"
          << "\t-ny <number of objects along Y direction> \n"
          << "\t-r <representation: 0-Points, 1-Wireframe, 2-Surface, "
             "3-Surface with edges> \n"
          << "\t-lw <line width> \n"
          << "\t-ps <point size> \n"
          << "\t--area-pick [Enables area picker. \'r\' toggles rubberband]\n"
          << "\t--hover-preselect [Highlights a mesh when mouse hovers above "
             "it]"
          << std::endl;
      return 0;
    }
  }

  BenchmarkApp app;
  app.Initialize();
  app.CreateDatasets(nx, ny);
  app.SetSelectedBlockColor(0.952, 0.937, 0.368);
  app.SetPickType(pickType);
  app.SetScrollSensitivity(1);
  app.SetLineWidth(lw);
  app.SetPointSize(ps);
  app.SetRepresentation(representation);
  app.SetEdgeColor(0.8, 0.8, 0.8);
  return app.Run();
}
#endif
