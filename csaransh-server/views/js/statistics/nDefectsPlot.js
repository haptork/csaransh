import React from 'react';
import Plot from 'react-plotly.js';
import { getColor } from "../utils";

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