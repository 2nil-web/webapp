const cnv_sta = document.getElementById("cnv_sta");
const cnv_ref = document.getElementById("cnv_ref");
const cnv_cor = document.getElementById("cnv_cor");
const freeze_button = document.getElementById("freeze_button");
const factor = document.getElementById("factor");
factor.addEventListener("change", function() {
  if (Number.isInteger(this.step)) {
    myParse = parseInt;
  } else {
    myParse = parseFloat;
  }

  function myParse(n) {
    return n;
  }
  min = myParse(this.min);
  value = myParse(this.value);
  max = myParse(this.max);
  step = myParse(this.step);
  console.log(`MIN: ${min} <= VALUE: ${value} <= MAX: ${max}, STEP: ${step}`);
  if (value < min) this.value = min;
  if (value > max) this.value = max;
});
const ctx_sta = cnv_sta.getContext("2d");
const ctx_ref = cnv_ref.getContext("2d");
const ctx_cor = cnv_cor.getContext("2d");
// Objets considérés comme des queues sans fin ... risque de memory overflow ?
// Normalement avec le rammasse-miette javascript ça devrait pas ...
var points1 = {};
var points2 = {};
var x = counter = 0;
var AI_Tolerance = cnv_ref.height / 10; // Plus la tolérance sa rapproche de zéro et plus il y aura de statut rouge
console.log(`Tolerance: ${AI_Tolerance}`);

function random_float(min, max) {
  return (Math.random() * (max - min + 1)) + min;
}

function random_int(min, max) {
  return Math.floor(Math.random() * (max - min + 1)) + min;
}
// La valeur de PI transformée à l'échelle du canvas
ScaledPI = Math.floor(Math.PI * cnv_ref.height / 2);
var last_y1 = 0;
// reference curve in blue
fc = 4;

function f1x() {
  if (random_int(0, 2) !== 1) return last_y1;
  if (x % ScaledPI === 0) {
    fc = random_float(3.4, 4.4);
  }
  // Plus fc se rapproche de zéro et plus la coube est plate
  last_y1 = (Math.sin(fc * x / cnv_ref.height) + 1) * cnv_ref.height / 2;
  return last_y1;
}

function Deg2Rad(degrees) {
  return degrees * Math.PI / 180;
}
var last_y2 = 0;
var slope = 4.2;
// corrected curve in orange
function f2x() {
  // Introduit une valeur parasite
  if (random_int(0, 1) !== 1) return last_y2;
  sinus = Math.sin(Deg2Rad(x));
  sign = (sinus >= 0 ? 1 : -1);
  last_y2 = (sign * (1 - Math.pow(1 - Math.abs(sinus), slope)) + 1) * cnv_ref.height / 2;
  //  last_y2=(Math.sin(slope*x/cnv_ref.height) + 1) * cnv_ref.height/2;
  //console.log(`SINUS: ${sinus}, SIGN: ${sign}, SLOPE: ${slope} LOCY: ${last_y2}`);
  return last_y2;
}

function drawCurveStart(ctx, fnx, yn, pointsn, color) {
  ctx.beginPath();
  ctx.moveTo(x - 1, yn);
  yn = fnx();
  pointsn[x] = yn;
  ctx.strokeStyle = color;
  ctx.lineTo(x, yn);
  ctx.stroke();
  return yn;
}

function drawCurveSlide0(cnv, ctx, fnx, yn, pointsn, color) {
  ctx.clearRect(0, 0, cnv.width, cnv.height);
  ctx.beginPath();
  pointsn[x] = yn;
  yn = fnx();
  ctx.strokeStyle = color;
  return yn;
}

function drawCurveSliden(cnv, ctx, ix, pointsn, color) {
  ctx.strokeStyle = color;
  iyn = pointsn[ix + counter - cnv.width];
  ctx.lineTo(ix, iyn);
  return iyn;
}

function sampling() {
  if (counter < cnv_ref.width) { // While the right of the canvas is not reached
    // Draw curves
    x += 1;
    y1 = drawCurveStart(ctx_ref, f1x, y1, points1, "blue");
    y2 = drawCurveStart(ctx_cor, f2x, y2, points2, "darkorange");
    // Draw status bar
    ctx_sta.beginPath();
    ctx_sta.moveTo(x - 1, cnv_sta.height / 2);
    ydiff = Math.abs(Math.abs(y1) - Math.abs(y2));
    if (ydiff < AI_Tolerance) ctx_sta.strokeStyle = "green";
    else ctx_sta.strokeStyle = "red";
    ctx_sta.lineTo(x, cnv_sta.height / 2);
    ctx_sta.stroke();
  } else { // Once the right of the canvas is reached
    x += 1;
    // Begin curves drawing
    y1 = drawCurveSlide0(cnv_ref, ctx_ref, f1x, y1, points1, "blue");
    y2 = drawCurveSlide0(cnv_cor, ctx_cor, f2x, y2, points2, "darkorange");
    // Begin status bar drawing
    ctx_sta.beginPath();
    for (var i = 0; i < cnv_ref.width; i++) {
      // Draw the curves
      iy1 = drawCurveSliden(cnv_ref, ctx_ref, i, points1, "blue");
      iy2 = drawCurveSliden(cnv_cor, ctx_cor, i, points2, "darkorange");
      // And the status bar
      iydiff = Math.abs(Math.abs(iy1) - Math.abs(iy2));
      if (iydiff < AI_Tolerance) ctx_sta.strokeStyle = "green";
      else ctx_sta.strokeStyle = "red";
      ctx_sta.strokeRect(i, 0, i, cnv_sta.height);
    }
    // Ends all the drawing
    ctx_ref.stroke();
    ctx_cor.stroke();
    ctx_sta.stroke();
  }
  counter++;
}
var sampling_freq = 1;
if (cnv_ref.getContext) {
  ctx_ref.lineWidth = 1;
  ctx_sta.lineWidth = 8;
  var y1 = f1x();
  var y2 = f2x();
  var timeout = setInterval(sampling, sampling_freq);
}

function do_freeze() {
  if (freeze_button.checked === true) {
    clearInterval(timeout);
  } else {
    timeout = setInterval(sampling, sampling_freq);
  }
}

function exit_on_esc() {
  if (event.keyCode === 27) app_exit();
}
document.addEventListener("keyup", exit_on_esc);