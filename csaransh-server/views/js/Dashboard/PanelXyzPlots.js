import React from "react";
import PropTypes from "prop-types";
import ArchiveIcon from '@material-ui/icons/Archive';
import SaveIcon from '@material-ui/icons/Save';
// react plugin for creating charts
// @material-ui/core
import withStyles from "@material-ui/core/styles/withStyles";
import Grid from "@material-ui/core/Grid";
// @material-ui/icons
import ContentCopy from "@material-ui/icons/ContentCopy";
import WbIridescentIcon from "@material-ui/icons/WbIridescent";
import Store from "@material-ui/icons/Store";
import InfoOutline from "@material-ui/icons/InfoOutline";
import Warning from "@material-ui/icons/Warning";
import DateRange from "@material-ui/icons/DateRange";
import LocalOffer from "@material-ui/icons/LocalOffer";
import Update from "@material-ui/icons/Update";
import ArrowUpward from "@material-ui/icons/ArrowUpward";
import AccessTime from "@material-ui/icons/AccessTime";
import Accessibility from "@material-ui/icons/Accessibility";
import BugReport from "@material-ui/icons/BugReport";
import Code from "@material-ui/icons/Code";
import Cloud from "@material-ui/icons/Cloud";
// core components
import GridItem from "components/Grid/GridItem.js";
import Table from "components/Table/Table.js";
import Tasks from "components/Tasks/Tasks.js";
//import CustomTabs from "components/CustomTabs/CustomTabs.js";
import CustomTabs from "./FabCustomTabs.js";
import TableView from "./TableView.js";
import Danger from "components/Typography/Danger.js";
import Card from "components/Card/Card.js";
import CardHeader from "components/Card/CardHeader.js";
import CardIcon from "components/Card/CardIcon.js";
import ListIcon from '@material-ui/icons/List';
import CompareIcon from '@material-ui/icons/InsertChart';
import StatsIcon from '@material-ui/icons/MultilineChart';
import ViewIcon from '@material-ui/icons/BubbleChart';
import CardBody from "components/Card/CardBody.js";
import CardFooter from "components/Card/CardFooter.js";
import Footer from "components/Footer/Footer.js";
import Typography from '@material-ui/core/Typography';
import IconButton from '@material-ui/core/IconButton';
import classNames from 'classnames';
import ChevronLeftIcon from '@material-ui/icons/ChevronLeft';
import ChevronRightIcon from '@material-ui/icons/ChevronRight';
import Divider from '@material-ui/core/Divider';
import MenuIcon from '@material-ui/icons/Menu';
import Drawer from '@material-ui/core/Drawer';
import ClickAwayListener from '@material-ui/core/ClickAwayListener';

import AppBar from "@material-ui/core/AppBar";
import Toolbar from "@material-ui/core/Toolbar";
import Button from '@material-ui/core/Button';
import Tooltip from '@material-ui/core/Tooltip';

import ExpansionPanel from '@material-ui/core/ExpansionPanel';
import ExpansionPanelDetails from '@material-ui/core/ExpansionPanelDetails';
import ExpansionPanelSummary from '@material-ui/core/ExpansionPanelSummary';
import ExpandMoreIcon from '@material-ui/icons/ExpandMore';

//import { bugs, website, server } from "variables/general";

import dashboardStyle from "assets/jss/material-dashboard-react/views/dashboardStyle.js";

// charts import
import { getData, toXyzAr, toXyzArSplit } from "../utils";
import { MainTable } from "../cascades/MainTable";
import { HeatMap, HeatMapC } from "../cascades/heatMap";
import { InputInfo, OutputInfo, DefectsList } from "../cascades/InfoModal";
import { groupBars, NDefectsPlot } from "../statistics/nDefectsPlot";
import { SplomPlot } from "../statistics/splomPlot";
import { ScatterPlot, ClusterPlot } from "../cascades/3d-plots.js";

import {
  DefectDistancePlot,
  addDefectDistance,
  addDefectAngles,
  removeDefectDistance,
  calcStatDistsAngles,
} from "../cascades/DefectDistancePlot";

import {
  CascadeVisualizer3D,
  getCurrentCascade,
  //removeCurrentCascade
} from "../cascades/CascadeVisualizer3D";
import PKADistributionPlots from "../cascades/PKADistributionPlots";

import {
  ClusterSizePlot,
  lookDefectsSizeDistrib
} from "../cascades/ClusterSizePlot";


const maxComparePlot = 3;

