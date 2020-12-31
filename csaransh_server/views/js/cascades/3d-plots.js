import React from 'react';
import Plot from 'react-plotly.js';
import { getColor, getPairColor, getPairColorOrient } from "../utils";

const getMinMaxFromAr = (mn, mx, ar) => {
  for (let i = 0; i < 3; i++) {
    mn[i] = Math.min(mn[i], ...ar[i]);
    mx[i] = Math.max(mx[i], ...ar[i]);
  }
  return mn, mx;
}

const cookDataLineCmp = (c) => {
  let data = [];
  let i = 0;
  let mn = [10000000000.0, 10000000000.0, 10000000000.0]; // TODO: float max
  let mx = [-10000000000.0, -10000000000.0, -10000000000.0];
  if (c['pointsV'][0].length > 0) {
    mn, mx = getMinMaxFromAr(mn, mx, c['pointsV']);
  }
  if (c['pointsI'][0].length > 0) {
    mn, mx = getMinMaxFromAr(mn, mx, c['pointsI']);
  }
  for (const x of c['lines']) {
    mn, mx = getMinMaxFromAr(mn, mx, x.main);
    data.push({
        x: x.main[0],
        y: x.main[1],
        z: x.main[2],
        mode: 'lines+markers',
        type: 'scatter3d',
        line: {
          width: 6,
          color: getPairColorOrient(x.main[4])
        },
        marker: {
          color: getPairColorOrient(x.main[4]),//'rgb(23, 190, 207)',
          size: 5
        },
        text: x.main[3],
        name: "line-" + i
    });
    if (x.sub[0].length > 0) {
      data.push({
          x: x.sub[0],
          y: x.sub[1],
          z: x.sub[2],
          mode: 'lines+markers',
          type: 'scatter3d',
          line: {
            width: 3,
            color: getPairColorOrient(x.sub[4])
          },
          marker: {
            color: getPairColorOrient(x.sub[4]),//'rgb(23, 190, 207)',
            size: 3,
            opacity: 0.6
          },
          text: x.sub[3],
          name: "sub-" + i
      });
    }
    i++;
  }
  for (const x of c['linesT']) {
    mn, mx = getMinMaxFromAr(mn, mx, x);
    data.push({
        x: x[0],
        y: x[1],
        z: x[2],
        mode: 'lines+markers',
        type: 'scatter3d',
        line: {
          width: 4,
          color: getPairColorOrient(x[4])
        },
        marker: {
          color: getPairColorOrient(x[4]),//'rgb(23, 190, 207)',
          size: 3.5,
          opacity: 0.8
        },
        text: x[3],
        name: "tline-" + i
    });
  }
  data.push({
      x: c['pointsI'][0],
      y: c['pointsI'][1],
      z: c['pointsI'][2],
      mode: 'markers',
      type: 'scatter3d',
      marker: {
        color: 'rgb(170, 200, 150)',
        size: 4.5
      },
      text: c.pointsI[3],
      name: "Non-line Is"
  });
  data.push({
      x: c['pointsV'][0],
      y: c['pointsV'][1],
      z: c['pointsV'][2],
      mode: 'markers',
      type: 'scatter3d',
      marker: {
        color: 'rgb(130, 150, 170)',
        size: 3.5
      },
      text: c.pointsV[3],
      name: "Non-line Vs"
  });
  const maxDiff = Math.max(Math.abs(mx[0] - mn[0]), Math.abs(mx[1] - mn[1]), Math.abs(mx[2] - mn[2]));
  mx[0] = mn[0] + maxDiff;
  mx[1] = mn[1] + maxDiff;
  mx[2] = mn[2] + maxDiff;
  return [data, mn, mx];
}

const cookDataCmp = (c, cmpColorIndex) => {
  var data = [{
        x: c[0],
        y: c[1],
        z: c[2],
        mode: 'markers',
        type: 'scatter3d',
        marker: {
          color: getColor(cmpColorIndex),//'rgb(23, 190, 207)',
          size: 6
        }
    }];
  return data;
}

    const layoutCmp = (mn, mx) => {
      return {
        autosize: true,
        scene: {
            aspectratio: {
                x: 1,
                y: 1,
                z: 1
            },
            camera: {
                center: {
                    x: 0,
                    y: 0,
                    z: 0
                },
                eye: {
                    x: 1.25,
                    y: 1.25,
                    z: 1.25
                },
                up: {
                    x: 0,
                    y: 0,
                    z: 1
                }
            },
            xaxis: {
                type: 'linear',
                zeroline: false,
                range: [mn[0], mx[0]],
                  title: 'x (A)',
                  titlefont: {
                    family: 'Arial, sans-serif',
                    size: 14,
                    color: 'grey'
                  },
                  tickfont: {
                    family: 'Old Standard TT, serif',
                    size: 14,
                    color: 'black'
                  },
                  dtick: Math.ceil((mx[0] - mn[0]) / 5)
                  /*
                  ticklen: 4,
                  tickwidth: 4,
                  tickcolor: '#eaa'
                  */
            },
            yaxis: {
                type: 'linear',
                zeroline: false,
                range: [mn[1], mx[1]],
                  title: 'y (A)',
                  titlefont: {
                    family: 'Arial, sans-serif',
                    size: 14,
                    color: 'grey'
                  },
                  tickfont: {
                    family: 'Old Standard TT, serif',
                    size: 14,
                    color: 'black'
                  },
                  dtick: Math.ceil((mx[1] - mn[1]) / 5)
            },
            zaxis: {
                type: 'linear',
                zeroline: false,
                range: [mn[2], mx[2]],
                  title: 'z (A)',
                  titlefont: {
                    family: 'Arial, sans-serif',
                    size: 14,
                    color: 'grey'
                  },
                  tickfont: {
                    family: 'Old Standard TT, serif',
                    size: 14,
                    color: 'black'
                  },
                  dtick: Math.ceil((mx[2] - mn[2]) / 5)
            }
        },
        margin: { l: 30, r: 10, b: 40, t: 30, pad: 1 },
      };
    };

