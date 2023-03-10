import css from './styles/app.module.css';
import * as glMetrics from './gl/metrics';

var Module = {
  canvas: (function () {
    var canvas = document.getElementById('canvas');
    canvas.addEventListener(
      "webglcontextlost",
      function (e) {
        console.error('WebGL context lost. You will need to reload the page.');
        e.preventDefault();
      },
      false
    );
    return canvas;
  })(),
  print: (text) => { console.log('stdout: ' + text); },
  printErr: (text) => { console.log('stderr: ' + text); },
};

// -----------------------------------------------------------
// Helpers
// -----------------------------------------------------------
// Converts the HTML color picker value into usable RGB
function hexToRGB(hex) {
  const result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
  return result
    ? {
      r: parseInt(result[1], 16),
      g: parseInt(result[2], 16),
      b: parseInt(result[3], 16),
    }
    : null;
}

// define benchmark app default settings here.
let scrollSensitivity = 0.15;
let lineWidth = 1;
let pointSize = 1;
let layer0Visibility = true; // cones
let layer1Visibility = true; // spheres
let layer2Visibility = true; // cylinders
let camStates = [null, null, null, null, null]
let nx = 8;
let ny = 8;

// -----------------------------------------------------------
// UI control handling
let controlContainer = document.createElement('div');
controlContainer.setAttribute('class', css.benchmarkControlPanel);
const body = document.querySelector('body');
body.appendChild(controlContainer);
const controller = `
<div>
<table>
  <tr>
    <td>
      <select class='representations' style="width: 100%">
        <option value='0'>Points</option>
        <option value='1'>Wireframe</option>
        <option value='2'>Surface</option>
        <option value='3' selected>Surface With Edges</option>
      </select>
    </td>
  </tr>
  <tr id='lw_row' style='display: none;'>
    <td>Line width</td>
    <td>
      <input class='lw' type='range' min='1' max='20' value='${lineWidth}' oninput='this.nextElementSibling.value = this.value'>
      <output>${lineWidth}</output>
    </td>
  </tr>
  <tr id='ec_row' style='display: none;'>
    <td>
      <label>Edge color: </label>
      <input class='edgeColor' type='color', value='#FFFFFF'/>
    </td>
  </tr>
  <tr id='ps_row' style='display: none;'>
    <td>Point size (Points)</td>
    <td>
      <input class='ps' type='range' min='1' max='20' value='${pointSize}' oninput='this.nextElementSibling.value = this.value'>
      <output>${pointSize}</output>
    </td>
  </tr>
  <tr>
    <td>Selector type</td>
    <td>
      <select class='pickerType' style="width: 100%">
        <option value='0' selected>Area Picker</option>
        <option value='1'>Hover Pre-select</option>
        <option value='2'>None</option>
      </select>
    </td>
    <tr id='selection_color_row' style='display: none;'>
      <td>Selected block color ('r' toggles rubberband): </td>
      <td>
        <input class='selectionColor' type='color', value='#FFF00F'/>
      </td>
    </tr>
  </tr>
  <tr>
    <td>Layer 0 (Cones)</td>
    <td>
      <input type='checkbox' class='layer0Visibility' checked='${layer0Visibility}'>
    </td>
  </tr>
  <tr>
    <td>Layer 1 (Spheres)</td>
    <td>
      <input type='checkbox' class='layer1Visibility' checked='${layer1Visibility}'>
    </td>
  </tr>
  <tr>
    <td>Layer 2 (Cylinders)</td>
    <td>
      <input type='checkbox' class='layer2Visibility' checked='${layer2Visibility}'>
    </td>
  </tr>
  <tr>
    <td>nx </td>
    <td>
      <input class='nx' type='range' min='2' max='128' value='${nx}' oninput="this.nextElementSibling.value = this.value">
      <output>${nx}</output>
    </td>
  </tr>
  <tr>
    <td>ny </td>
    <td>
      <input class='ny' type='range' min='2' max='128' value='${ny}' oninput="this.nextElementSibling.value = this.value">
      <output>${ny}</output>
    </td>
  </tr>
</table>
<hr>
<table>
  <tr>
    <td>Scroll sensitivity </td>
    <td>
      <input class='scrollSensitivity' type='range' min='0' max='1' value='${scrollSensitivity}' step='0.01' oninput="this.nextElementSibling.value = this.value">
      <output>${scrollSensitivity}</output>
    </td>
  </tr>
  <tr>
    <td>Camera Orientation Widget</td>
    <td>
      <input type='checkbox' class='camManipulatorVisibility' checked='true'>
    </td>
  </tr>
</table>
<table>
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">
  <tr>
    <td>
      <button class='view1button' style="width: 100%">View 1</button>
    </td>
    <td>
      <button class='view1erasebutton' style="width: 100%"><i class="fa fa-trash"></i></button>
    </td>
    <td>
      <button class='view2button' style="width: 100%">View 2</button>
    </td>
    <td>
      <button class='view2erasebutton' style="width: 100%"><i class="fa fa-trash"></i></button>
    </td>
    <td>
      <button class='view3button' style="width: 100%">View 3</button>
    </td>
    <td>
      <button class='view3erasebutton' style="width: 100%"><i class="fa fa-trash"></i></button>
    </td>
    <td>
      <button class='view4button' style="width: 100%">View 4</button>
    </td>
    <td>
      <button class='view4erasebutton' style="width: 100%"><i class="fa fa-trash"></i></button>
    </td>
    <td>
      <button class='view5button' style="width: 100%">View 5</button>
    </td>
    <td>
      <button class='view5erasebutton' style="width: 100%"><i class="fa fa-trash"></i></button>
    </td>
  </tr>
</table>
<table>
  <td>
    <button class='resetviewbutton' style="width: 100%">Reset View</button>
  </td>
  <td>
    <button class='clearselectionbutton' style="width: 100%">Clear Selection</button>
  </td>
</table>
</div>`
controlContainer.innerHTML = controller;
controlContainer.style.display = 'block';

