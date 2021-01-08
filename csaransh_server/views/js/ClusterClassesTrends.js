import React from 'react';

import Grid from "@material-ui/core/Grid";
import GridItem from "components/Grid/GridItem.js";

import Card from "components/Card/Card.js";
import CardHeader from "components/Card/CardHeader.js";
import CardBody from "components/Card/CardBody.js";
import CardFooter from "components/Card/CardFooter.js";
import ViewIcon from '@material-ui/icons/BubbleChart';

import CompareIcon from '@material-ui/icons/InsertChart';
import CustomTabs from "components/CustomTabs/CustomTabs.js";
import AngleIcon from "@material-ui/icons/CallSplit";
import DistIcon from "@material-ui/icons/LinearScale";
import StatsIcon from '@material-ui/icons/MultilineChart';

import Plot from 'react-plotly.js';
import Paper from '@material-ui/core/Paper';
import Select from 'react-select';

const getTags = () => {
  const clusterClasses = window.cluster_classes;
  let picked = "";
  for (const key in clusterClasses) {
    if (picked.lenth == 0) picked = key;
    if (key.startsWith('line')) {
      picked = key;
    }
  }
  if ((picked.length) == 0) return {};
  return clusterClasses[picked].tags;
};

const groupByKey = (row, groupingLabels) => {
  let res = '';
  for (const label of groupingLabels) {
    res += row[label.value] + '_';
  }
  res = res.slice(0, -1); 
  return res;
};

const groupByName = (groupingLabels) => {
  let res = '';
  for (const label of groupingLabels) {
    res += label.value + '_';
  }
  res = res.slice(0, -1); 
  return res;
};

const ClusterClassesPieChart = props => {
  let classesCount = {};
  let familyCount = {};
  let total = 0;
  for (const x in props.tags) {
    classesCount[x] = props.tags[x].length;
    const family = parseInt(x[0]);
    if (!familyCount.hasOwnProperty(family)) familyCount[family] = 0;
    familyCount[family] += classesCount[x];
    total += classesCount[x];
  }
  let classLabels = Object.keys(classesCount);
  classLabels.sort();
  let familyLabels = Object.keys(familyCount);
  familyLabels.sort();
  let labels = [];
  let parents = [];
  let values = [];
  labels.push("All Clusters");
  parents.push("");
  values.push(total);
  for (const x of familyLabels) {
    labels.push(x);
    parents.push("All Clusters");
    values.push(familyCount[x]);
  }
  for (const x of classLabels) {
    if (x == '8') {
      labels.push('8a');
    } else {
      labels.push(x);
    }
    const family = parseInt(x[0]);
    parents.push(family);
    values.push(classesCount[x]);
  }
  const data = [
  {
    "type": "sunburst",
    "labels": labels,
    "parents": parents,
    "values":  values,
    "leaf": {"opacity": 0.75},
    "marker": {"line": {"width": 2}},
    "branchvalues": 'total'
  }];
  const layout = {
     margin: { l: 0, r: 0, b: 5, t: 5, pad: 1 },
  };
  return (
    <Plot data={data} layout={layout}  
      style={{height: "320px", width: "100%"}}
      useResizeHandler/>
  );
};

const ClusterClassesEnergyBar1 = props => {
  let labels = Object.keys(props.tags);
  const groupingLabels = props.groupingLabels;
  labels.sort();
  let totalCascades = {};
  for (const row of props.data) {
    const groupingLabel = groupByKey(row, groupingLabels);
    if (!totalCascades.hasOwnProperty(groupingLabel)) totalCascades[groupingLabel] = 0;
    totalCascades[groupingLabel]++;
  }
  const allEnergies = Object.keys(totalCascades);
  let vals = [];
  for (const x in allEnergies) {
    let cur = [];
    for (const y in labels) {
      cur.push(0);
    }
    vals.push(cur);
  }
  for (const classLabel in props.tags) {
    const y = labels.indexOf(classLabel);
    if (y == -1) continue;
    for (const cluster of props.tags[classLabel]) {
      if (!(props.ids.has(props.allData[cluster[0]].id))) continue;
      const groupingLabel = groupByKey(props.data[props.indexMap.get(cluster[0])], groupingLabels);
      const x = allEnergies.indexOf(groupingLabel);
      vals[x][y]++;
    }
  }
  for (const ienergy in vals) {
    const energy = allEnergies[ienergy];
    for (const ilabel in vals[ienergy]) {
      vals[ienergy][ilabel] /= (totalCascades[energy]*1.0);
    }
  }
  let traces = [];
  for (const i in vals) {
    traces.push({
      x: labels,
      y: vals[i],
      type: 'bar',
      name: allEnergies[i]
    });
  }
  const layout = {
     margin: { l: 40, r: 20, b: 40, t: 20, pad: 1 },
     barmode: 'stack',
     xaxis: {
       title: {
         text: "class labels"
       }
     },
     yaxis: {
       title: {
         text: "avg. fraction of clusters per cascade"
       }
     }
  };
  return (
    <Plot data={traces} layout={layout} 
      style={{height: "320px", width: "100%"}}
    useResizeHandler/>
  );
}

