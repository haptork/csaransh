import React from 'react';
import {Bar} from 'react-chartjs-2';
import { getColorGrad, uniqueKey } from "../utils";

const calcDefectSizeDistribution = (row) => {
  var frequency = {};
  var coords = row.coords;
  var singleIs = 0;
  var singleVs = 0;
  for (const v of coords) {
    if (!v[5]) continue;
    if (v[4] == 0) {
      if (v[3] == 1) singleIs++;
      if (v[3] == 0) singleVs++;
    }
    frequency[v[4]] = (frequency[v[4]] || 0) + (v[3] == 1 ? 1 : -1);
  }
  var Ints = {};
  var Vacs = {};
  for (var cId in frequency) {
    if (cId == 0) continue;
    let x = frequency[cId];
    if (x == 1 || x == -1) {
      if (x == 1) singleIs++;
      if (x == -1) singleVs++;
      continue;
    }
    if(x > 0) {
      Ints[x] = (Ints[x] || 0) + 1;
    } else if (x < 0) {
      x = x * -1;
      Vacs[x] = (Vacs[x] || 0) + 1;
    }
  }
  var res = [[["1"], [singleIs]], [["1"], [singleVs]]];
  for (var size in Ints) {
    res[0][0].push(size);
    res[0][1].push(Ints[size] * size);
  }
  for (var size in Vacs) {
    res[1][0].push(size);
    res[1][1].push(Vacs[size] * size);
  }
  return res;
}

  const cookLabel = (i, _) => "Size " + i;

  const findLabel = (size1, type1, ar, type) => {
    for (const i in ar) {
      if (size1 == ar[i] && type1 == type) return i;
    }
    return -1;
  };

  const addNew = (ar, size, type) => {
    let res = true;
    let found = 0;
    for (const x of ar) {
      if (x.clusterSize == size) {
        found++;
        if (x.stack == type) res = false;
      }
    }
    return [res, found];
  };

  const cookStackName = type => type;

  function addToDataSet(newDataSets, d, type, len) {
    let isFound = false;
    let stack = cookStackName(type);
    for (let dict of newDataSets) {
      if (dict.stack != type) continue;
      let i = findLabel(dict.clusterSize, dict.stack, d[0], type);
      if (i < 0) dict.data.push(0);
      else dict.data.push(d[1][i])
    }
    for (const i in d[0]) {
      let xy = addNew(newDataSets, d[0][i], type);
      let x = xy[0]; let y = xy[1];
      if (x) {
        let label = cookLabel(d[0][i], type);
        if (y == 1) {
          label = "f" + label;
        }
        newDataSets.push({
          label: label,
          stack: stack,
          data: [...(Array(len).fill(0)), d[1][i]],
          clusterSize: d[0][i],
        })
      }
    }
  }

  function addedDataSets(curDef, row) {
    const d = calcDefectSizeDistribution(row);
    let newDataSets = curDef.datasets.slice();
    if (!curDef.datasets || curDef.datasets.length == 0) {
      for (const i in d[0][0]) {
        newDataSets.push({
            label: cookLabel(d[0][0][i], 1),
            stack: cookStackName(1),
            data: [d[0][1][i]],
            clusterSize: d[0][0][i]
        });
      }
      addToDataSet(newDataSets, d[1], 0, curDef.labels.length);
    } else {
      addToDataSet(newDataSets, d[0], 1, curDef.labels.length);
      addToDataSet(newDataSets, d[1], 0, curDef.labels.length);
    }
    newDataSets.sort((a, b) => {
      return a.clusterSize - b.clusterSize;
    });
    let maxSize = 1;
    for (let data of newDataSets) {
      maxSize = Math.max(data.clusterSize, maxSize);
    }
    for (let data of newDataSets) {
      data.backgroundColor = getColorGrad(data.clusterSize, maxSize);
    }
    return newDataSets;
  }

  export function addDefectsSizeDistrib(curDef, row) {
    return {
      labels: [...curDef.labels, ["int | vac", uniqueKey(row)]],
      datasets : addedDataSets(curDef, row)
    };
  }


  export function lookDefectsSizeDistrib(curDe, row, limitSize) {
    let curDef = curDe;
    const cookDataLabel = name => ["int | vac", name];
    if (curDef.labels.length === limitSize) {
      //console.log("removing");
      curDef = removeDefectsSizeDistrib(curDe, {name:curDef.labels[0][1]});
    }
    //console.log(curDef.labels);
    return {
      labels: [...curDef.labels, cookDataLabel(row.id)],
      datasets : addedDataSets(curDef, row)
    };
  }

  export function removeDefectsSizeDistrib(curDef, row) {
    let toDel = 0;
    for (const i in curDef.labels) {
      if (curDef.labels[i][1] == uniqueKey(row)) {
        toDel = i;
        break;
      }
    }
    let labels = curDef.labels.slice();
    labels.splice(toDel, 1);
    let datasets = curDef.datasets.slice();
    let empty = [];
    for (const i in datasets) {
      datasets[i].data.splice(toDel, 1);
      let allZero = true;
      for (let y of datasets[i].data) {
        if (y > 0) allZero = false;
      }
      if (allZero) empty.push(i);
    }
    for (let x = empty.length - 1; x >= 0; x--) {
      datasets.splice(empty[x], 1);
    }
      return {
        labels,
        datasets
      }
  }

export class ClusterSizePlot extends React.Component {
  constructor(props) {
    super(props);
  }

  renderActionButtons(cellInfo) {
    return (
     <div>
     </div>
    );
  }

  render() {
    return (
        <Bar data={this.props.data} 
          height={this.props.height}
          //width={this.props.height}
        	options={{ 
		        maintainAspectRatio: false,
            scales: {
              yAxes:[{
                ticks: {
                 fontStyle: "bold",
                }
              }],
              xAxes:[{
                ticks: {
                 fontStyle: "bold"
                }
              }
              ]
            },
          tooltips: {
						mode: 'index',
						intersect: false
          },
          legend: { 
            onClick : undefined,
            labels : {
            filter: (leg, data) => {
              //console.log(data);
              if (leg.text[0] == "f") return false;
              return true;
            },
            boxWidth: 15
          }},
          tooltips: {
            callbacks: {
              title: function() {
                return '';
              },
              label: function(item, data) {
                let i = item.datasetIndex;
                let sp = data.datasets[i].stack == 1 ? "Interstitial" : "Vacancy";
                let sz = data.datasets[i].clusterSize;
                return sp + " size-" + sz + ": " + item.yLabel / sz; 
              }
            }
          }

        }}
        />
    );
  }
}