class Dashboard extends React.Component {
  constructor(props) {
    super(props);
    this.data = getData();//.slice(0,4);
    let d = groupBars(this.data);
    let elemSet = new Set();
    let energySet = new Set();
    this.maxDefects = [0, "", ""];
    this.maxCluster = [0, "", ""];
    this.planarCent = 26;
    for (const x of this.data) {
      elemSet.add(x.substrate);
      energySet.add(parseFloat(x.energy));
      if (x.n_defects > this.maxDefects[0]) {
        this.maxDefects = [x.n_defects, x.substrate, x.energy];
      }
      if (x.max_cluster_size_I > this.maxCluster[0]) {
        this.maxCluster = [x.max_cluster_size_I, x.substrate, x.energy];
      }
      if (x.max_cluster_size_V > this.maxCluster[0]) {
        this.maxCluster = [x.max_cluster_size_V, x.substrate, x.energy];
      }
    }
    this.allElems = [...elemSet];
    this.allEnergies = [...energySet];
    this.allElems.sort();
    this.allEnergies.sort();
    this.state = {
      loadedCascades: [],
      curRows: this.data,
      DefectSizeDistribConfig: {labels:[], datasets:[]},
      defectDistances: [],
      defectAngles: [],
      focusOnClusterId: undefined,
      selected: new Set(),
      except: new Set(),
      look: "",
      statDists: [],
      statAngles: [],
      mobileOpen: false,
      lookrow: this.data[0],
      singleLocks : [true, false],
      singleRows : [this.data[0], this.data[1]],
      clusterElems : "Click on table action buttons to plot.",
      clusterEnergies : "",
      barData : d[1],
      splomVals : d[0],
      splomKeys: d[2],
      curXyzCoords: toXyzArSplit(this.data[0].coords)
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

  handleToggleDrawer = () => {
    this.setState({ mobileOpen: !this.state.mobileOpen });
  };

  handleHideDrawer = () => {
    if (this.state.mobileOpen === false) return;
    this.setState({ mobileOpen: false });
  };

  handleShowDrawer = () => {
    if (this.state.mobileOpen === true) return;
    this.setState({ mobileOpen: true });
  };

  /* 
  Table handlers
  */
  setRows(curRows) {
    const [statDists, statAngles] = calcStatDistsAngles(curRows);
    let d = groupBars(curRows);
    this.setState({
      curRows,
      statDists,
      statAngles,
      barData:d[1],
      splomVals:d[0],
      splomKeys:d[2],
    });
  }

  exceptCurRowHandler(row) {
    const isExcepted = this.state.except.has(row.name);
    let except = new Set(this.state.except);
    if (isExcepted) {
      except.delete(row.name);
    } else {
      except.add(row.name);
    }
    this.setState({
      except
    });
  }

  lookCurRowHandler(row) {
    const isLooking = (this.state.look === row.name)
    if (isLooking) return;
    let singleRows = this.state.singleRows.slice();
    for (const i in singleRows) {
      if (!this.state.singleLocks[i]) {
        singleRows[i] = row;
      }
    }
    let temp = lookDefectsSizeDistrib(
          this.state.DefectSizeDistribConfig,
          row,
          3
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
    this.setState({
      cascadeData: getCurrentCascade(this.state.loadedCascades, row),
      DefectSizeDistribConfig: temp,
      look: row.name,
      lookrow: row,
      singleRows,  
      clusterElems,
      clusterEnergies,
      curXyzCoords: toXyzArSplit(row.coords)
    });
  }

clearClusterSizeDistrib() {
    this.setState({
      DefectSizeDistribConfig: {labels:[], datasets:[]},
      clusterElems: "Plot Cleared!",
      clusterEnergies: ""
    });
  }



  viewCurRowHandler(row) {
    const isSelected = this.state.selected.has(row.name);
    let selected = new Set(this.state.selected);
    if (isSelected) {
      selected.delete(row.name);
    } else {
      selected.add(row.name);
    }
    if (!isSelected) {
      this.setState({
        DefectSizeDistribConfig: addDefectsSizeDistrib(
          this.state.DefectSizeDistribConfig,
          row
        ),
        defectDistances: addDefectDistance(this.state.defectDistances, row),
        defectAngles: addDefectAngles(this.state.defectAngles, row),
        selected
      });
    } else {
      this.setState({
        DefectSizeDistribConfig: removeDefectsSizeDistrib(
          this.state.DefectSizeDistribConfig,
          row
        ),
        defectDistances: removeDefectDistance(this.state.defectDistances, row),
        defectAngles: removeDefectDistance(this.state.defectAngles, row),
        selected
      });
    }
  }



  render() {
    const { classes } = this.props;
    return (
      <div className="main-panel">
    <AppBar>
            <Toolbar disableGutters={!open}>
             <Typography variant="title" color="inherit" noWrap>
                CSaransh
              </Typography>
  
              <IconButton
                color="inherit"
                aria-label="open drawer"
                //onClick={this.handleDrawerOpen}
                className={classNames(classes.menuButton, open && classes.hide)}
              >
              </IconButton>

           </Toolbar>
    </AppBar>

        <Grid id="mainTableC" /*justify="flex-end"*/ container> 
          <ClickAwayListener onClickAway={this.handleHideDrawer}>
          <GridItem id="mainTable" xs={12}>
<ExpansionPanel id="mainTablePanel" expanded={this.state.mobileOpen} onChange={this.handleToggleDrawer}>
          <ExpansionPanelSummary /*onMouseEnter={this.handleShowDrawer}*/ expandIcon={<ExpandMoreIcon />}>
            <Typography className={classes.heading}>Cascades List - {this.state.curRows.length} cascades {(this.state.curRows.length < this.data.length) ? " filtered out of total " + this.data.length : " - Filter View Plot"} </Typography>
          </ExpansionPanelSummary>
          <ExpansionPanelDetails>
                <MainTable
                  data={this.data}
                  selected={this.state.selected}
                  except={this.state.except}
                  look={this.state.look}
                  setRows={(rows) => this.setRows(rows)}
                  onViewCur={o => this.viewCurRowHandler(o)}
                  onLookCur={o => this.lookCurRowHandler(o)}
                  onExceptCur={o => this.exceptCurRowHandler(o)}
                />
          </ExpansionPanelDetails>
        </ExpansionPanel>
          </GridItem>
        </ClickAwayListener>
          </Grid>
        <Grid container className="content">
          <GridItem xs={12} sm={6} md={3}>
            <Card>
              <CardHeader color="warning" stats icon>
                <CardIcon color="warning">
                  <WbIridescentIcon />
                </CardIcon>
                <p className={classes.cardCategory}>Elements</p>
                <h3 className={classes.cardTitle}>
                  {this.allElems.length} <small>{this.allElems.join(", ")}</small>
                </h3>
              </CardHeader>
            </Card>
          </GridItem>
          <GridItem xs={12} sm={6} md={3}>
            <Card>
              <CardHeader color="success" stats icon>
                <CardIcon color="success">
                  <Store />
                </CardIcon>
                <p className={classes.cardCategory}>Energies</p>
                <h3 className={classes.cardTitle}>
                  {this.allEnergies.length} <small> From {Math.min(...this.allEnergies)}k to {Math.max(...this.allEnergies)}k</small>
                </h3>
              </CardHeader>
            </Card>
          </GridItem>
         <GridItem xs={12} sm={6} md={3}>
            <Card>
              <CardHeader color="info" stats icon>
                <CardIcon color="info">
                  <Accessibility />
                </CardIcon>
                <p className={classes.cardCategory}>Planarity</p>
                <h3 className={classes.cardTitle}>
                  {this.planarCent} 
                  <small> planar cascades</small>
                </h3>
              </CardHeader>
            </Card>
          </GridItem>
          <GridItem xs={12} sm={6} md={3}>
            <Card>
              <CardHeader color="danger" stats icon>
                <CardIcon color="danger">
                  <InfoOutline />
                </CardIcon>
                <p className={classes.cardCategory}>Biggest Cluster</p>
                <h3 className={classes.cardTitle}>
                  {this.maxCluster[0]} <small> in {this.maxCluster[1]} at {this.maxCluster[2]}k</small>
                </h3>
              </CardHeader>
            </Card>
          </GridItem>
 
        </Grid>
        <Grid container>
          <GridItem xs={12} sm={4} md={4}>
            <Card chart>
              <CardHeader color="info">
                <ClusterSizePlot data={this.state.DefectSizeDistribConfig} height={320}
                />
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
          <GridItem xs={12} sm={8} md={8}>
            <Card chart>
              <CardHeader color="warning">
               Statistics over filtered rows - {this.state.curRows.length} 
              </CardHeader>
              <CardBody>
                                <NDefectsPlot
                  data= {this.state.barData}
                />
                <h4 className={classes.cardTitle}>Defects Count Distributions</h4>
              </CardBody>
              <CardFooter chart>
                <div className={classes.stats}>
                  <StatsIcon /> Statistical Plot
                </div>
              </CardFooter>
            </Card>
          </GridItem>
        </Grid>
        <Grid container>
          <GridItem xs={12} sm={12} md={6}>
          <HeatMapC coords={this.state.curXyzCoords[1]}/>
          </GridItem>
          <GridItem xs={12} sm={12} md={6}>
          <ClusterPlot coords={this.state.curXyzCoords}/>
          </GridItem>
        </Grid>
        <Grid container>
          <GridItem xs={12} sm={12} md={6}>
            <HeatMapC coords={this.state.curXyzCoords[2]}/>
          </GridItem>
          <GridItem xs={12} sm={12} md={6}>
                  <CascadeVisualizer3D
                    data={this.state.cascadeData}
                    focusOn={this.state.focusOnClusterId}
                  />
 
          </GridItem>
        </Grid>
 
        <Grid container>
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
        <Grid container>
          <GridItem xs={12}>
         </GridItem>
        </Grid>
 

        <Grid container>
         <GridItem xs={12} sm={12} md={8}>
            <Card>
              <CardHeader color="warning">
                <h4 className={classes.cardTitleWhite}>Splom Plots</h4>
                <p className={classes.cardCategoryWhite}>
                Splom
                </p>
              </CardHeader>
              <CardBody>
                <SplomPlot keys={this.state.splomKeys} vals={this.state.splomVals}/>
              </CardBody>
              <CardFooter chart>
                <div className={classes.stats}>
                  <StatsIcon /> Statistical Plot
                </div>
              </CardFooter>
            </Card>
          </GridItem>
        </Grid>
        <Footer />
      </div>
    );
  }
}

Dashboard.propTypes = {
  classes: PropTypes.object.isRequired
};

export default withStyles(dashboardStyle)(Dashboard);