const ClusterClassesEnergyBar2 = props => {
  let labels = Object.keys(props.tags);
  const groupingLabels = props.groupingLabels;
  labels.sort();
  let totalCascades = {};
  for (const row of props.data) {
    const groupingLabel = groupByKey(row, groupingLabels);
    if (!totalCascades.hasOwnProperty(groupingLabel)) totalCascades[groupingLabel] = 0;
    totalCascades[groupingLabel]++;
  }
  const allEnergies = Object.keys(totalCascades);
  //allEnergies.sort(function(a, b) { return a - b; });
  let vals = [];
  for (const x in labels) {
    let cur = [];
    for (const y in allEnergies) {
      cur.push(0);
    }
    vals.push(cur);
  }
  for (const classLabel in props.tags) {
    let y = labels.indexOf(classLabel);
    for (const cluster of props.tags[classLabel]) {
      // console.log(cluster[0]); 
      // console.log(props.data.length); 
      // console.log(props.indexMap.get(cluster[0])); 
      const groupingLabel = groupByKey(props.data[cluster[0]], groupingLabels);
      const x = allEnergies.indexOf(groupingLabel);
      vals[y][x]++;
    }
  }
  for (const ilabel in vals) {
    for (const ienergy in vals[ilabel]) {
      const energy = allEnergies[ienergy];
      vals[ilabel][ienergy] /= (totalCascades[energy]*1.0);
    }
  }
  let traces = [];
  for (const i in vals) {
    traces.push({
      x: allEnergies,
      y: vals[i],
      type: 'bar',
      name: labels[i]
    });
  }
  const layout = {
     margin: { l: 40, r: 20, b: 40, t: 20, pad: 1 },
     barmode: 'stack',
     xaxis: {
       title: {
         text: groupByName(groupingLabels)
       }
     },
     yaxis: {
       title: {
         text: "avg. fraction of clusters per cascade"
       }
     }
  };
  return (
    <Plot data={traces} layout={layout} 
      style={{height: "540px", width: "100%"}}
    useResizeHandler/>
  );
}

const ClusterClassesEnergyBar3 = props => {
  let labels = Object.keys(props.tags);
  const groupingLabels = props.groupingLabels;
  labels.sort();
  let totalCascades = {};
  for (const classLabel in props.tags) {
    for (const y of props.tags[classLabel]) {
      const row = props.data[y[0]];
      const groupingLabel = groupByKey(row, groupingLabels);
      if (!totalCascades.hasOwnProperty(groupingLabel)) totalCascades[groupingLabel] = 0;
      totalCascades[groupingLabel]++;
    }
  }
  const allEnergies = Object.keys(totalCascades);
  //allEnergies.sort(function(a, b) { return a - b; });
  let vals = [];
  for (const x in labels) {
    let cur = [];
    for (const y in allEnergies) {
      cur.push(0);
    }
    vals.push(cur);
  }
  for (const classLabel in props.tags) {
    const family = parseInt(classLabel[0]);
    let y = labels.indexOf(classLabel);
    for (const cluster of props.tags[classLabel]) {
    const groupingLabel = groupByKey(props.data[cluster[0]], groupingLabels);
      const x = allEnergies.indexOf(groupingLabel);
      vals[y][x]++;
    }
  }
  for (const ilabel in vals) {
    for (const ienergy in vals[ilabel]) {
      const energy = allEnergies[ienergy];
      vals[ilabel][ienergy] /= (totalCascades[energy]*1.0);
    }
  }
  let traces = [];
  for (const i in vals) {
    traces.push({
      x: allEnergies,
      y: vals[i],
      type: 'bar',
      name: labels[i]
    });
  }
  const layout = {
     margin: { l: 40, r: 20, b: 40, t: 20, pad: 1 },
     barmode: 'stack',
     xaxis: {
       title: {
         text: groupByName(groupingLabels)
       }
     },
     yaxis: {
       title: {
         text: "avg. fraction of clusters normed by total clusters in each energy."
       }
     }
  };
  return (
    <Plot data={traces} layout={layout} 
      style={{height: "540px", width: "100%"}}
    useResizeHandler/>
  );
}