// Use the export name to instantiate the app
var app = null;
document
  .querySelector('.scrollSensitivity')
  .addEventListener('input', (e) => {
    scrollSensitivity = Number(e.target.value);
    app.setScrollSensitivity(scrollSensitivity);
  });
document
  .querySelector('.representations')
  .addEventListener('change', (e) => {
    const newRepValue = Number(e.target.value);
    app.setRepresentation(newRepValue);
    app.render();
    updatePropertyWidgets();
  });
document
  .querySelector('.lw')
  .addEventListener('input', (e) => {
    lineWidth = Number(e.target.value);
    app.setLineWidth(lineWidth);
    app.render();
  });
document
  .querySelector('.edgeColor').addEventListener('input', (e) => {
    const color = hexToRGB(e.target.value);
    app.setEdgeColor(color.r / 255.0, color.g / 255.0, color.b / 255.0);
    app.render();
  });
document
  .querySelector('.ps')
  .addEventListener('input', (e) => {
    pointSize = Number(e.target.value);
    app.setPointSize(pointSize);
    app.render();
  });
document
  .querySelector('.pickerType')
  .addEventListener('change', () => {
    updatePickers();
  });
document
  .querySelector('.selectionColor').addEventListener('input', (e) => {
    const color = hexToRGB(e.target.value);
    app.setSelectedBlockColor(color.r / 255.0, color.g / 255.0, color.b / 255.0);
  });
document
  .querySelector('.layer0Visibility')
  .addEventListener('change', () => {
    layer0Visibility = document.querySelector('.layer0Visibility').checked;
    app.setLayerVisibility(Module.LayerID.Cone, layer0Visibility);
    app.render();
  });
document
  .querySelector('.layer1Visibility')
  .addEventListener('change', () => {
    layer1Visibility = document.querySelector('.layer1Visibility').checked;
    app.setLayerVisibility(Module.LayerID.Sphere, layer1Visibility);
    app.render();
  });
document
  .querySelector('.layer2Visibility')
  .addEventListener('change', () => {
    layer2Visibility = document.querySelector('.layer2Visibility').checked;
    app.setLayerVisibility(Module.LayerID.Cylinder, layer2Visibility);
    app.render();
  });
document
  .querySelector('.nx')
  .addEventListener('input', () => {
    updateDatasets();
  });
document
  .querySelector('.ny')
  .addEventListener('input', () => {
    updateDatasets();
  });
document
  .querySelector('.camManipulatorVisibility')
  .addEventListener('change', () => {
    let camManipulatorVisibility = document.querySelector('.camManipulatorVisibility').checked;
    app.setShowCameraManipulator(camManipulatorVisibility);
    app.render();
  });
document
  .querySelector('.view1button')
  .addEventListener('click', () => {
    if (camStates[0] === null) {
      camStates[0] = app.getCameraState();
    }
    else {
      app.setCameraState(camStates[0]);
      app.render();
    }
  });
document
  .querySelector('.view2button')
  .addEventListener('click', () => {
    if (camStates[1] === null) {
      camStates[1] = app.getCameraState();
    }
    else {
      app.setCameraState(camStates[1]);
      app.render();
    }
  });
