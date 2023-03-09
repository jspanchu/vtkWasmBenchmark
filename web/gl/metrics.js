import css from '../styles/app.module.css';

let primMap = {};
let primCounts = [0, 0, 0];
primMap[WebGL2RenderingContext.POINTS] = { ndx: 0, fn: count => count, };
primMap[WebGL2RenderingContext.LINE_LOOP] = { ndx: 1, fn: count => count, };
primMap[WebGL2RenderingContext.LINE_STRIP] = { ndx: 1, fn: count => count - 1, };
primMap[WebGL2RenderingContext.LINES] = { ndx: 1, fn: count => count / 2 | 0, };
primMap[WebGL2RenderingContext.TRIANGLE_STRIP] = { ndx: 2, fn: count => count - 2, };
primMap[WebGL2RenderingContext.TRIANGLE_FAN] = { ndx: 2, fn: count => count - 2, };
primMap[WebGL2RenderingContext.TRIANGLES] = { ndx: 2, fn: count => count / 3 | 0, };

function addCount(ctx, type, count) {
  // todo: associate primitive counts with a ctx.
  const primInfo = primMap[type];
  primCounts[primInfo.ndx] += primInfo.fn(count);
}

WebGL2RenderingContext.prototype.drawArrays = (function (oldFn) {
  return function (type, offset, count) {
    addCount(this, type, count);
    oldFn.call(this, type, offset, count);
  };
}(WebGL2RenderingContext.prototype.drawArrays));

WebGL2RenderingContext.prototype.drawArraysInstanced = (function (oldFn) {
  return function (type, first, count, instanceCount) {
    addCount(this, type, count * instanceCount);
    oldFn.call(this, type, first, count, instanceCount);
  };
}(WebGL2RenderingContext.prototype.drawArraysInstanced));

WebGL2RenderingContext.prototype.drawElements = (function (oldFn) {
  return function (type, count, indexType, offset) {
    addCount(this, type, count);
    oldFn.call(this, type, count, indexType, offset);
  };
}(WebGL2RenderingContext.prototype.drawElements));

const times = [];
const framesWindow = 1000; // ms
let fps = 0;
// get the current counts
const glMetricsContainer = document.getElementById('glmetrics');
glMetricsContainer.setAttribute('class', css.glMetricsPanel);
const elem = document.getElementById('info');
let numObjects = 0;

// this function is called after vtkRenderWindow finishes rendering from C++
function tick(now) {

  while (times.length > 0 && times[0] <= now - framesWindow) {
    times.shift();
  }
  times.push(now);
  fps = times.length;

  elem.textContent = 
`${fps.toFixed(1)} fps
${numObjects} objects
${primCounts[2]} triangles
${primCounts[1]} lines
${primCounts[0]} points`;

  primCounts = [0, 0, 0];
}

function setNumberOfObjects(num) {
  numObjects = num;
  // html text updated on next tick.
}

export { setNumberOfObjects, tick };
