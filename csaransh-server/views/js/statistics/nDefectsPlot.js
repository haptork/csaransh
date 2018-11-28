import React from 'react';
import { getColor } from "../utils";
import Plot from 'react-plotly.js';

const averageByKey = (data, key, value) => {
  let res = {};
  for (const x of data) {
    if (!res.hasOwnProperty(x[key])) res[x[key]] = [0.0, 0];
    res[x[key]][0] += x[value];
    res[x[key]][1]++;
  }
  for (const k in res) {
    res[k][0] = res[k][0] / res[k][1];
  }
  let err = {}
  for (const x of data) {
    if (!res.hasOwnProperty(x[key])) err[x[key]] = 0.0;
    err[x[key]] += (x[value] - res[x[key]][0]) * (x[value] - res[x[key]][0]);
  }
  let keys = [];
  let avg = [];
  let stddev = [];
  for (const k in res) {
    keys.push(k);
    avg.push(res[k][0])
    stddev.push(Math.sqrt(err[k]/(res[k][1] - 1)));
  }
  return [keys, avg, stddev];
};

export const groupBars = (data) => {
  let keys = [];
  let keys2 = [];
  /* n_defects = []; max_sizeI = []; max_sizeV = []; in_clusterI = []; in_clusterV = [];  */
  const names = ["defects count", "max int cluster", "max vac size", "int in cluster", "vac in cluster", "subcascades", "subcasd impact", "eigen dim1", "eigen dim2", "eigen dim1+2", "energy"];

  let vals = [{label:names[0], values:[]}, {label:names[1], values:[]}, {label:names[2], values:[]},{label:names[3], values:[]},
              {label:names[4], values:[]}, {label:names[5], values:[]}, {label:names[6], values:[]},{label:names[7], values:[]},
              {label:names[8], values:[]}, {label:names[9], values:[]}, {label:names[10], values:[]}];

  let mx = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0];
  let mn = [100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0]; // valid assumption
  for (const x of data) {
    keys.push(x.substrate + "_" + x.energy);
    keys2.push(x.name);
    vals[0].values.push(x.n_defects);
    vals[1].values.push(x.max_cluster_size_I);
    vals[2].values.push(x.max_cluster_size_V);
    vals[3].values.push(x.in_cluster_I);
    vals[4].values.push(x.in_cluster_V);
    vals[5].values.push(x.density_cluster_vac.length); // s
    vals[6].values.push(parseInt(x.density_cluster_vac.length <= 1 ? 0 : x.clusters[x.density_cluster_vac[1]].length * 100 / x.clusters[x.density_cluster_vac[0]].length));
    vals[7].values.push(x.eigen_var[0]);
    vals[8].values.push(x.eigen_var[1]);
    vals[9].values.push(x.eigen_var[0] + x.eigen_var[1]);
    vals[10].values.push(x.energy);
    const lastElem = vals[0].values.length - 1;
    for (let i = 0; i < mn.length; ++i) {
      mn[i] = Math.min(mn[i], vals[i].values[lastElem]);
      mx[i] = Math.max(mx[i], vals[i].values[lastElem]);
    }
  }
  let d1 = [];
  let i = 0;

  for (let val of vals) {
    if (i == 10) break;
    d1.push(
      {
        y: val.values.slice(),
        x: keys,
        visible: (i == 0),
        name: names[i],
        marker: {color: getColor(i)},
        type: 'box',
        meanline: {
          visible: true
        },
        boxmean: true,
        jitter: 0.3,
        pointpos: -1.5,
        boxpoints: 'all'
      }
    );
    i++;
  }
  // normalization
  for (let i = 0; i < vals.length; ++i) {
    for (let j in vals[i].values) {
      vals[i].values[j] = (vals[i].values[j] - mn[i]) / (mx[i] - mn[i]);
    }
  }
  return [vals, d1, keys2];
};

export class NDefectsPlot extends React.Component {
  constructor(props) {
    super(props);
  }

  render() {
    return (
      <Plot
        data= {this.props.data}
        layout = {{
          //title: this.props.title,
          hovermode: "closest",
          yaxis: {
              //showgrid: true
              zeroline: false
          },
          boxmode: "group",
          margin: { l: 30, r: 10, b: 35, t: 30, pad: 1 },
         updatemenus: [{
        y: 1.05,
        x: 0.11,
        yanchor: 'top',
        buttons: [{
            method: 'restyle',
            args: ['visible', [true, false, false, false, false, false, false, false, false, false]],
            label: 'Number of defects'
        }, {
            method: 'restyle',
            args: ['visible', [false, true, false, false, false, false, false, false, false, false]],
            label: 'Max int cluster sizes'
        }, {
            method: 'restyle',
            args: ['visible', [false, false, true, false, false, false, false, false, false, false]],
            label: 'Max vac cluster sizes'
        }, {
            method: 'restyle',
            args: ['visible', [false, false, false, true, false, false, false, false, false, false]],
            label: 'Int in clusters'
        }, {
            method: 'restyle',
            args: ['visible', [false, false, false, false, true, false, false, false, false, false]],
            label: 'Vac in clusters'
        }, {
            method: 'restyle',
            args: ['visible', [false, false, false, false, false, true, false, false, false, false]],
            label: 'Subcascades'
        }, {
            method: 'restyle',
            args: ['visible', [false, false, false, false, false, false, true, false, false, false]],
            label: 'Impact of first subcascade'
        }, {
            method: 'restyle',
            args: ['visible', [false, false, false, false, false, false, false, true, false, false]],
            label: 'Variance of 1st Eigen Dim'
        }, {
            method: 'restyle',
            args: ['visible', [false, false, false, false, false, false, false, false, true, false]],
            label: 'Variance of 2nd Eigen Dim'
        }, {
            method: 'restyle',
            args: ['visible', [false, false, false, false, false, false, false, false, false, true]],
            label: 'Variance 1st + 2nd Eigen Dim'
        }
      ]
    }]

       }}
        style={{height: "460px", width: "100%"}}
        useResizeHandler
      />
    );
  }
}
/*

Plotly.Plot.resize()
*/