export const ScatterLinePlot = props => {
  //let mn = Math.min(...props.coords[0], ...props.coords[1], ...props.coords[2]);
  //let mx = Math.max(...props.coords[0], ...props.coords[1], ...props.coords[2]);
  const [data, mn, mx] = cookDataLineCmp(props.coords);
  return (<Plot data={data} layout={ layoutCmp(mn, mx) }
  style={{height: "320px", width: "100%"}}
  onClick={props.clickHandler}
  useResizeHandler
/>);
}

export const ScatterCmpPlot = props => {
  let mn = Math.min(...props.coords[0], ...props.coords[1], ...props.coords[2]);
  let mx = Math.max(...props.coords[0], ...props.coords[1], ...props.coords[2]);
  return (<Plot data={cookDataCmp(props.coords, props.colorIndex)} layout={ layoutCmp(mn, mx) }
  style={{height: "320px", width: "100%"}}
  onClick={props.clickHandler}
  useResizeHandler
/>);
}

const cookData = (row) => {
  let data = [];
  let i = 0;
  for (const [cid, cindexList] of Object.entries(row.dclust_coords)) {
    let c = [[],[],[]];
    for (const cindex of cindexList) {
        c[0].push(row.eigen_coords[cindex][0]);
        c[1].push(row.eigen_coords[cindex][1]);
        c[2].push(row.eigen_coords[cindex][2]);
    }
    data.push({
        x: c[0],
        y: c[1],
        z: c[2],
        mode: 'markers',
        type: 'scatter3d',
        marker: {
          color: getColor(i),//'rgb(23, 190, 207)',
          size: 2
        },
        name:cid
    });
    data.push({
        //alphahull: 7,
        opacity: 0.2,
        type: 'mesh3d',
        color: getColor(i),
        x: c[0],
        y: c[1],
        z: c[2],
        name:cid
    });
    i++;
  }
  return data;
  /*
  var data = [{
        x: c[0],
        y: c[1],
        z: c[2],
        mode: 'markers',
        type: 'scatter3d',
        marker: {
          color: 'rgb(23, 190, 207)',
          size: 2
        }
    },{
        alphahull: 7,
        opacity: 0.1,
        type: 'mesh3d',
        x: c[0],
        y: c[1],
        z: c[2],
    }];
  return data;
  */
}
    const layout = {
        autosize: true,
        scene: {
            aspectratio: {
                x: 1,
                y: 1,
                z: 1
            },
            camera: {
                center: {
                    x: 0,
                    y: 0,
                    z: 0
                },
                eye: {
                    x: -0.001,
                    y: -0.25,
                    z: +1.25
                },
                up: {
                    x: 0,
                    y: 0,
                    z: 1
                }
            },
            xaxis: {
                type: 'linear',
                zeroline: false
            },
            yaxis: {
                type: 'linear',
                zeroline: false
            },
            zaxis: {
                type: 'linear',
                zeroline: false
            }
        },
        margin: { l: 30, r: 10, b: 40, t: 30, pad: 1 },
    };

export const ClusterPlot = props => 
  <Plot data={cookData(props.row)} layout={ layout }
  style={{height: "320px", width: "100%"}}
  useResizeHandler
  />;

const cookDataScatter = (c) => {
var trace1 = {
	x:c[0][0], y: c[0][1], z: c[0][2],
	mode: 'markers',
	marker: {
		size: 5,
		line: {
        color: 'rgba(22, 22, 212, 0.14)',
		line: {
		color: 'rgb(12, 22, 222)',
		width: 0.1},
		width: 0.5},
		opacity: 0.8},
    name: "interstitial",
	type: 'scatter3d'
};

var trace2 = {
	x:c[1][0], y: c[1][1], z: c[1][2],
	mode: 'markers',
	marker: {
		color: 'rgb(127, 57, 57)',
		size: 5,
		symbol: 'circle',
		line: {
		color: 'rgb(204, 22, 22)',
		width: 1},
        opacity: 0.6},
    name: "vacancy",
	type: 'scatter3d'};

return [trace1, trace2];
}

export const ScatterPlot = props => 
  <Plot data={cookDataScatter(props.coords)} layout={ layout }
  style={{height: "320px", width: "100%"}}
  useResizeHandler
  />;

// ===============

const cookDataClasses = (classData) => {
  var traces = [];
  let i = 0;
  let labelsOrdered = Object.keys(classData);
  labelsOrdered.sort();
  for (const classLabel of labelsOrdered) {
    traces.push(
      {
        x: classData[classLabel][0],
        y: classData[classLabel][1],
        z: classData[classLabel][2],
        mode: 'markers',
        type: 'scatter3d',
        name: classLabel,
        marker: {
          color: (classLabel == "noise") ? 'rgb(220,220,220)' : getColor(i++),
          size: 2
        }
      }
    )
  }
  return traces;
}

export class ClassesPlot extends React.Component {
  constructor(props) {
    super(props);
  }

  shouldComponentUpdate(nextProps, nextState) {
      return nextProps.mode != this.props.mode;
  }

  render() {
    const props = this.props; 
    return (
      <Plot data={cookDataClasses(props.coords)} layout={ layout }
      style={{height: "320px", width: "100%"}}
      onClick={props.clickHandler}
      useResizeHandler
    />
    );
  }
}

