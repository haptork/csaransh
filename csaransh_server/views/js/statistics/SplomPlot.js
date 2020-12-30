import React from 'react';
import Plot from 'react-plotly.js';

    const pl_colorscale=[
               [0.0, '#19d3f3'],
               [0.333, '#19d3f3'],
               [0.333, '#e763fa'],
               [0.666, '#e763fa'],
               [0.666, '#636efa'],
               [1, '#636efa']
    ];

const cookData = (keys, values, scaled_planarity) => {
    return [{
      type: 'splom',
      dimensions: values,
      text: keys,
      marker: {
        color: values[0].values, // TODO
        colorscale:pl_colorscale,
        size: 7,
        line: {
          color: 'white',
          width: 0.5
        }
      }
    }];
}

const axis = {
      showline:false,
      zeroline:false,
      gridcolor:'#ffff',
      ticklen:4
    };

const layout = {
      height: 1600,
      width: 1600,
      autosize: false,
      hovermode:'closest',
      dragmode:'select',
      plot_bgcolor:'rgba(240,240,240, 0.95)',
      xaxis:axis,
      yaxis:axis,
      xaxis2:axis,
      xaxis3:axis,
      xaxis4:axis,
      xaxis5:axis,
      yaxis2:axis,
      yaxis3:axis,
      yaxis4:axis,
      yaxis5:axis
    }

export const SplomPlot = props => 
  <Plot data={cookData(props.keys, props.vals)} layout={layout}/>;