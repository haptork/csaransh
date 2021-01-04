import React from 'react';

import Grid from "@material-ui/core/Grid";
import GridItem from "components/Grid/GridItem.js";

import Card from "components/Card/Card.js";
import CardHeader from "components/Card/CardHeader.js";
import CardBody from "components/Card/CardBody.js";
import CardFooter from "components/Card/CardFooter.js";
import ViewIcon from '@material-ui/icons/BubbleChart';

import CompareIcon from '@material-ui/icons/InsertChart';

import TableView from "./TableView.js";

import { uniqueKey } from "../utils";

import {
  ClusterSizePlot,
  lookDefectsSizeDistrib
} from "./ClusterSizePlot";

class ComparisonTable extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      lock : this.props.defaultLock,
    };
    this.row = this.props.row;
    //console.log(this.props.row.name);
    //this.row = this.props.row;
  }

  shouldComponentUpdate(nextProps, nextState) {
    return this.state.lock != nextState.lock || (!this.state.lock && uniqueKey(this.row) != uniqueKey(nextProps.row));
  }

  toggleLock() {
    const lock = !this.state.lock; 
    this.setState ({
      lock
    });
  }

  setRow(row) {
    if (!this.state.lock) this.row = row;
  }

  render() {
    const { row } = this.props;
    this.setRow(row);
    return (
          <GridItem xs={12} sm={6} md={6}>
            <TableView 
              row={this.row} 
              lock={this.state.lock} 
              handleLock={()=>this.toggleLock()} 
              headerColor="primary"
              />
          </GridItem>
    );
  }
}

export class ClusterSizeCmpItem extends React.Component {
  constructor(props) {
    super(props);
  }

  shouldComponentUpdate(nextProps, nextState){
    return !this.props.compareRowsHas;
  }

  render() {
    const {classes, data, elems} = this.props;
    return (
      <GridItem xs={12} sm={6} md={6}>
        <Card chart>
          <CardHeader color="info">
            <ClusterSizePlot data={data} height={320} />
          </CardHeader>
          <CardBody>
            <h4 className={classes.cardTitle}>Cluster Size Distribution</h4>
            <p className={classes.cardCategory}>
              <span className={classes.successText}>
              {elems} 
              </span>
            </p>
          </CardBody>
          <CardFooter chart>
            <div className={classes.stats}>
              <CompareIcon/> Comparison Plot
            </div>
          </CardFooter>
        </Card>
      </GridItem>
    );
  }
}
 
export class Comparison extends React.Component {
  constructor(props) {
    super(props);
    this.compareRows = new Set();
    this.limit = 4;
    this.defectSizeDistribConfig = {labels:[], datasets:[]};
    this.defectDistances = [];
    this.defectAngles = [];
    this.clusterEnergies = "";
    this.clusterElems = "";
  }

  shouldComponentUpdate(nextProps, nextState){
    return this.props.data.id != nextProps.data.id;
    //return true;
    //return this.props.data != nextProps.data;
  }

  render() {
    const { classes } = this.props;
    const row = this.props.data;
    const compareRowsHas = this.compareRows.has(uniqueKey(row));
    if (!compareRowsHas) {
      this.compareRows.add(uniqueKey(row));
      if (this.compareRows.size > this.limit) {
        this.compareRows.delete(this.compareRows.values().next().value);
      }
      this.defectSizeDistribConfig = lookDefectsSizeDistrib(this.defectSizeDistribConfig, row, this.limit);
      let elemsSet = new Set();
      //console.log(this.defectSizeDistribConfig.labels);
      for (const i in this.defectSizeDistribConfig.labels) {
        //console.log(this.defectSizeDistribConfig.labels[i]);
        const x = this.defectSizeDistribConfig.labels[i][1];
        elemsSet.add(x);
      }
      elemsSet = [...elemsSet];
      this.clusterElems = elemsSet.join(", ");
    }
    //if (this.compareRows.length > this.limit) this.compareRows.delete(this.compareRows.values[0]);
    return (
        <Grid container>
          <ClusterSizeCmpItem elems={this.clusterElems} data={this.defectSizeDistribConfig} compareRowsHas={compareRowsHas} classes={classes} />
          <ComparisonTable row={row} defaultLock={false} /> 
        </Grid>
    );
  }
}

