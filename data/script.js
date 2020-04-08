var opts = {
  angle: -0.25, // The span of the gauge arc
  lineWidth: 0.30, // The line thickness
  radiusScale: 1, // Relative radius
  pointer: {
    length: 0.6, // // Relative to gauge radius
    strokeWidth: 0.035, // The thickness
    color: '#000000' // Fill color
  },
  limitMax: false,     // If false, max value increases automatically if value > maxValue
  limitMin: false,     // If true, the min value of the gauge will be fixed
  percentColors: [[0.0, "#ff0000" ], [0.50, "#f9c802"], [1.0, "#ff0000"]], // !!!!
  //colorStart: '#2FADCF',   // Colors
  //colorStop: '#0FC0DA',    // just experiment with them
  strokeColor: '#E0E0E0',  // to see which ones work best for you
  generateGradient: true,
  highDpiSupport: true,     // High resolution support
    renderTicks: {
    divisions: 6,
    divWidth: 2.1,
    divLength: 0.7,
    divColor: '#333333',
    subDivisions: 2,
    subLength: 0.5,
    subWidth: 1,
    subColor: '#666666'
  }
};
var target = document.getElementById('canvas-preview'); // your canvas element
var gauge = new Gauge(target).setOptions(opts); // create sexy gauge!
gauge.maxValue = 25; // set max gauge value
gauge.setMinValue(-10);  // Prefer setter over gauge.minValue = 0
gauge.animationSpeed = 20; // set animation speed (32 is default value)
gauge.set(0); // set actual value
gauge.setTextField(document.getElementById("preview-textfield"));
gauge.setTextField(document.getElementById("preview-textfield"), 2);