import React from 'react';
//import { getColor } from "./utils";
import Plot from 'react-plotly.js';


const getColor = i => (i === 1) ? "#bebada" : "#8dd3c7";

export const calcStatDistsAngles = (rows) => {
  let dists = {};
  let angles = {};
  for (const row of rows) {
    const name = row.substrate + "-" + row.energy;
    if (!dists.hasOwnProperty([name])) {
      dists[name] = [row.distancesI, row.distancesV];
      angles[name] = [row.anglesI, row.anglesV];
    } else {
      dists[name][0] = dists[name][0].concat(row.distancesI);
      dists[name][1] = dists[name][1].concat(row.distancesV);
      angles[name][0] = angles[name][0].concat(row.anglesI);
      angles[name][1] = angles[name][1].concat(row.anglesV);
    }
  }
  let tracesDists = [];
  let tracesAngles = [];
  let showLegend = true;
  for (const name in dists) {
    tracesDists.push(getTrace(dists[name][0], name, "Interstitial", 1, showLegend, "positive"));
    tracesDists.push(getTrace(dists[name][1], name, "Vacancy", 0, showLegend, "negative"));
    tracesAngles.push(getTrace(angles[name][0], name, "Interstitial", 1, showLegend, "positive"));
    tracesAngles.push(getTrace(angles[name][1], name, "Vacancy", 0, showLegend, "negative"));
    showLegend = false;
  }
  //console.log(traceDists);
  return [tracesDists, tracesAngles];
}

function getTrace(row, name, group, color, showLegend, side) {
  return {
            //hoveron: "kde",
            meanline: {
              visible: true
            },
            legendgroup: group,
            scalegroup: name + group,
            points: "none",
            pointpos: 0.0,
            box: {
                visible: true
            },
            jitter: 0.1,
            scalemode: "count",
            marker: {
                line: {
                    width: 1,
                    color: getColor(color)
                },
                symbol: "line-ns"
            },
            showlegend: showLegend,
            side: side,
            type: "violin",
            name: group,
            span: [
                0
            ],
            line: {
                color: getColor(color)
            },
            y0: name/* + curDefDist.length*/,
            x: row,
            orientation: "h"
        };
}

//export function getTrace(row, name, group, color, showLegend, side);
export function addDefectAngles(curData1, row, limit) {
  for(const x of curData1) {
    if (x.y0 == row.name) return curData1;
  }
  let curData;
  if (curData1.length > limit + 2) {
    const nm = curData1[0].y0;
    curData = curData1.filter(x => x.y0 != nm);
  } else {
    curData = curData1.slice();
  }
  const trace1 = getTrace(row.anglesI, row.name, "Interstitial", 1, (curData.length === 0), "positive");
  const trace2 = getTrace(row.anglesV, row.name, "Vacancy", 0, (curData.length === 0), "negative");
  //let defDist = curData.slice();
  curData.push(trace1);
  curData.push(trace2);
  return curData;
}

export function addDefectDistance(curData1, row, limit) {
  for(const x of curData1) {
    if (x.y0 == row.name) return curData1;
  }
  let curData;
  if (curData1.length > limit + 2) {
    const nm = curData1[0].y0;
    curData = curData1.filter(x => x.y0 != nm);
  } else {
    curData = curData1.slice();
  }
  const trace1 = getTrace(row.distancesI, row.name, "Interstitial", 1, (curData.length === 0), "positive");
  var trace2 = getTrace(row.distancesV, row.name, "Vacancy", 0, (curData.length === 0), "negative");
  //let defDist = curData.slice();
  curData.push(trace1);
  curData.push(trace2);
  return curData;
}

export function removeDefectDistance(curData, row) {
  return curData.filter(x => x.y0 != row.name);
}

export class DefectDistancePlot extends React.Component {
  constructor(props) {
    super(props);
    this.child = React.createRef();
  }

  render() {
    if (this.child && this.child.current) this.child.current.resizeHandler();
    return (
      <div>
      <Plot
        ref={this.child}
        data= {this.props.data}
        layout = {{
          title: this.props.title,
          hovermode: "closest",
          yaxis: {
              showgrid: true
          },
          xaxis: {
              title: this.props.xlabel,
          },
          legend: {
              tracegroupgap: 0
          },
          violingap: 0,
          violingroupgap: 0,
          violinmode: "overlay",
          margin: { l: 100, r: 20, b: 55, t: 50, pad: 2 }
        }}

        style={{height: this.props.height, width: "95%"}}
        //style={{height: this.props.data.length == 0 ? "276px" : 276 * (this.props.data.length / 2) + "px", width: "90%", 
        //      }}
        config={{
          'displayModeBar': false
        }}
        useResizeHandler
      />
      </div>
    );
  }
}
/*

Plotly.Plot.resize()
*/