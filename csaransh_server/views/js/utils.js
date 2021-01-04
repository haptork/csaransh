/*jshint esversion: 6 */
import React from "react";
//import { data } from "./cascades-data"
import Palette from "./palette";

export const accessorsAndParseFns = {
  "accessorDefault" : name => x => x[name],
  "accessorTwod" : x => (parseFloat(x["eigen_var"][0]) + parseFloat(x["eigen_var"][1])) * 100,
  "accessorSubc": x => (Object.keys(x.dclust_coords).length) <= 1 ? 0 : (Object.keys(x.dclust_coords).length),
  "roundOff" : x => +parseFloat(x).toFixed(2),
  "noParse" : x => x,
  "boolParse" : x => (x.toLowerCase() == "true" || x.toLowerCase() == "yes"),
  "parseVol" : x => parseInt(x / 1000),
  "parseFileName" : x => x.split('\\').pop().split('/').pop()
};

const ac = accessorsAndParseFns;
// all inputs first followed by outputs. short name depends on this
export const getAllCol = () => {
  const res = [
    { value: 'substrate', label: 'Material', isShow: true, filterType: "select", type:"input", parseFn: ac.noParse},
    { value: 'energy', label: 'Energy (keV)', isShow: true, filterType: "select", type:"input"},
    { value: 'temperature', label: 'Temperature', isShow: false, filterType: "select", type:"input"},
    { value: 'simulationTime', label: 'Simulation Time', isShow: false, filterType: "range", type: "input"},
    { value: 'author', label: 'Author', isShow: false, filterType: "select", type: "input", parseFn: ac.noParse},
    { value: 'potentialUsed', label: 'Potential Used', isShow: true, filterType: "select", type: "input" , parseFn: ac.noParse},
    { value: 'es', label: 'Electronic Stopping', isShow: false, filterType: "select", type: "input" , parseFn: ac.noParse},
    { value: 'xyzFilePath', label: 'Xyz file path', isShow: false, filterType: "text" , type: "input", parseFn: ac.noParse},
    { value: 'xyzFileName', label: 'Xyz file name', isShow: false, filterType: "text" , type: "input", accessor: ac.accessorDefault("xyzFilePath"), parseFn: ac.parseFileName},
    { value: 'infile', label: 'Input File', isShow: false, filterType: "text" , type: "input", parseFn: ac.noParse},
    { value: 'infileName', label: 'Input File Name', isShow: false, filterType: "text" , type: "input", accessor: ac.accessorDefault("infile"), parseFn: ac.parseFileName},
    { value: 'tags', label: 'Tags', isShow: false, filterType: "text" , type: "input", parseFn: ac.noParse},
    { value: 'rectheta', label: 'PKA angle - theta', isShow: false, filterType: "range" , type: "input", parseFn: ac.roundOff},
    { value: 'recphi', label: 'PKA angle - phi', isShow: false, filterType: "range" , type: "input", parseFn: ac.roundOff},
    { value: 'n_defects', label: 'Defects Count', isShow: true },
    { value: 'max_cluster_size', label: 'Max cluster size', isShow: false },
    { value: 'in_cluster', label: '% defects in cluster', isShow: false },
    { value: 'n_clusters', label: 'Clusters Count', isShow: false },
    { value: 'hull_vol', label: 'Volume of cascade hull', isShow: false},
    { value: 'twod', label: 'Planarity', isShow: false, parseFn: parseInt, "accessor": ac.accessorTwod },
    { value: 'subc', label: 'Subcascades', isShow: false, parseFn: parseInt, "accessor": ac.accessorSubc },
    { value: 'dclust_sec_impact', label: 'Impact fo 2nd big subcascade', isShow: false },
    { value: 'max_cluster_size_I', label: 'Interstitial max cluster size', isShow: false },
    { value: 'max_cluster_size_V', label: 'Vacancy max cluster size', isShow: false },
    { value: 'in_cluster_I', label: '% interstitials in cluster', isShow: false },
    { value: 'in_cluster_V', label: '% vacancies in cluster', isShow: false },
    { value: 'hull_density', label: 'Density of cascade hull', isShow: true, parseFn: ac.roundOff },
    { value: 'hull_nsimplices', label: 'Hull simplices', isShow: false },
  ];
  const defaultParse = parseInt;
  const defaultType = "output";
  const defaultFilterType = "range";
  const both = (f, g) => x => f(g(x));
  for (const x of res) {
   const key = x.value;
   if (!x.hasOwnProperty("type")) {
     x.type = defaultType;
   }
   if (!x.hasOwnProperty("filterType")) {
     x.filterType = defaultFilterType;
   }
   if (x.hasOwnProperty("accessor")) {
     if (x.hasOwnProperty("parseFn")) {
       x.accessor = both(x.parseFn, x.accessor);
     }
   } else {
     if (!x.hasOwnProperty("parseFn")) x.parseFn = defaultParse;
     x.accessor = both(x.parseFn, ac.accessorDefault(key));
   }
  }
  return res;
};

export const getData = () => window.cascades.data;

export const getClassData = (curMode) => window.cluster_classes[curMode];

export const uniqueKey = (row) => row.id + row.xyzFilePath;

export const uniqueName = (row) => row.id + "-" + row.substrate + "-" + row.energy;

export const groupByKey = (row) => row.substrate + "-" + row.energy;

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

const reds = ['#fbbaba',	'#ff9191',	'#f97a7a',	'#ff6a6a'];
const greens = ['#94ce98', '#61af66', '#388e3e', '#1b7021', '#064e0a'];
const oranges = ["#ff9a00", "#fff400", "#ffdb00", "#ffc100", "#ff8100"];
const slate = ["#515a5e", "#60666d", "#41484d", "#5e5e65", "#3a3d45"];

export const getColor = i => colors[(i) % colors.length];

export const getPairColor = x => {
  if (x == undefined) return slate[0];
  if (x[0] == 0) return greens[(x[1]) % (greens.length)];
  if (x[0] == 1) return reds[(x[1]) % (reds.length)];
  if (x[0] == 2 || x[0] == 3) return oranges[(x[1]) % (oranges.length)];
  return slate[(x[1]) % (slate.length)];
}

const componentToHex = c => {
  var hex = c.toString(16);
  return hex.length == 1 ? "0" + hex : hex;
}

const rgbToHex = (r, g, b) => {
  return "#" + componentToHex(r) + componentToHex(g) + componentToHex(b);
}
/*
export const getPairColorOrient = x => {
  if (x == undefined) return slate[0];
  const r = Math.round(255 * x[0]);
  const g = Math.round(255 * x[1]);
  const hexCode = rgbToHex(r , g, 100);
  //console.log("" + r + ", " + g + ", " + hexCode);
  return hexCode;
}
*/
export const getPairColorOrient = x => {
  if (x == undefined) return slate[0];
  const i = Math.round(200 * x[0]);
  const j = Math.round(200 * x[1]);
  //const hexCode = rgbToHex(120, j, 50 + i);
  const hexCode = rgbToHex(120, i, (255 - j));
  //console.log(" " + i + ", " + j + ", " + hexCode);
  return hexCode;
}

export const getColorGrad = (i, max) => "#" + Palette("tol-dv", max)[(i - 1)];