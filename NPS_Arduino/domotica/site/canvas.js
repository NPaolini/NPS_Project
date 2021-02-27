function drawRect(ctx, x, y, w, h, hbase, curr)
{
  var t = h;
  if(t == hbase)
    t -= 3;
  var grad1 = "#d4d400";
  var grd = ctx.createLinearGradient(0, t, 0, hbase);
  if(curr) {
    grd.addColorStop(0, "#d4d400");
    grd.addColorStop(1, "#c2ba55");
    }
  else {
    grd.addColorStop(0, "#f55");
    grd.addColorStop(1, "#55f");
    }
  ctx.fillStyle = grd;
  ctx.fillRect(x, y, w, h); 
  var color1 = '#fa5';
  var color2 = '#95a';
  ctx.beginPath();
  ctx.lineWidth = 1;
  ctx.strokeStyle = color1;
  --x; 
  w += 2;
  ctx.moveTo(x, y + h);
  ctx.lineTo(x, y);
  ctx.lineTo(x + w, y);
  ctx.stroke();
  ctx.beginPath();
  ctx.strokeStyle = color2;
  ctx.moveTo(x + w, y);
  ctx.lineTo(x + w, y + h);
  ctx.lineTo(x, y + h);
  ctx.stroke();
}
//----------------------------------
function calcH(val, h)
{
  var top = 30.0;
  var bottom = 10.0;
  var t = val - bottom;
  t /= top - bottom;
  return h * t;
}
//----------------------------------
function drawScale(ctx, canvas)
{
  var font = "sans";
  var fontsize = 8;
  var top = 10;
  var offs = 10;
  var left = canvas.width - offs;
  var height = canvas.height - top;

  var num = 10;
  var step = parseInt(height / num) * 2;

  var y = top;
  ctx.beginPath();
  ctx.strokeStyle = "rgb(125,125,125)";
  l = left - offs;
  offs *= 2;
  for (var i = 0; i <= num; i++, y += step) {
    ctx.moveTo(l, y);
    ctx.lineTo(l + offs, y);
    }
  ctx.stroke();
  offs /= 2;
//  ctx.setTickness(font, 1.5);
  ctx.strokeStyle = "rgb(255,225,0)";
  y = top;
  for (var i = 0; i <= num; i++, y += step) {
    var txt = "" + (30 - i * 4);
    ctx.drawTextCenter(font, fontsize, left, y, txt);
    }
}
//----------------------------------
function drawHorzRed(ctx, canvas)
{
  ctx.lineWidth = 1;
  var left = 10;
  var top = 10;

  var height = canvas.height - top;
  var width = canvas.width;

  var num = 10;
  var step = parseInt(height / num);

  var y = top + step * num / 2;

  ctx.beginPath();
  ctx.strokeStyle = "rgb(200,55,55)";
  ctx.moveTo(left, y);
  ctx.lineTo(width, y);
  ctx.stroke();
}
//----------------------------------
function drawGrid(ctx, canvas)
{
  ctx.strokeStyle = "rgb(93,93,93)";
  ctx.lineWidth = 1;

  var left = 10;
  var top = 10;
  var height = canvas.height - top;
  var width = canvas.width - left * 2;

  var end = 10;
  var step = parseInt(height / end);

  ctx.beginPath();
  var y = top;
  for (var i = 0; i <= end; i++, y += step) {
    ctx.moveTo(left, y);
    ctx.lineTo(width, y);
    }
  ctx.stroke();

  ctx.beginPath();
  end = 24;
  step = parseInt(width / end);
  for (var i = 0, x = left; i <= end; i++, x += step) {
    ctx.moveTo(x, top);
    ctx.lineTo(x, canvas.height);
    }
  ctx.stroke();
}
//----------------------------------
function drawGraph(ctx, canvas, my_array, ix)
{
  ctx.clearRect(0, 0, canvas.width, canvas.height);

  drawGrid(ctx, canvas);

  var left = 10;
  var top = 10;
  var width = canvas.width - left * 2;

  var end = my_array.length;
  var step = parseInt(width / end);
  var step1 = parseInt(step * 0.65);
  var step2 = step - step1;
  var x = left + step2;
  var y = canvas.height - top;
  for (var i = 0; i < end; i++) {
    var dx = (i & 1) ? step2 : step1;
    var h = calcH(parseFloat(my_array[i]), y);
    drawRect(ctx, x, y - h + top, dx, h, y, i == ix);
    x += step;
    }
  drawScale(ctx, canvas);
  drawHorzRed(ctx, canvas);
}
//------------------------------
function drawAx_X(ctx, canvas)
{
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  var font = "sans";
  var fontsize = 12;
  var y = ctx.fontAscent(font,fontsize) + 5;
  ctx.strokeStyle = "rgb(255,225,0)";
  var offs = 10;
  var left = offs;
  var width = canvas.width - offs * 2;

  ctx.setTickness(font, 1.5);
  var len = 24;
  step = parseInt(width / len);

  for (var i = 0, x = left + step / 2; i < len; i++, x += step) {
    var txt = i < 10 ? "0" + i : "" + i;
    ctx.drawTextCenter(font, fontsize, x, y, txt);
    }
}
//------------------------------
function draw_axes(fz, id, p)
{
  var canvas = document.getElementById(id);
  var ctx = canvas.getContext('2d');
  CanvasTextFunctions.enable(ctx);
  fz(ctx, canvas, p);
}
//------------------------------
function drawRect2(ctx, x, y, w, h, curr)
{
	ctx.shadowColor = '#d53';
	ctx.shadowBlur = 10;
	ctx.lineJoin = 'bevel';
	ctx.lineWidth = 5;
	ctx.strokeStyle = '#38f';
	ctx.strokeRect(x, y, w, h);

	ctx.fillStyle = '#f83';
	var y2 = y + 5;
	while(true) {
	  ctx.fillRect(x + 5, y2, w - 10, 3); 
	  y2 += 5;
	  if(y2 >= curr)
	  	break;
	  }
}
//------------------------------
function drawBarPos(idPos, h)
{
  var canvas = document.getElementById(idPos);
  var ctx = canvas.getContext('2d');
  ctx.clearRect(0, 0, canvas.width, canvas.height);
	drawRect2(ctx, 0, 0, canvas.width, canvas.height, canvas.height * h);	
}
