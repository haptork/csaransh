import React from "react";
//import { data } from "./cascades-data"
import Palette from "./palette";

export function getData() {
  return window.cascades.data;
}

export const exportToJson = (objectData) => {
  let filename = "export.json";
  let contentType = "application/json;charset=utf-8;";
  if (window.navigator && window.navigator.msSaveOrOpenBlob) {
    var blob = new Blob([decodeURIComponent(encodeURI(JSON.stringify(objectData)))], { type: contentType });
    navigator.msSaveOrOpenBlob(blob, filename);
  } else {
    var a = document.createElement('a');
    a.download = filename;
    a.href = 'data:' + contentType + ',' + encodeURIComponent(JSON.stringify(objectData));
    a.target = '_blank';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
  }
}

function loadJSON(callback) {   
  var xobj = new XMLHttpRequest();
      xobj.overrideMimeType("application/json");
  xobj.open('GET', 'cascades-data_py.json', false); // Replace 'my_data' with the path to your file
  xobj.onreadystatechange = function () {
        if (xobj.readyState == 4 && xobj.status == "200") {
          // Required use of an anonymous callback as .open will NOT return a value but simply returns undefined in asynchronous mode
          callback(xobj.responseText);
        }
  };
  xobj.send(null);  
}


export const toXyzArSplit = (data, onlySurviving = true) => {
  const coords = data.coords;
  const eigen_coords = data.eigen_coords;
  let inter = [[],[], []];
  let vac = [[],[], []];
  for (const i in  coords) {
    if (onlySurviving && coords[i][5] == 0) continue;
    if (coords[i][3] === 1) {
      inter[0].push(eigen_coords[i][0]);
      inter[1].push(eigen_coords[i][1]);
      inter[2].push(eigen_coords[i][2]);
    } else {
      vac[0].push(eigen_coords[i][0]);
      vac[1].push(eigen_coords[i][1]);
      vac[2].push(eigen_coords[i][2]);
    }
  }
  let res = [[], [], []];
  for (const i in res) {
    res[i] = [].concat(inter[i], vac[i]);
  }
  return [inter, vac, res];
};

export const toXyzAr = (coords) => {
  let co = [[],[], []];
  for (const c of coords) {
      co[0].push(c[0]);
      co[1].push(c[1]);
      co[2].push(c[2]);
  }
  return co;
};


export const Logo = () =>
  <div style={{ margin: '1rem auto', display: 'flex', flexWrap: 'wrap', alignItems: 'center', justifyContent: 'center'}}>
    Work in Progress {''}
  </div>;

export const Tips = () =>
  <div style={{ textAlign: "center" }}>
    <em>Tip: Hold shift when sorting to multi-sort!</em>
  </div>;

export const clusterCent = x => {
  const d = x.coords;
  let single = 0;
  let clustered = 0;
  for (const v of d) {
    if (!v[5]) continue;
    if (v[4] == 0) ++single;
    else ++clustered;
  }
  const res = (clustered * 100) / (single + clustered);
  return res.toFixed(2);
};
/*
export const clusterSizes = x => {
  var frequency = {};
  var d = x.coords;
  for (var v in d) {
    if (d[v][5] == 1) frequency[d[v][4]] = (frequency[d[v][4]] || 0) + 1;
  }
  var ar = [];
  for (var cId in frequency) {
    if (cId == 0) continue;
    ar.push(frequency[cId]); 
  }
  return ar;
}
*/

const colors = [
  "#d11141", "#00b159", "#00aedb", "#f37735", "#ffc425", "#d696bb", "#84bac7", "#c3cb71", "#bb95ff",
  "#b2b081", "#22c1ba", "#5cb85c", "#ae5a41", "#cae3e0", "#1dea5a",
  '#4dc9f6', '#f67019', '#f53794', '#537bc4', '#acc236', '#166a8f', '#00a950', '#58595b', '#8549ba',
];

export const getColor = i => colors[(i) % colors.length];

export const getColorGrad = (i, max) => "#" + Palette("tol-dv", max)[(i - 1)];