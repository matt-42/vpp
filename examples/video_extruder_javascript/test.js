

// Wrap the C++ API.
var cpp = {
  update: Module.cwrap('update', 'number', ['number', 'number', 'number']),
  get_display_buffer: Module.cwrap('get_display_buffer', 'number', [])
};

function showSuccess() {}
function showError(e) { alert(e); }

// Start the webcam stream.
var gum = new GumWrapper({video: 'video'}, showSuccess, showError);
gum.play();

function draw_on_canvas(canvas, rgba_buffer) {

  var ctx = canvas.getContext('2d');
  var imageData = ctx.createImageData(canvas.width, canvas.height);
  var data = imageData.data;
  
  for (var i = 0; i < data.length; i += 4) {
    data[i]     = rgba_buffer[i]; // red
    data[i + 1] = rgba_buffer[i+1]; // green
    data[i + 2] = rgba_buffer[i+2]; // blue
    data[i + 3] = 255; // blue
  }
  ctx.putImageData(imageData, 0, 0);  
};

var frame_buffer = { data: null, size: 42, set: function (d, s) { this.data = d; this.size = s; } };

function update()
{
  var video = document.getElementById('video');
  var canvas1 = document.getElementById('canvas1');
  var ctx1 = canvas1.getContext('2d');
  var canvas3 = document.getElementById('canvas3');
  var ctx3 = canvas3.getContext('2d');

  // Put webcam image on canvas1
  ctx1.drawImage(video, 0, 0, canvas1.width, canvas1.height);

  // Canvas frame.
  var frame = ctx1.getImageData(0, 0, canvas1.width, canvas1.height);

  // Allocate
  size = frame.width * frame.height * 4;
  if (frame_buffer.size != size)
  {
    if (frame_buffer.data) Module._free(frame_buffer.data);
    frame_buffer.set(Module._malloc(size), size);
  }
  Module.HEAPU8.set(frame.data, frame_buffer.data);
  
  var start = Date.now();
  var t = cpp.update(frame_buffer.data, frame.width, frame.height);
  var end = Date.now();
  console.log(end - start, "ms");

  if (cpp.get_display_buffer() != 0)
  {
    var display_buffer = new Uint8Array(Module.HEAPU8.buffer, cpp.get_display_buffer(), frame.width * frame.height * 4);
    draw_on_canvas(canvas1, display_buffer);
    ctx3.drawImage(canvas1, 0, 0, canvas1.width, canvas1.height, 0,0,canvas3.width, canvas3.height);
  }
  setTimeout(function() { update(); }, 0);
}

function run()
{
  console.log("run!");
  document.getElementById('video').addEventListener("playing", function() {
    setTimeout(function() { update(); }, 0);
    //setInterval(function() { update(); }, 1000/30);
  }, false);
}
