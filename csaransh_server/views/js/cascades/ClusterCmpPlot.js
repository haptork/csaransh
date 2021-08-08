import React from 'react';

import Grid from "@material-ui/core/Grid";
import GridItem from "components/Grid/GridItem.js";

import Card from "components/Card/Card.js";
import CardHeader from "components/Card/CardHeader.js";
import CardBody from "components/Card/CardBody.js";
import CardFooter from "components/Card/CardFooter.js";
//import { ScatterCmpPlot } from "../cascades/3d-plots.js";
import { Cluster2CmpPlot } from "../cascades/3d-plots.js";
import ViewIcon from '@material-ui/icons/BubbleChart';

import InputLabel from '@material-ui/core/InputLabel';
import MenuItem from '@material-ui/core/MenuItem';
import FormControl from '@material-ui/core/FormControl';
import FormGroup from '@material-ui/core/FormGroup';
import Select from '@material-ui/core/Select';
import FormControlLabel from '@material-ui/core/FormControlLabel';
import Checkbox from '@material-ui/core/Checkbox';

import Stepper from '@material-ui/core/Stepper';
import Step from '@material-ui/core/Step';
import StepButton from '@material-ui/core/StepButton';
import Typography from '@material-ui/core/Typography';
import Paper from '@material-ui/core/Paper';

const getClass = (row, cid) => {
    return (row.hasOwnProperty("clusterClasses") && row.clusterClasses.savi.hasOwnProperty(cid) && row.clusterClasses.savi[cid] !== -1 && row.clusterClasses.savi[cid] !== "noise") ? "; class-" + row.clusterClasses.savi[cid] : "";
}

const getClusterVar = (row, cid) => {
  if (cid) {
    return row.eigen_features[cid].var[0] + ", " + row.eigen_features[cid].var[1];
  }
  return "";
};

const getClusterTypeAndClass = (row, cid) => {
  if (cid) {
    const typeInfo = (row.clusterSizes[cid] > 0) ? "majority interstitials" : "majority vacancies";
    const classInfo = getClass(row, cid);
    //const componentClassInfo = (row.hasOwnProperty("clusterClasses") && row.clusterClasses.hasOwnProperty("comp") && row.clusterClasses.comp.hasOwnProperty(cid) && row.clusterClasses.comp[cid] !== -1 && row.clusterClasses.comp[cid] !== "noise") ? "; component class-" + row.clusterClasses.comp[cid] : "";
    return [typeInfo, classInfo];
  }
  return [-1, -1];
};

export const getCids = (row) => {
  const cids = [];
  const c = Object.keys(row.features);
  for (const x of c) {
    cids.push({label:x, value:x});
  }
  return cids;
  //const curSelection = (cids.length > 0) ? cids[0] : "";
};

export const getInitialSelection = (row) => {
  const cids = getCids(row);
  if (cids.length > 0) return cids[0].value;
  return "";
};

const getCmpCoord = (row, cid, data, mode, isSize, val) => {
  if (cid == '') return [row, cid];
  if (!(row.clust_cmp_size.hasOwnProperty(cid))) {
    const cids = getCids(row);
    if (cids.length > 0) cid = cids[0].value;
    else return [row, cid];
  }
  let x = row.clust_cmp[cid][mode];
  //let count = row.clust_cmp_size[cid][mode]
  if (isSize) x = row.clust_cmp_size[cid][mode];
  if (val >= x.length || x[val].length < 3) return [row, ''];
  const fid = parseInt(x[val][1]);
  return [data[fid], x[val][2]];
};

const getCmpCids = (row, cid, data, mode, isSize, shortName) => {
  if (cid == '') return [];
  if (!(row.clust_cmp_size.hasOwnProperty(cid))) {
    const cids = getCids(row);
    if (cids.length > 0) cid = cids[0].value;
    else return [];
  }
  let scores = row.clust_cmp[cid][mode];
  if (isSize) {
    scores = row.clust_cmp_size[cid][mode];
  }
  scores = scores.filter(x => x.length >= 3 && x[1] < data.length);
  return scores.map(x => {
    const name = "cid " + x[2] + ' of ' + shortName(data[x[1]]);
    const iorv = (data[x[1]].clusterSizes[x[2]] > 0) ? "; inter." : "; vac.";
    const clabel = getClass(data[x[1]], x[2])
    // TODO: add component class
    const info = "diff: " + (x[0]).toFixed(2) + " eigen-var: " + 
           data[x[1]].eigen_features[x[2]]["var"][0] + ", " + data[x[1]].eigen_features[x[2]]["var"][1] +
           iorv + clabel;
    return {"name": name, "info": info};
  });
};

