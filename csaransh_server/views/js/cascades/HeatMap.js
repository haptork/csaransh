import React from 'react';
import { toXyArSplit, getColor } from "../utils";
import Plot from 'react-plotly.js';

const cookData = (new_coords, type) => {

var trace3 = {
  x: new_coords[0],
  y: new_coords[1],
  type: type,
  colorscale : [['0' , 'rgb(0,225,100)'],['1', 'rgb(100,0,200)']],
};
    return [trace3];
}

export const HeatMap = props => 
  <Plot data={cookData(props.coords, 'histogram2d')} layout={
   {
     margin: { l: 30, r: 10, b: 35, t: 30, pad: 1 },
   }
  }
  style={{height: "320px", width: "100%"}}
  useResizeHandler
  />;

export const HeatMapC = props => {
  return (<Plot data={cookData(props.coords, 'histogram2dcontour')} layout={
   {
     margin: { l: 30, r: 10, b: 35, t: 30, pad: 1 },
   }
  }
  style={{height: "320px", width: "100%"}}
  useResizeHandler
/>);
}
