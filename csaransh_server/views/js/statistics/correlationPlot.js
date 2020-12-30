import React from 'react';
import Plot from 'react-plotly.js';
import Correlation from "node-correlation";

export class CorrelationPlot extends React.Component {
  constructor(props) {
    super(props);
  }

  render() {
    let columns = [];
    let vals = [];
    for (let i = 0; i < this.props.data.length; i++) {
      const it = this.props.data[i];
      columns.push(it.label);
      let cur = [];
      for (let j = 0; j < this.props.data.length; j++) {
        if (j >= i) {
          cur.push(undefined);
          continue;
        }
        const jt = this.props.data[j];
        if (isNaN(it.values[0]) || isNaN(jt.values[0])) {
          cur.push(NaN);
        } else {
          cur.push(Correlation.calc(it.values, jt.values).toFixed(2));
        }
      }
      vals.push(cur);
    }
    const trace1 = {
      x:columns, 
      y:columns, 
      z:vals, 
      colorscale: [['0.0', '#407f93'], ['0.058823529411764705', '#518b9d'], ['0.11764705882352941', '#6497a7'], ['0.17647058823529413', '#76a4b2'], ['0.23529411764705882', '#89b1bd'], ['0.29411764705882354', '#9bbdc8'], ['0.35294117647058826', '#acc9d1'], ['0.4117647058823529', '#bed5dc'], ['0.47058823529411764', '#d1e2e7'], ['0.5294117647058824', '#e4eef2'], ['0.5882352941176471', '#f2f2f2'], ['0.6470588235294118', '#f9e0e2'], ['0.7058823529411765', '#f5ced1'], ['0.7647058823529411', '#f2bbbf'], ['0.8235294117647058', '#eea8ad'], ['0.8823529411764706', '#eb969c'], ['0.9411764705882353', '#e7858c'], ['1.0', '#e57880']], 
      type: 'heatmap'
    };

  
    const annotations = [];
    for ( var i = 0; i < columns.length; i++ ) {
      for ( var j = 0; j < columns.length; j++ ) {
        if (j >= i) continue;
        const currentValue = vals[i][j];
        const textColor = 'black';
        const result = {
          xref: 'x1',
          yref: 'y1',
          x: columns[j],
          y: columns[i],
          text: currentValue,
          showarrow: false,
          font: {
            color: textColor
          }
        };
        annotations.push(result);
      }
    }
    var layout = {
      annotations: annotations,
      xaxis: {
        ticks: '',
        side: 'top',
        showgrid: false
      },
      yaxis: {
        ticks: '',
        showgrid: false
        //ticksuffix: ' ',
        //width: 700,
        //height: 700,
        //autosize: false
      }
    };
    return (
      <Plot
        data= {[trace1]}
        layout = {layout}
        style={{height: "460px", width: "100%"}}
        useResizeHandler
      />
    );
  }
}
/*

Plotly.Plot.resize()
*/