/*
export class Comparison extends React.Component {
    this.state = {
      DefectSizeDistribConfig: {labels:[], datasets:[]},
      defectDistances: [],
      defectAngles: [],
      singleLocks : [false, true],
      singleRows : [this.data[0], this.data[initialPick]],
      clusterElems : "Click on table action buttons to plot.",
      clusterEnergies : "",
    };
  }

  toggleLocks(i) {
    let singleLocks = this.state.singleLocks.slice();
    singleLocks[i] = !singleLocks[i];
    if (singleLocks[i] == false && this.state.singleRows[i].name !== this.state.lookrow.name) {
      let singleRows = this.state.singleRows.slice();
      singleRows[i] = this.state.lookrow;
      this.setState({
        singleLocks,
        singleRows
      });
    } else {
      this.setState ({
        singleLocks
      });
    }
  }

  clearClusterSizeDistrib() {
    this.setState({
      DefectSizeDistribConfig: {labels:[], datasets:[]},
      clusterElems: "Plot Cleared!",
      clusterEnergies: ""
    });
  }

  shouldComponentUpdate(nextProps, nextState){
    return true;
    //return this.props.data != nextProps.data;
  }

  someFn() {
    let singleRows = this.state.singleRows.slice();
    for (const i in singleRows) {
      if (!this.state.singleLocks[i]) {
        singleRows[i] = row;
      }
    }
    let temp = lookDefectsSizeDistrib(
          this.state.DefectSizeDistribConfig,
          row,
          maxComparePlot
        );
    let elemsSet = new Set();
    let energySet = new Set();
    for (const i in temp.labels) {
      const x = temp.labels[i][1].split("_");
      elemsSet.add(x[0]);
      energySet.add(x[1]);
    }
    elemsSet = [...elemsSet];
    energySet = [...energySet];
    let clusterElems = elemsSet.join(", ");
    let clusterEnergies = energySet.join(", ");
    if (energySet.length == 1) clusterEnergies = " at energy " + clusterEnergies;
    else clusterEnergies = " at energies " + clusterEnergies;
 
  }

  render() {
    const { classes } = this.props;
    const data = groupBars(this.props.data); 

      defectDistances: addDefectDistance(this.state.defectDistances, row, maxComparePlot),
      defectAngles: addDefectAngles(this.state.defectAngles, row, maxComparePlot),
    return (
        <Grid container>
          <GridItem xs={12} sm={6} md={5}>
            <Card chart>
              <CardHeader color="info">
                <ClusterSizePlot data={this.state.DefectSizeDistribConfig} height={320} />
              </CardHeader>
              <CardBody>
                <h4 className={classes.cardTitle}>Cluster Size Distribution</h4>
                <p className={classes.cardCategory}>
                  <span className={classes.successText}>
                  {this.state.clusterElems} 
                  </span>{" "}
                  {this.state.clusterEnergies} 
                </p>
                  <Button className="fabButtonsInfo" onClick={()=>this.clearClusterSizeDistrib()}>
                    Clear Plot
                    </Button>
              </CardBody>
              <CardFooter chart>
                <div className={classes.stats}>
                  <CompareIcon/> Comparison Plot
                </div>
              </CardFooter>
            </Card>
          </GridItem>
           <GridItem xs={12} sm={6} md={7}>
       <CustomTabs
              title={"Distribution around PKA"}
              headerColor="danger"
              tabs={[
                {
                  tabName: "Angles",
                  tabIcon: Code,
                  tabContent: (

<DefectDistancePlot
                  data={this.state.defectAngles}
                  title={""}
                  xlabel={"Degree"}
                  height={"325px"}
                />
                  ),
                  footerContent: (
                    <div className={classes.stats}>
                      <ViewIcon/> Angular distribution of defects around PKA direction
                    </div>
                  )

                },
                {
                  tabName: "Distances",
                  tabIcon: BugReport,
                  tabContent: (

<DefectDistancePlot
                  data={this.state.defectDistances}
                  title={""}
                  xlabel={"Angstroms"}
                  height={"325px"}
                />
                  ),
                  footerContent: (
                    <div className={classes.stats}>
                      <ViewIcon/> Distribtion of defects distances from PKA position
                    </div>
                  )

                }
              ]}
            />
          </GridItem>
          <GridItem xs={12} sm={12} md={6}>
            <TableView 
              row={this.state.singleRows[0]} 
              lock={this.state.singleLocks[0]} 
              id={0} 
              handleLock={(i)=>this.toggleLocks(i)} 
              headerColor="primary"
              />
          </GridItem>
          <GridItem xs={12} sm={12} md={6}>
            <TableView row={this.state.singleRows[1]} 
            lock={this.state.singleLocks[1]} 
            id={1} 
            handleLock={(i)=>this.toggleLocks(i)} 
            headerColor="warning"
            />
          </GridItem>
          </Grid>
    );
  }
}
*/
/*

Plotly.Plot.resize()
*/