const ClusterClassesEnergyLine = props => {
  let labels = Object.keys(props.tags);
  const groupingLabels = props.groupingLabels;
  labels.sort();
  if (labels[0] == '-1') {
      labels.splice(0, 1)
  }
  // console.log('label')
  // console.log(labels);
  // console.log(groupingLabels);
  let totalCascades = {};
  for (const row of props.data) {
    const groupingLabel = groupByKey(row, groupingLabels);
    if (!totalCascades.hasOwnProperty(groupingLabel)) totalCascades[groupingLabel] = 0;
    totalCascades[groupingLabel]++;
  }
  let families = [];
  let lastLabel = -2;
  for (const x of labels) {
    const curLabel = parseInt(x[0]);
    if (curLabel != lastLabel) {
      families.push(curLabel);
      lastLabel = curLabel;
    }
  }
  for (const x of families) {
    labels.push(x);
  }
  const allEnergies = Object.keys(totalCascades);
  //allEnergies.sort(function(a, b) { return a - b; });
  let vals = [];
  for (const x in labels) {
    let cur = [];
    for (const y in allEnergies) {
      cur.push(0);
    }
    vals.push(cur);
  }
  for (const classLabel in props.tags) {
    const y = labels.indexOf(classLabel);
    if (y == -1) continue;
    const curFamily = parseInt(classLabel[0]);
    const familyIndex = labels.indexOf(curFamily);
    /*
    console.log(classLabel);
    console.log(y);
    console.log(curFamily);
    console.log(labels);
    console.log(familyIndex);
    */
    for (const cluster of props.tags[classLabel]) {
      if (!(props.ids.has(props.allData[cluster[0]].id))) continue;
      const groupingLabel = groupByKey(props.data[props.indexMap.get(cluster[0])], groupingLabels);
      const x = allEnergies.indexOf(groupingLabel);
      vals[y][x]++;
      vals[familyIndex][x]++;
    }
  }
  for (const ilabel in vals) {
    for (const ienergy in vals[ilabel]) {
      const energy = allEnergies[ienergy];
      vals[ilabel][ienergy] /= (totalCascades[energy]*1.0);
    }
  }
  const menuItems = [];
  for (const familyLabel of families) {
    let visibility = [];
    for (const label of labels) {
      const curFamily = parseInt(label[0]);
      if (familyLabel == 1 && label == 1) {
        visibility.push(false);
        continue;
      }
      if (familyLabel == 8 && label === 8) {
        visibility.push(false);
        continue;
      }
      visibility.push(curFamily == familyLabel || label == familyLabel);
    } 
    menuItems.push({'method':'restyle', args:['visible', visibility], label:familyLabel});
  }

  const updatemenus = [{
    y: 1,
    yanchor: 'top',
    buttons: menuItems
  }];

  let traces = [];
  for (const i in vals) {
    traces.push({
      x: allEnergies,
      y: vals[i],
      type: 'scatter',
      visible: (labels[i] == 1) ? true : false,
      name: labels[i]
    });
  }
  const layout = {
     margin: { l: 40, r: 20, b: 40, t: 20, pad: 1 },
     updatemenus: updatemenus,
     xaxis: {
       title: {
         text: groupByName(groupingLabels)
       }
     },
     yaxis: {
       title: {
         text: "avg. fraction of clusters per cascade"
       }
     }
  };

  return (
    <Plot data={traces} layout={layout} config={{displayModeBar: false}}
      style={{height: "320px", width: "100%"}}
    useResizeHandler/>
  );
}


export class ClusterClassesTrends extends React.Component {
  constructor(props) {
    super(props);
    this.tags = getTags();
    this.options = [
      { value: 'substrate', label: 'Material' },
      { value: 'energy', label: 'Energy' },
      { value: 'temperature', label: 'Temperature' },
      { value: 'potentialUsed', label: 'Potential' },
      { value: 'es', label: 'Electronic stopping' },
      { value: 'author', label: 'Author' },
      { value: 'tags', label: 'Tags' }
    ];
    this.defaultGroupingLabels = this.options.slice(0, 1);
    this.state = {
      groupingLabels: this.defaultGroupingLabels
    };
    this.allIds = new Map(this.props.allData.map((element, index) => {
      return [element.id, index];
    }));
    if (this.allIds.length != this.props.allData.length) {
      console.log("Error because of non-unique ids.");
    }
    /*
    this.props.allData.forEach(element, index => {
      this.allIds.set(element.id, index);
    });
    */
  }

