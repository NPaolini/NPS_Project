var svAjax = getHTTPObject();
var onWorking = false;
var queryList = [];
var listDayOfWeek = ["Lunedì", "Martedì" ,"Mercoledì", "Giovedì", "Venerdì", "Sabato", "Domenica"]; 
var currDOW = getCurrWeek();
var svCanvas;
var svCtx;
var currIxHour;
//------------------------------
function getHTTPObject()
{
  if(typeof XMLHttpRequest != 'undefined')
    return new XMLHttpRequest();
  try {
    return new ActiveXObject("Msxml2.XMLHTTP");
    } catch(e) {
        try {
          return new ActiveXObject("Microsoft.XMLHTTP");
          } catch(e) {}
        }
  return false;
}
//------------------------------
function updateData(myurl)
{
  svAjax.open("GET", myurl, true);
//  ajax.setRequestHeader("connection", "close");
  svAjax.onreadystatechange = useHttpResponse;
  svAjax.send(null);
}
//------------------------------
function mainLoop(code)
{
  if(onWorking) {
    if(queryList.length < 10 && -1 == queryList.indexOf(code))
      queryList.push(code);
    return;
    }
  onWorking = true;
  updateData("/?" + code);
}
//------------------------------
//------------------------------
function startAcquireLoop()
{
  setTimeout("acquireLoop()", 1000);
}
//------------------------------
function acquireLoop()
{
	if(!(onWorking || queryList.length))
	  mainLoop("a=W," + currDOW);
  startAcquireLoop();
}
//------------------------------
// ogni record è formato dal numero di elementi, dal codice e, a seguire, gli elementi
//------------------------------
function useHttpResponse()
{
  if (svAjax.readyState == 4) {
    try {
      if(svAjax.responseText.startsWith("<Refresh>")) {
        location = location.href;
        return;
        }
      var svArray2 = svAjax.responseText.split("|");
      var len = svArray2.length;
      var offs = 0;
      while(offs < len) {
        var nElem = parseInt(svArray2.slice(offs, offs + 1));
        var code = svArray2.slice(offs + 1, offs + 2)[0];
        var arr = svArray2.slice(offs + 2, offs + 2 + nElem);
        switch(code) {
          case "t":
            update_temper(arr);
            break;
          case "r":
            update_rele(arr);
            break;
          case "u":
            update_humid(arr);
            break;
          case "x":
            update_alarm(arr);
            break;
          case "c":
            updateCronoTemper(arr);
            break;
          case "i":
            updateImpTemper(arr);
            break;
          case "m":
            update_motor(arr);
            break;
          }
        offs += nElem + 2;
        }
      onWorking = false;
      if(queryList.length) {
        code = queryList.shift();
        mainLoop(code);
        }
      } catch(e) { onWorking = false; }
    }
}
//------------------------------
function update_xxx(idbase, arr)
{
  var nElem = arr.length;
  for(var ii = 0; ii < nElem; ++ii) {
    var id = idbase + (ii + 1);
    try {
      var resp = document.getElementById(id);
      if(resp)
        resp.innerHTML = arr[ii];
      } catch(e) {}
    }
}
//------------------------------
function update_temper(arr)
{
  update_xxx('idTemper', arr);
}
//------------------------------
function add_remove(id, addC, remC, set)
{
  var obj = document.getElementById(id);
  if(!obj)
    return;
  if(set) {
    obj.classList.add(addC);
    obj.classList.remove(remC);
    }
  else {
    obj.classList.add(remC);
    obj.classList.remove(addC);
    }
}
//------------------------------
function add_removeArr(id, addC, remC)
{
  var obj = document.getElementById(id);
  if(!obj)
    return;
  var len = remC.length;
  for(var i = 0; i < len; ++i)
    obj.classList.remove(remC[i]);
  obj.classList.add(addC);
}
//------------------------------
function update_rele(arr)
{
  var nElem = arr.length;
  for(var ii = 0; ii < nElem; ++ii) {
    var id = "idRele" + (ii + 1);
    try {
      var val = parseInt(arr[ii]);
      add_remove(id, 'green', 'grey', val);
      } catch(e) {}
    }
}
//------------------------------
function update_alarm(arr)
{
//  indice, abilitato(0) | escluso, stato(0) | pin attivo
  var len = arr.length;
  var offs = 0;
  while(offs < len) {
    var ix = parseInt(arr.slice(offs, offs + 1));
    var ena = parseInt(arr.slice(offs + 1, offs + 2));
    var stat = parseInt(arr.slice(offs + 2, offs + 3));
    var id = 'idAlarm' + ix + 'a';
    add_remove(id, 'green', 'grey', ena);
    
    id = 'idAlarm' + ix + 'b';
    if(!ix) {
      if(!ena) {
        add_removeArr(id, 'grey', ['red', 'blue']);
        }
      else {
        switch(stat) {
          case 0:
            add_removeArr(id, 'grey', ['red', 'blue']);
            break;
          case 1:
            add_removeArr(id, 'blue', ['red', 'grey']);
            break;
          case 2:
            add_removeArr(id, 'red', ['blue', 'grey']);
            break;
          }
        }
      }
    else {
      add_remove(id, 'red', 'grey', stat);
      id = "alarmTxt" + ix;
      setInputVal(id, ena ? "Escludi" : "Includi");
      }
    offs += 3;
    }
}
//------------------------------
function update_humid(arr)
{
  update_xxx('idHumid', arr);
}
//------------------------------
function update_child(id, arr, offsChild)
{
  var obj = document.getElementById(id).childNodes;
  if(!obj)
    return;
  var nElem = arr.length;
  for(var ii = 0; ii < nElem; ++ii) {
    try {
      obj[ii + offsChild].innerHTML = arr[ii];
      } catch(e) {}
    }
}
//------------------------------
function setInputVal(id, val)
{
  try {
    var resp = document.getElementById(id);
    if(resp)
      resp.value = val;
    } catch(e) {}
}
//------------------------------
function setVal(id, val)
{
  try {
    var resp = document.getElementById(id);
    if(resp)
      resp.innerHTML = val;
    } catch(e) {}
}
//------------------------------
function clearCronoTemper()
{
  var arr = [];
  arr.length = 12;
  arr.fill('.');
  for(var i = 0; i < 4; ++i) {
    var id = 'idRowCrono' + (i + 1);
    update_child(id, arr, 0);
    }
}
//------------------------------
function getCurrIndex()
{
  var d = new Date();
  var h = d.getHours();
  var m = d.getMinutes();
  return fromHourToIndex2(h, m);
}
//------------------------------
function updateImpTemper(arr)
{
  var ix = parseInt(arr.slice(0, 1));
  var temp = arr.slice(1, 2);
  var tm = ixToHour(ix) + "-" + ixToHour(ix + 1) + " -> " + temp;
  setVal('idCronoCurrTemper', tm);
  temp = arr.slice(2, 3);
  setVal('idCronoActTemper', temp);
  if(currIxHour != ix) {
    currIxHour = ix;
    mainLoop("c=R");
    }
}
//------------------------------
/*
  il primo valore contiene la settimana cui si riferiscono i dati,
  il secondo lo stato, il terzo la temperatura in manuale,
  a seguire tutte le temperature
*/
function updateCronoTemper(arr)
{
  var dow = parseInt(arr.slice(0, 1));
  if(dow != currDOW) {
    currDOW = dow;
    chgWeekBtn(dow);
    }
  var status = parseInt(arr.slice(1, 2));
  setVal('idCronoOff', status & 1 ? 'ON' : 'OFF');
  setInputVal('idCronoOffBtn', status & 1 ? 'OFF' : 'ON');
  add_remove('idCronOffLed', 'green', 'grey',  status & 1); 

  setVal('idCronoManual', status & 2 ? 'Manuale' : 'Auto');
  setInputVal('idCronoManualBtn', status & 2 ? 'Auto' : 'Manuale');
  setVal('idCronStatus', status & 4 ? 'Acceso' : 'Spento');  
  add_remove('idCronStatusLed', 'green', 'grey',  status & 4); 
  
  var vmanual = arr.slice(2, 3)[0];
  setVal('idCronoManualTemper', vmanual);
  var nElem = arr.length;
  if(nElem < 4)
    return;
  var stepGroup = (nElem - 3) / 4;
  var offs = 3;
  for(var i = 0; i < 4; ++i) {
    var arr2 = arr.slice(offs, offs + stepGroup);
    var id = 'idRowCrono' + (i + 1);
    update_child(id, arr2, 0);
    offs += stepGroup;
    }
  drawGraph(svCtx, svCanvas, arr.slice(3), currIxHour);
}
//------------------------------
function chgWeekBtn(val)
{
  var obj = document.getElementById('idmenuBtn').children;
  var c = obj.length;
  for(var i = 0; i < c; ++i) {
    if(val == i + 1)
      obj[i].classList.add('menuBtnItemAct');
    else
      obj[i].classList.remove('menuBtnItemAct');
    }
}
//------------------------------
function chgWeek(val)
{
  if(0 == val) {
    val = getCurrWeek();
    }
  chgWeekBtn(val);
  mainLoop("c=W," + val);
  clearCronoTemper();
}
//------------------------------
function getSel(id)
{
  var obj = document.getElementById(id);
  return obj.options[obj.selectedIndex].value;
}
//------------------------------
function setSel(id, val)
{
  var obj = document.getElementById(id);
  for(var i = 0; i < obj.length; ++i) {
    var v = obj.options[i].value;
    if(v == val) {
      obj.selectedIndex = i;
      break;
      }
    }
}
//------------------------------
function getCurrWeek()
{
  var d = new Date();
  var w = d.getDay();
  // trasforma da 0-6 (con 0 -> domenica) a 1-7 spostando domenica a 7
  if(!w)
    return 7;  
  return w;
}
//------------------------------
function fillWeekCBX()
{
  var objWeek = document.getElementById('idGroupWeek');
  for(var i = 0; i < 7; ++i) {
    var elNew = document.createElement('option');
    elNew.text =   listDayOfWeek[i];
    elNew.value = i + 1;
    try {
      objWeek.add(elNew, null); // standards compliant; doesn't work in IE
      }
    catch(ex) {
      objWeek.add(elNew); // IE only
      }
    }
  chgWeekBtn(getCurrWeek());
}    
//------------------------------
function copyGroup()
{
  var w = getSel('idGroupWeek');
  // copia da selezione corrente (c) a visualizzazione corrente (W)
  mainLoop("c=C," + w + ",W," + currDOW);
}
//------------------------------
function fromHourToIndex2(h, m)
{
  return h * 2 + parseInt(m / 30) + 2;
}
//------------------------------
function zeroFill(val)
{
  if(val < 10)
    return '0' + val;
  return val;
}
//------------------------------
function ixToHour(ix)
{
  var h = parseInt(ix / 2);
  var m = parseInt(ix % 2) * 30;
  return '' + zeroFill(h) + ':' + zeroFill(m);
}
//------------------------------
function fromHourToIndex(v)
{
  var a = v.split(':');
  var h = 0;
  var m = 0;
  if(a.length) {
    h = parseInt(a[0]);
    if(a.length > 1)
      m = parseInt(a[1]);
    } 
  return fromHourToIndex2(h, m);
}
//------------------------------
function setManualTemper(id)
{
  var t = document.getElementById(id);
  t = parseFloat(t.value) * 100;
  t = parseInt(t);
  var toSend = "c=S,W,0,1,"
  toSend +=  t;
  toSend += '#';
  mainLoop(toSend);
}
//------------------------------
function setRange(idTemper, idtimeFrom, idtimeTo)
{
  var t = document.getElementById(idTemper);
  t = parseFloat(t.value) * 100;
  t = parseInt(t);
  from = document.getElementById(idtimeFrom);
  to = document.getElementById(idtimeTo);
  from = fromHourToIndex(from.value);
  to = fromHourToIndex(to.value);
  if(!to || from == to)
    to = from + 1;
  else if(from > to) {
    var tmp = from;
    from = to;
    to = tmp;
    }
  var toSend = "c=S,W,";
  toSend += currDOW;
  for(var i = from; i < to; ++i) 
    toSend += ',' + i + ',' + t;
  toSend += '#';
  mainLoop(toSend);
}
//------------------------------
function sendDateTime()
{
  var d = new Date();
  var arr = [];
  var t = d.getDay();
  if(!t)
    t = 7;
  arr.push(t);
  t = d.getDate();
  arr.push(t);
  t = d.getMonth() + 1;
  arr.push(t);
  t = d.getFullYear();
  arr.push(t);
  t = d.getHours();
  arr.push(t);
  t = d.getMinutes();
  arr.push(t);
  t = d.getSeconds();
  arr.push(t);
  var toSend = "d=" + arr[0];
  for(var i = 1; i < arr.length; ++i) 
    toSend += "," + arr[i];
  toSend += "#";
  mainLoop(toSend);
}
//------------------------------
var Motors = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
//------------------------------
function get_1_0(v)
{
  return v ? '1' : '0';
}
//------------------------------
function update_motor(arr)
{
// ix,pulsante_locale,pulsante_remoto,finecorsa[hard(1)|soft(2)],pos
// es. 0|1|0|0|0| 1|0|0|0|55| 2|0|0|1|255| ...
// idMotA1_1
  var nElem = arr.length;
  var nblk = nElem / 5;
  var A = 'A'.charCodeAt(0);
  var posMotors = [];
  var styles = [0, 'red', 'yellow', 'green'];
  var pos = 0;
  for(var jj = 0; jj < nblk; ++jj) {
  	var ax = String.fromCharCode(A + jj / 2) + ((jj & 1) + 1);
    var idLight = "idMot" + ax + "_"; // idMotA1_
    var idBth = "idBtnMot" + ax; // idBtnMotA1
     ++pos;
    for(var ii = 1; ii < 5; ++ii, ++pos) {
      var id = idLight + ii;
      if(2 == ii) {
        try {
          var val = parseInt(arr[pos]);
          if(jj & 1)
            add_remove(idBth, 'm_down_on', 'm_down', val);
          else
            add_remove(idBth, 'm_up_on', 'm_up', val);
          Motors[jj] = val;
          } catch(e) {}
        }
      else {
        try {
          var val = parseInt(arr[pos]);
          if(4 == ii) {
            posMotors.push(val);
            }
          if(3 == ii) {
            switch(val) {
              case 0:
                add_removeArr(id, 'grey', ['green', 'blue']);
                break;
              case 1:
                add_removeArr(id, 'green', ['blue', 'grey']);
                break;
              case 2:
                add_removeArr(id, 'blue', ['green', 'grey']);
                break;
              }
            }
          else
            add_remove(id, styles[ii], 'grey', val);
          } catch(e) {}
        }
      }
    }
  nblk = posMotors.length / 2;
  pos = 0;
  for(var jj = 0; jj < nblk; ++jj, pos += 2) {
    var idPos = "idMotPos" + String.fromCharCode(A + jj); // idMotPosA
    var v1 = posMotors[pos];
    var v2 = posMotors[pos + 1];
    var h = 0;
    if(v1) {
      h = (255 - v1) / 255.0;
      }
    else if(v2) {
      h = v2 / 255.0;      
      }
    if(v1 || v2)
      drawBarPos(idPos, h);      
    }
}
//------------------------------
function chgMotor(act)
{
  var mot = act[0];
  if('Z' == mot) {
    var toSend = "m=" + act + '#';
    mainLoop(toSend);    
    }
  else {
    var updn = parseInt(act[1]);
    var ix = (act.charCodeAt(0) - 'A'.charCodeAt(0)) * 2 + updn - 1;
    Motors[ix] = !Motors[ix];
    var toSend = "m=" + act + get_1_0(Motors[ix]) + '#';
    var forDebug = false;
    if(forDebug) {
      var to_ = ix & 1 ? 'm_down' : 'm_up';
      var toAdd = to_;
      var toRem = to_ + '_on';
      if(Motors[ix]) {
        toAdd = to_ + '_on';
        toRem = to_;
        }
      var obj = document.getElementById('idBtnMot' + act);
      obj.classList.remove(toRem);
      obj.classList.add(toAdd);
      }
    mainLoop(toSend);
    }
}
//------------------------------
function makeAlarmBlock(ix, title)
{
  var txt = '<td>' + title + '</td>';
  txt += '<td><div id="idAlarm' + ix + 'a" class="circle grey"><div class="glare"></div></div></td>';
  txt += '<td><input id="alarmTxt' + ix + '" type="button" value="Escludi" onClick="mainLoop(\'x=e' + ix + '\')" class="menuBtnItem" /></td>';
  txt += '<td><div id="idAlarm' + ix + 'b" class="circle grey"><div class="glare"></div></div></td>';
  txt += '<td></td>';
  var id = "idAlarm" + ix;   
  setVal(id, txt);
}
//------------------------------
function makeroll_up_shutterBlock_part(title, letter)
{
  var txt = '<td><table class="center"><tr><td rowspan="3"><h3>' + title + ': </h3></td>';
  txt += '<td>In mov.</td><td>Posiz.</td><td></td><td>Fine<br />corsa</td></tr>';
  txt += '<tr><td><div id="idMot' + letter + '1_1"   class ="circle grey"><div class="glare"></div></div></td>';
  txt += '<td rowspan="2"><canvas id="idMotPos' + letter +'" width="60" height="90"></canvas></td>';
  txt += '<td><div id="idBtnMot' + letter + '1" onClick="chgMotor(\'' + letter + '1\')" class="m_up"></div></td>';
  txt += '<td><div id="idMot' + letter + '1_3" class="circle grey"><div class="glare"></div></div></td>';

  txt += '<tr><td><div id="idMot' + letter + '2_1"   class ="circle grey"><div class="glare"></div></div></td>';
  txt += '<td><div id="idBtnMot' + letter + '2" onClick="chgMotor(\'' + letter + '2\')" class="m_down"></div></td>';
  txt += '<td><div id="idMot' + letter + '2_3" class="circle grey"><div class="glare"></div></div></td><tr></tr></table></td>';
  return txt;
}  
//------------------------------
function makeroll_up_shutterBlock(ix, letter, title)
{
  var txt = makeroll_up_shutterBlock_part(title[0], letter[0]);
  txt += makeroll_up_shutterBlock_part(title[1], letter[1]);
  var id = "idShutter" + ix;   
  setVal(id, txt);
}  
//------------------------------
function makerele(titles)
{
	var txt = '';
  var A = 'A'.charCodeAt(0);
	var tot = titles.length;
	for(var i = 0; i < tot; ++i) {
		if(!(i % 2)) 
			txt += "<tr>";
		txt += "<td>" + titles[i] + "</td><td><div id='idRele" + (i + 1) + "' class='circle grey'><div class='glare'></div></div></td>";
		txt += "<td><input type='button' value='On/Off' onClick=\"mainLoop('r=" + String.fromCharCode(A + i) + "')\" class='menuBtnItem' /></td>";
		if(!((i + 1) % 2)) 
			txt += "</tr>";
		}
	if(tot % 2)
		txt += "</tr>";
	return txt;
}	
//------------------------------
function makeHourCrono(ix)
{
	var h = parseInt(ix / 2);
	var m = ix % 2 * 30;
	return txt = '<th>' + zeroFill(h) + ':' + zeroFill(m) + '</th>';
}	
//------------------------------
function makeThCrono(ix)
{
	var txt = '<tr>';
	for(var i = 0; i < 12; ++i) 
		txt += makeHourCrono(ix + i);
	return txt + '</tr>';
}
//------------------------------
function makeTdCrono(ix)
{
	var txt = "<tr id='idRowCrono" + ix + "'>";
	for(var i = 0; i < 12; ++i) 
		txt += '<td></td>';
	return txt + '</tr>';
}
//------------------------------
function makeCrono()
{
	var txt = '';
	for(var i = 0; i < 4; ++i) {
		txt += makeThCrono(i * 12);
		txt += makeTdCrono(i + 1);
		}
  setVal("idCrono", txt);
}
//------------------------------
function makeWeekInput(day, ix)
{
	return '<input type="button" value="' + day + '" onClick="chgWeek(' + ix + ');blur();" class="menuBtnItem" />';
}
//------------------------------
function makeCronoWeek()
{
	var info = [ ["Lunedì", 1], ["Martedì", 2], ["Mercoledì", 3], ["Giovedì", 4], ["Venerdì", 5], ["Sabato", 6], ["Domenica", 7], ["Oggi", 0], ];
	var txt = '';
  for(var i = 0; i < info.length; ++i)
    txt += makeWeekInput(info[i][0], info[i][1]);
  setVal("idmenuBtn", txt);
}
//------------------------------
function init()
{
  var alarmBlock = [ [1, "Sala (sensore)(*)"], [2, "Portone ingresso (*)"], [3, "Finestra cucina"], 
      [4, "Camera (sensore)"], [5, "Camera (finestra)"], [6, "Cameretta (sensore)"], [7, "Cameretta (finestra)"],
      [8, "Finestra bagno"] ];
  for(var i = 0; i < alarmBlock.length; ++i)
    makeAlarmBlock(alarmBlock[i][0], alarmBlock[i][1]);
    
  var shutterBlock = [ [1, ['A', 'B'], ["Sala", "Cucina"]], [3, ['C', 'D'], ["Camera", "Cameretta"]] ];
  for(var i = 0; i < shutterBlock.length; ++i)
    makeroll_up_shutterBlock(shutterBlock[i][0], shutterBlock[i][1], shutterBlock[i][2]);

	var releTitles = ['Luce Sala', 'Luce Cucina', 'Luce Camera', 'Luce Cameretta', 'Balcone est', 'Balcone nord', 'Balcone ovest', 'Luce Bagno', ];
  setVal("idRele", makerele(releTitles));
	makeCrono();
	makeCronoWeek();
  fillWeekCBX();
  svCanvas = document.getElementById('cv');
  svCtx = svCanvas.getContext('2d');
  CanvasTextFunctions.enable(svCtx);
  draw_axes(drawAx_X, 'ax');
}
//------------------------------
if(window.onload) {
  var old = window.onload;
  window.onload = function() { old(); init(); acquireLoop(); }
  }
else
  window.onload = function() { init(); acquireLoop();  }