document
  .querySelector('.view3button')
  .addEventListener('click', () => {
    if (camStates[2] === null) {
      camStates[2] = app.getCameraState();
    }
    else {
      app.setCameraState(camStates[2]);
      app.render();
    }
  });
document
  .querySelector('.view4button')
  .addEventListener('click', () => {
    if (camStates[3] === null) {
      camStates[3] = app.getCameraState();
    }
    else {
      app.setCameraState(camStates[3]);
      app.render();
    }
  });
document
  .querySelector('.view5button')
  .addEventListener('click', () => {
    if (camStates[4] === null) {
      camStates[4] = app.getCameraState();
    }
    else {
      app.setCameraState(camStates[4]);
      app.render();
    }
  });
document
  .querySelector('.view1erasebutton')
  .addEventListener('click', () => {
    camStates[0] = null;
  });
document
  .querySelector('.view2erasebutton')
  .addEventListener('click', () => {
    camStates[1] = null;
  });
document
  .querySelector('.view3erasebutton')
  .addEventListener('click', () => {
    camStates[2] = null;
  });
document
  .querySelector('.view4erasebutton')
  .addEventListener('click', () => {
    camStates[3] = null;
  });
document
  .querySelector('.view5erasebutton')
  .addEventListener('click', () => {
    camStates[4] = null;
  });
document
  .querySelector('.resetviewbutton')
  .addEventListener('click', () => {
    app.resetView();
    app.render();
  });
document
  .querySelector('.clearselectionbutton')
  .addEventListener('click', () => {
    app.clearSelections();
    app.render();
  });

function updatePropertyWidgets() {
  const newRepValue = Number(document.querySelector('.representations').value);
  document.getElementById('ps_row').style.display = 'none';
  document.getElementById('lw_row').style.display = 'none';
  document.getElementById('ec_row').style.display = 'none';
  if (newRepValue === 0) {
    document.getElementById('ps_row').style.display = 'table-row';
  }
  if (newRepValue === 1) {
    document.getElementById('lw_row').style.display = 'table-row';
    }
  if (newRepValue === 3) {
    document.getElementById('lw_row').style.display = 'table-row';
    document.getElementById('ec_row').style.display = 'table-row';
  }
}

function updatePickers() {
  const newPickerType = Number(document.querySelector('.pickerType').value);
  if (newPickerType === 0) {
    document.getElementById('selection_color_row').style.display = 'table-row';
    app.setPickType(Module.PickType.Area);
  }
  else if (newPickerType === 1) {
    document.getElementById('selection_color_row').style.display = 'none';
    app.setPickType(Module.PickType.Hover);
  }
  else if (newPickerType === 2) {
    document.getElementById('selection_color_row').style.display = 'none';
    app.setPickType(Module.PickType.None);
  }
}

function updateDatasets() {
  nx = Number(document.querySelector('.nx').value);
  ny = Number(document.querySelector('.ny').value);
  glMetrics.setNumberOfObjects(app.createDatasets(nx, ny));
  app.resetView();
  app.render();
}

function updateRepresentation() {
  const color = hexToRGB(document.querySelector('.edgeColor').value);
  const newLineWidth = Number(document.querySelector('.lw').value);
  const newPointSize = Number(document.querySelector('.ps').value);
  const newRepValue = Number(document.querySelector('.representations').value);
  app.setEdgeColor(color.r / 255.0, color.g / 255.0, color.b / 255.0);
  app.setLineWidth(newLineWidth);
  app.setPointSize(newPointSize);
  app.setRepresentation(newRepValue);
  app.render();
  updatePropertyWidgets();
}

vtkRenderingApplicationExport(Module).then(runtime => {
  app = new Module.BenchmarkApp();
  console.log('App created');
  // expose to the console for easy access
  global.app = app;
  global.tick = glMetrics.tick;
  // -----------------------------------------------------------
  // Initialize application
  // -----------------------------------------------------------
  app.initialize();
  app.setScrollSensitivity(scrollSensitivity);
  app.setSelectedBlockColor(0.952, 0.937, 0.368);
  updateDatasets();
  updateRepresentation();
  updatePickers();
  // sends a resize event so that the render window fills up browser tab dimensions.
  setTimeout(() => {
    window.dispatchEvent(new Event('resize'));
  }, 0);
  // focus on the canvas to grab keyboard inputs.
  canvas.setAttribute('tabindex', '0');
  // grab focus when the render window region receives mouse clicks.
  canvas.addEventListener('click', () => canvas.focus());
  // starts processing events on browser main thread.
  app.run();
})