  handleChange = groupingLabels => {
   this.setState(
      { groupingLabels }
    );
  };

  shouldComponentUpdate(nextProps, nextState){
    return this.props.data != nextProps.data || this.state.groupingLabels != nextState.groupingLabels;
  }

  render() {
    const { groupingLabels } = this.state;
    const { classes } = this.props;
    const tags = this.tags;
    const data = this.props.data;
    const ids = new Set(this.props.data.map(val => val.id));
    const indexMap = new Map(data.map((element, index) => {
      return [this.allIds.get(element.id), index];
    }));
    //console.log("allIds"); 
    //console.log(this.allIds); 
    //console.log("ids"); 
    //console.log(indexMap); 
    return (
      <Grid container justify="center">
       <GridItem xs={12} sm={12} md={12}>
          <Card chart>
            <CardHeader color="primary"> <span>Classification grouped by </span>
              <span style={{display:"inline-block", top:"8px", position:"relative", marginLeft:"10px"}}>
              <Select
                value={groupingLabels}
                closeOnSelect={false}
                multi
                options={this.options}
                onChange={this.handleChange}
              />
              </span>
            </CardHeader>
            <CardBody>

         <Grid container justify="center">
         <GridItem xs={12} sm={12} md={8}>
              <ClusterClassesEnergyBar1 tags={tags} data={data} ids={ids} indexMap={indexMap} allData={this.props.allData} groupingLabels={groupingLabels}/>
         </GridItem>
         <GridItem xs={12} sm={12} md={4}>
            <Paper>
            <ClusterClassesEnergyLine tags={tags} data={data} ids={ids} indexMap={indexMap} allData={this.props.allData} groupingLabels={groupingLabels}/>
            </Paper>
          </GridItem>
          </Grid>
            </CardBody>
            <CardFooter chart>
              <div className={classes.stats}>
                <StatsIcon /> Distribution of classes among groups - Select grouping from the top selection.
              </div>
            </CardFooter>
          </Card>
        </GridItem>
      </Grid>
    );
    /*
    return (
        <Grid container>
         <GridItem xs={12} sm={12} md={2}>
          <Paper>
            <ClusterClassesPieChart tags={tags}/>
            <div className={this.props.classes.stats}>
              Shows proportions of cluster families and classes across all the 140 cascades. Click on a family to zoom in it's corresponding classes, click again to go back. As expected the families with parallel oriented dumbbells are dominant such as 5th, 4th, 2nd. Crowdions are also substantial. Some of the classes (like 2a, 3b, 8) are very less or missing completely.
            </div>
          </Paper>
         </GridItem>
         <GridItem xs={12} sm={6} md={8}>
          <Paper>
            <img style={{width:"100%"}} src="images/drawing.png" />
            <div className={this.props.classes.stats}>
              A schematic diagram of all the classes used for supervised learning. The typical cluster shape for each class is also shown. The classes are placed in a way similar to where dimensionality reduction above places them.
            </div>
          </Paper>
          </GridItem>
         <GridItem xs={12} sm={3} md={2}>
            <Paper>
            <ClusterClassesEnergyBar2 tags={tags} data={data} groupingLabels={groupingLabels}/>
            <div className={this.props.classes.stats}>
              Shows average number of clusters per cascade in each class for different energies.
            </div>
            </Paper>
          </GridItem>
         <GridItem xs={12} sm={12} md={6}>
            <Paper>
            <ClusterClassesEnergyBar1 tags={tags} data={data} groupingLabels={groupingLabels}/>
            <div className={this.props.classes.stats}>
              Shows average number of clusters for each class label at different energies. Mouse over to look for exact fractions for each energy.
            </div>
            </Paper>
          </GridItem>
          <GridItem xs={12} sm={12} md={4}>
          <Paper>
            <ClusterClassesEnergyLine tags={tags} data={data} groupingLabels={groupingLabels}/>
            <div className={this.props.classes.stats}>
              Shows mean per cascade count for various classes and families against PKA energies. Select a family from the drop down list. Some of the trends are: Number of bigger 5f clusters increase after 150keV, 5e after 100keV while medium sized 5c starts decreasing after 100keV.
            </div>
          </Paper>
          </GridItem>
         <GridItem xs={12} sm={3} md={2}>
            <Paper style={{marginTop:"10px"}}>
            <ClusterClassesEnergyBar3 tags={tags} data={data} groupingLabels={groupingLabels}/>
            <div className={this.props.classes.stats}>
              Shows average number of clusters in each class normalized by the total number of clusters in each energy.
            </div>
            </Paper>
          </GridItem>
        </Grid>
    );
    */
  }
}
/*

*/