import React from 'react';

import Grid from "@material-ui/core/Grid";
import GridItem from "components/Grid/GridItem.js";
import Card from "components/Card/Card.js";
import CardHeader from "components/Card/CardHeader.js";
import CardBody from "components/Card/CardBody.js";
import CardFooter from "components/Card/CardFooter.js";
import { ScatterCmpPlot, ClassesPlot } from "./cascades/3d-plots.js";
import ClassesIcon from '@material-ui/icons/Category';
import InputLabel from '@material-ui/core/InputLabel';
import MenuItem from '@material-ui/core/MenuItem';
import FormControl from '@material-ui/core/FormControl';
import FormGroup from '@material-ui/core/FormGroup';
import Select from '@material-ui/core/Select';
import Typography from '@material-ui/core/Typography';
import Paper from '@material-ui/core/Paper';

const getData = (curMode) => {
  return window.cluster_classes[curMode];
};

const getName = (curMode, classIndex, index, data, shortName) => {
  const d = getData(curMode);
  if (!d.tags.hasOwnProperty(classIndex) || index >= d.tags[classIndex].length) {
    return "";
  }
  let row = data[d.tags[classIndex][index][0]];
  let cid = d.tags[classIndex][index][1];
  return "cluster-id " + cid + ' of ' + shortName(row);
  //return row.substrate + " @ " + row.energy + "keV; cluster-id: " + cid + "; file: " +  row.infile;
};

const getClusterCoords = (curMode, classIndex, index, data) => {
  const d = getData(curMode);
  let c = [[],[],[]];
  if (!d.tags.hasOwnProperty(classIndex) || index >= d.tags[classIndex].length) {
    return c;  
  }
  let row = data[d.tags[classIndex][index][0]];
  let cid = d.tags[classIndex][index][1];
  for (const x of row.eigen_features[cid].coords) {
    c[0].push(x[0]);
    c[1].push(x[1]);
    c[2].push(x[2]);
  }
  return c;
};

export class ClusterClassesPlot extends React.Component {
  constructor(props) {
    super(props);
    this.allModes = [];
    for (const k in window.cluster_classes) {
      this.allModes.push({label:k, value:k});
    }
    const curMode = this.allModes[0].value;
    const featureCoords = getData(curMode).show_point;
    this.state = {
      curMode : curMode,
      featureCoords : featureCoords,
      clusterCoords: getClusterCoords(curMode, 0, 0, this.props.data),
      nm: getName(curMode, 0, 0, this.props.data, this.props.shortName),
      curIndex: 0
    };
  }

  handleShow(classIndex, index) {
    this.setState({
      clusterCoords : getClusterCoords(this.state.curMode, classIndex, index, this.props.data),
      nm: getName(this.state.curMode, classIndex, index, this.props.data, this.props.shortName),
      curIndex : index
    });
  }

  handleMode(curMode) {
    this.setState({
      curMode,
      featureCoords : getData(curMode).show_point,
      clusterCoords: getClusterCoords(curMode, 0, 0, this.props.data),
      nm: getName(curMode, 0, 0, this.props.data, this.props.shortName),
      curIndex: 0
    })
  }

  onPointClick(data) {
    this.handleShow(data.points[0].fullData.name, data.points[0].pointNumber);
  }

  shouldComponentUpdate(nextProps, nextState){
    return this.state.curIndex != nextState.curIndex || this.state.curMode != nextState.curMode;
  }

  render() {
    return (
    <Card chart>
      <CardHeader color="info">
      Cluster Classes
      </CardHeader>
      <CardBody>
        <Grid container>
          <GridItem xs={12} sm={12} md={6}>
          <Paper>
          <ClassesPlot mode={this.state.curMode} coords={this.state.featureCoords} colorIndex={1} clickHandler={(dt)=>this.onPointClick(dt)} />
        <Grid container justify="center">
        <GridItem xs={12} sm={12} md={12} >
        <FormGroup column>
         <FormControl>
          <InputLabel htmlFor="cid-select">Classification type</InputLabel>
          <Select
            value={this.state.curMode}
            onChange={(event) => { this.handleMode(event.target.value); }}
            inputProps={{
              name: 'class-mode',
              id: 'class-mode',
            }}
          >
          {this.allModes.map((o, i) => <MenuItem key={i} value={o.value}>{o.label}</MenuItem>)}
          </Select>
          </FormControl>
        </FormGroup>
        </GridItem>
        </Grid>

 
          </Paper>
          </GridItem>
          <GridItem xs={12} sm={12} md={6}>
          <ScatterCmpPlot coords={this.state.clusterCoords} colorIndex={this.state.curIndex}/>
          <Typography  variant="caption" style={{textAlign:"center"}}>{this.state.nm}</Typography>
          </GridItem>
        </Grid>
      </CardBody>
      <CardFooter chart>
        <div className={this.props.classes.stats}>
          <ClassesIcon/> Shows cluster classification. Different classes are shown with different colors. Click on a point on left to view the cluster on right. Click on legend to toggle a class and double click on legend label to only make that class points appear. The classes are found using supervised learning on the classes described <a href="https://www.sciencedirect.com/science/article/pii/S0927025619306639">: here</a>.
        </div>
      </CardFooter>
   </Card>
    );
  }
}