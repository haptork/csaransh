import React from 'react';
import Plot from 'react-plotly.js';
import { getColor } from "../utils";

/*
const cookDataSubC = (c) => {
  var data = [{
        x: c[2][0],
        y: c[2][1],
        z: c[2][2],
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
        x: c[2][0],
        y: c[2][1],
        z: c[2][2],
    }];
  return data;
}
    const layoutSubC = {
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
        margin: { l: 30, r: 10, b: 35, t: 30, pad: 1 },
    };

export const SubCPlot = props => 
  <Plot data={cookDataSubC(props.row)} layout={ layout }
  style={{height: "320px", width: "100%"}}
  useResizeHandler
  />;
*/
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
                range: [mn, mx]
            },
            yaxis: {
                type: 'linear',
                zeroline: false,
                range: [mn, mx]
            },
            zaxis: {
                type: 'linear',
                zeroline: false,
                range: [mn, mx]
            }
        },
        margin: { l: 30, r: 10, b: 40, t: 30, pad: 1 },
      };
    };

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
  for (const cid of row.density_cluster_vac) {
    let c = [[],[],[]];
    for (const cindex of row.clusters[cid]) {
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

const cookDataClasses = (c) => {
  var traces = [];
  let i = 0;
  for (const cid in c) {
    traces.push(
      {
        x: c[cid][0],
        y: c[cid][1],
        z: c[cid][2],
        mode: 'markers',
        type: 'scatter3d',
        marker: {
          color: (cid == -1) ? 'rgb(10,10,10)' : getColor(i++),
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