export class ClusterCmpPlot extends React.Component {
  constructor(props) {
    super(props);
    this.allModes = [{label:"Angles", value:"angle"}, 
                     {label:"Distances", value:"dist"},
                     {label:"All", value:"all"}
                    ];
    const curMode = "angle";
    const isSize = true;
    const curShow = 0;
    this.state = {
      curMode : curMode,
      isSize : isSize,
      curShow : curShow,
    };
  }

  shouldComponentUpdate(nextProps, nextState) {
    return this.props.row.id != nextProps.row.id
           || this.props.cid != nextProps.cid
           || this.state.curMode != nextState.curMode
           || this.state.curShow != nextState.curShow
           || this.state.isSize != nextState.isSize
           ;
  }

  handleMode(curMode) {
    this.setState({
      curMode
    });
  }

  handleIsSize(isSize) {
    //const isSize = !this.state.isSize;
    this.setState({
      isSize
    });
  }

  handleShow(val) {
    this.setState({
      curShow : val,
    });
  }
/*
  render() {
    return (
      <div>whatever</div>
    )
  }
  */
 render() {
    const {classes, row, cid, data, allCids} = this.props;
    let cid2 = "" + cid;
    if (cid2.length == 0) cid2 = getInitialSelection(row);
    const cmpCids = getCmpCids(row, cid2, data, this.state.curMode, this.state.isSize, this.props.shortName);
    const cmpCoords = getCmpCoord(row, cid2, data, this.state.curMode, this.state.isSize, this.state.curShow);
    const mainVariance = getClusterVar(row, cid2);
    const typeAndClass = getClusterTypeAndClass(row, cid2);
    return (
    <Card chart>
      <CardHeader color="primary">
      Cluster Comparison
      </CardHeader>
      <CardBody>
        <Grid container>
        <GridItem xs={12} sm={12} md={6}>
        <Paper>
        <Cluster2CmpPlot row={row} cid={cid2}/>
        <Typography  variant="caption" style={{textAlign:"center"}}>eigen dim. var:{mainVariance}; {typeAndClass[0]}{typeAndClass[1]}</Typography>
        <Grid container justify="center">
        <GridItem xs={12} sm={12} md={12} >
        <FormGroup column>
         <FormControl>
          <InputLabel htmlFor="cid-select">Cluster Id</InputLabel>
          <Select
            value={cid}
            onChange={(event) => { this.props.handleClusterCmp(event.target.value); }}
            inputProps={{
              name: 'cluster-selection',
              id: 'cid-select',
            }}
          >
          {allCids.map((o, i) => <MenuItem key={i} value={o.value}>{o.label}</MenuItem>)}
          </Select>
          </FormControl>
         <FormControl>
          <InputLabel htmlFor="cluster-mode">Similarity By</InputLabel>
          <Select
            value={this.state.curMode}
            onChange={(event) => { this.handleMode(event.target.value); }}
            inputProps={{
              name: 'cluster-mode',
              id: 'cluster-mode',
            }}
          >
          {this.allModes.map((o, i) => <MenuItem key={i} value={o.value}>{o.label}</MenuItem>)}
          </Select>
          </FormControl>
        <FormControlLabel
          control={
            <Checkbox
              checked={this.state.isSize}
              onChange={(event) => { this.handleIsSize(event.target.checked); }}
              value="isSize"
              color="primary"
            />
          }
          label="Match only clusters with similar number of defects"
        />
        </FormGroup>
        </GridItem>
        </Grid>
        </Paper>
        </GridItem>
       <GridItem xs={12} sm={12} md={6}>
        <Cluster2CmpPlot row={cmpCoords[0]} cid={cmpCoords[1]}/>
        <Stepper alternativeLabel nonLinear activeStep={this.state.curShow}>
          {cmpCids.map((label, index) => {
            const buttonProps = {};
            buttonProps.optional = <Typography variant="caption">{label.info}</Typography>;
            return (
              <Step key={index} completed={false}>
                <StepButton
                  onClick={() => this.handleShow(index)}
                  completed={false}
                  {...buttonProps}
                >
                  {label.name}
                </StepButton>
              </Step>
            );
          })}
        </Stepper>
        </GridItem>
        </Grid>
      </CardBody>
      <CardFooter chart>
        <div className={classes.stats}>
          <ViewIcon/> For the selected cluster of the current cascade, shows the top similar clusters from the whole database. Plots are in eigen basis, eigen var hints at dimensionality.
        </div>
      </CardFooter>
   </Card>
   );
 }

}
/*
    const {classes, row, cid, data, allCids} = this.props;
    const cmpCids = getCmpCids(row, cid, data, this.state.curMode, this.state.isSize, this.props.shortName);
    const cmpCoords = getCmpCoord(row, cid, data, this.state.curMode, this.state.isSize, this.state.curShow);
    const mainVariance = getClusterVar(row, cid);
    const typeAndClass = getClusterTypeAndClass(row, cid);
    return (
    <Card chart>
      <CardHeader color="primary">
      Cluster Comparison
      </CardHeader>
      <CardBody>
        <Grid container>
        <GridItem xs={12} sm={12} md={6}>
        <Paper>
        <Cluster2CmpPlot row={row} cid={cid}/>
        <Typography  variant="caption" style={{textAlign:"center"}}>eigen dim. var:{mainVariance}; {typeAndClass[0]}{typeAndClass[1]}</Typography>
        <Grid container justify="center">
        <GridItem xs={12} sm={12} md={12} >
        <FormGroup column>
         <FormControl>
          <InputLabel htmlFor="cid-select">Cluster Id</InputLabel>
          <Select
            value={cid}
            onChange={(event) => { this.props.handleClusterCmp(event.target.value); }}
            inputProps={{
              name: 'cluster-selection',
              id: 'cid-select',
            }}
          >
          {allCids.map((o, i) => <MenuItem key={i} value={o.value}>{o.label}</MenuItem>)}
          </Select>
          </FormControl>
         <FormControl>
          <InputLabel htmlFor="cluster-mode">Similarity By</InputLabel>
          <Select
            value={this.state.curMode}
            onChange={(event) => { this.handleMode(event.target.value); }}
            inputProps={{
              name: 'cluster-mode',
              id: 'cluster-mode',
            }}
          >
          {this.allModes.map((o, i) => <MenuItem key={i} value={o.value}>{o.label}</MenuItem>)}
          </Select>
          </FormControl>
        <FormControlLabel
          control={
            <Checkbox
              checked={this.state.isSize}
              onChange={(event) => { this.handleIsSize(event.target.checked); }}
              value="isSize"
              color="primary"
            />
          }
          label="Match only clusters with similar number of defects"
        />
        </FormGroup>
        </GridItem>
        </Grid>
        </Paper>
        </GridItem>
       <GridItem xs={12} sm={12} md={6}>
        <ScatterCmpPlot coords={cmpCoords} colorIndex={parseInt(cid)}/>
        <Stepper alternativeLabel nonLinear activeStep={this.state.curShow}>
          {cmpCids.map((label, index) => {
            const buttonProps = {};
            buttonProps.optional = <Typography variant="caption">{label.info}</Typography>;
            return (
              <Step key={index} completed={false}>
                <StepButton
                  onClick={() => this.handleShow(index)}
                  completed={false}
                  {...buttonProps}
                >
                  {label.name}
                </StepButton>
              </Step>
            );
          })}
        </Stepper>
        </GridItem>
        </Grid>
      </CardBody>
      <CardFooter chart>
        <div className={classes.stats}>
          <ViewIcon/> For the selected cluster of the current cascade, shows the top similar clusters from the whole database. Plots are in eigen basis, eigen var hints at dimensionality.
        </div>
      </CardFooter>
   </Card>
    );

    */