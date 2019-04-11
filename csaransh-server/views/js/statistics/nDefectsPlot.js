import React from 'react';
import Plot from 'react-plotly.js';
import { getColor } from "../utils";

export const groupBarsdaf = (data) => {
  console.log("calculating stats");
  let keys = [];
  let keys2 = [];
  let keysCluster = [];
  let keysCluster2 = [];
  /* n_defects = []; max_sizeI = []; max_sizeV = []; in_clusterI = []; in_clusterV = [];  */
  const names = ["defects count", "max int cluster", "max vac size", "int in cluster", 
                 "vac in cluster", "subcascades", "subcasd impact", "eigen dim1",
                 "eigen dim2", "eigen dim1+2", "cluster-sizes", "cluster dim1", "cluster dim1+2", "energy"];
  let vals = [{label:names[0], values:[]}, {label:names[1], values:[]}, {label:names[2], values:[]},{label:names[3], values:[]},
              {label:names[4], values:[]}, {label:names[5], values:[]}, {label:names[6], values:[]},{label:names[7], values:[]},
              {label:names[8], values:[]}, {label:names[9], values:[]}, {label:names[10], values:[]}, {label:names[11], values:[]},
              {label:names[12], values:[]}, {label:names[13], values:[]}];
  //let valsSplom = vals.slice();

  let mx = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0];
  let mn = [100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0]; // valid assumption
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
    for (const clusterId in x.eigen_features) {
      keysCluster.push(x.substrate + "_" + x.energy);
      keysCluster2.push(x.name);
      /*
      valsSplom[0].values.push(x.n_defects);
      valsSplom[1].values.push(x.max_cluster_size_I);
      valsSplom[2].values.push(x.max_cluster_size_V);
      valsSplom[3].values.push(x.in_cluster_I);
      valsSplom[4].values.push(x.in_cluster_V);
      valsSplom[5].values.push(x.density_cluster_vac.length); // s
      valsSplom[6].values.push(parseInt(x.density_cluster_vac.length <= 1 ? 0 : x.clusters[x.density_cluster_vac[1]].length * 100 / x.clusters[x.density_cluster_vac[0]].length));
      valsSplom[7].values.push(x.eigen_var[0]);
      valsSplom[8].values.push(x.eigen_var[1]);
      valsSplom[9].values.push(x.eigen_var[0] + x.eigen_var[1]);
      */
      vals[10].values.push(x.clusters[clusterId].length);
      vals[11].values.push(x.eigen_features[clusterId].var[0]);
      vals[12].values.push(1.0 - x.eigen_features[clusterId].var[2]);
      /*
      valsSplom[10].values.push(x.clusters[clusterId].length);
      valsSplom[11].values.push(x.eigen_features[clusterId].var[0]);
      valsSplom[12].values.push(1.0 - x.eigen_features[clusterId].var[2]);
      valsSplom[13].values.push(x.energy);
      */
      const lastElem = vals[10].values.length - 1;
      for (let i = 10; i < 13; ++i) {
        mn[i] = Math.min(mn[i], vals[i].values[lastElem]);
        mx[i] = Math.max(mx[i], vals[i].values[lastElem]);
      }
      //break;
    }
    vals[13].values.push(x.energy);
    const lastElem = vals[0].values.length - 1;
    for (let i = 0; i < mn.length; ++i) {
      if (i == 10 || i == 11 || i == 12) continue;
      mn[i] = Math.min(mn[i], vals[i].values[lastElem]);
      mx[i] = Math.max(mx[i], vals[i].values[lastElem]);
    }
  }
  //console.log(vals[10].values.length);
  //console.log(vals[0].values.length);
  let d1 = [];
  let i = 0;

  for (let val of vals) {
    if (i == vals.length - 1) break;
    d1.push(
      {
        y: val.values.slice(),
        x: (i == 10 || i == 11 || i == 12) ? keysCluster : keys,
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
  /*
  for (let i = 0; i < valsSplom.length; ++i) {
    for (let j in valsSplom[i].values) {
      valsSplom[i].values[j] = (valsSplom[i].values[j] - mn[i]) / (mx[i] - mn[i]);
    }
  }
  */
  return d1;//[valsSplom, d1, keysCluster2];
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
            args: ['visible', [true, false, false, false, false, false, false, false, false, false, false, false, false]],
            label: 'Number of defects'
        }, {
            method: 'restyle',
            args: ['visible', [false, true, false, false, false, false, false, false, false, false, false, false, false]],
            label: 'Max int cluster sizes'
        }, {
            method: 'restyle',
            args: ['visible', [false, false, true, false, false, false, false, false, false, false, false, false, false]],
            label: 'Max vac cluster sizes'
        }, {
            method: 'restyle',
            args: ['visible', [false, false, false, true, false, false, false, false, false, false, false, false, false]],
            label: 'Int in clusters'
        }, {
            method: 'restyle',
            args: ['visible', [false, false, false, false, true, false, false, false, false, false, false, false, false]],
            label: 'Vac in clusters'
        }, {
            method: 'restyle',
            args: ['visible', [false, false, false, false, false, true, false, false, false, false, false, false, false]],
            label: 'Subcascades'
        }, {
            method: 'restyle',
            args: ['visible', [false, false, false, false, false, false, true, false, false, false, false, false, false]],
            label: 'Impact of first subcascade'
        }, {
            method: 'restyle',
            args: ['visible', [false, false, false, false, false, false, false, true, false, false, false, false, false]],
            label: 'Variance of 1st Eigen Dim'
        }, {
            method: 'restyle',
            args: ['visible', [false, false, false, false, false, false, false, false, true, false, false, false, false]],
            label: 'Variance of 2nd Eigen Dim'
        }, {
            method: 'restyle',
            args: ['visible', [false, false, false, false, false, false, false, false, false, true, false, false, false]],
            label: 'Variance 1st + 2nd Eigen Dim'
        }, {
            method: 'restyle',
            args: ['visible', [false, false, false, false, false, false, false, false, false, false, true, false, false]],
            label: 'Cluster sizes'
        }, {
            method: 'restyle',
            args: ['visible', [false, false, false, false, false, false, false, false, false, false, false, true, false]],
            label: 'Cluster variances 1st Eigen Dim'
        }, {
            method: 'restyle',
            args: ['visible', [false, false, false, false, false, false, false, false, false, false, false, false, true]],
            label: 'Cluster Variances 1st + 2nd Eigen Dim'
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