import React from "react";
import PropTypes from "prop-types";
import ArchiveIcon from '@material-ui/icons/Archive';
import SaveIcon from '@material-ui/icons/Save';
// react plugin for creating charts
// @material-ui/core
import withStyles from "@material-ui/core/styles/withStyles";
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
import Grid from "@material-ui/core/Grid";
import GridItem from "components/Grid/GridItem.js";
import Table from "components/Table/Table.js";
import Tasks from "components/Tasks/Tasks.js";
import CustomTabs from "components/CustomTabs/CustomTabs.js";
//import CustomTabs from "./WatCustomTab.js";
import TableView from "./TableView.js";
import Danger from "components/Typography/Danger.js";
import Card from "components/Card/Card.js";
import CardHeader from "components/Card/CardHeader.js";
import CardIcon from "components/Card/CardIcon.js";
import CardBody from "components/Card/CardBody.js";
import CardFooter from "components/Card/CardFooter.js";
import ListIcon from '@material-ui/icons/List';
import CompareIcon from '@material-ui/icons/InsertChart';
import StatsIcon from '@material-ui/icons/MultilineChart';
import ViewIcon from '@material-ui/icons/BubbleChart';
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
import ExpansionPanelActions from '@material-ui/core/ExpansionPanelActions';

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
import {ClusterCmpPlot, getCids, getInitialSelection, getClusterCoord, getClusterVar} from "../cascades/ClusterCmpPlot";

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

const styles = theme => ({
  /*
  root: {
    width: '100%',
  },
  */
  heading: {
    fontSize: theme.typography.pxToRem(15),
  },
  secondaryHeading: {
    fontSize: theme.typography.pxToRem(15),
    color: theme.palette.text.secondary,
  },
  icon: {
    verticalAlign: 'bottom',
    height: 20,
    width: 20,
  },
  details: {
    alignItems: 'center',
  },
  column: {
    flexBasis: '33.33%',
  },
  helper: {
    borderLeft: `2px solid ${theme.palette.divider}`,
    padding: `${theme.spacing.unit}px ${theme.spacing.unit * 2}px`,
  },
  /*
  link: {
    color: theme.palette.primary.main,
    textDecoration: 'none',
    '&:hover': {
      textDecoration: 'underline',
    },
  },
  */
});

const maxComparePlot = 4;

class DashboardSimple extends React.Component {
  constructor(props) {
    super(props);
    this.data = getData();//.slice(0,4);
    let d = groupBars(this.data);
    let elemSet = new Set();
    let energySet = new Set();
    this.maxDefects = [0, "", ""];
    this.maxCluster = [0, "", "", 0];
    let pc = 0.0;
    for (const x of this.data) {
      if ((x.eigen_var[0] + x.eigen_var[1]) > 0.9) pc++;
    }
    this.planarCent = parseInt(pc * 100 / this.data.length);  // for 90% threshold in PCA
    for (const x of this.data) {
      elemSet.add(x.substrate);
      energySet.add(parseFloat(x.energy));
      if (x.n_defects > this.maxDefects[0]) {
        this.maxDefects = [x.n_defects, x.substrate, x.energy];
      }
      if (x.density_cluster_vac.length - 1 >= this.maxCluster[0]) {
        let impact = x.density_cluster_vac.length <= 1 ? 0 : parseInt(x.clusters[x.density_cluster_vac[1]].length * 100 / x.clusters[x.density_cluster_vac[0]].length);
        if (this.maxCluster[1].length > 0 && impact < this.maxCluster[3] && impact < 50) {
          continue;
        }
        this.maxCluster = [x.density_cluster_vac.length - 1, x.substrate, x.energy, impact];
      }
    }
    this.allElems = [...elemSet];
    this.allEnergies = [...energySet];
    this.allElems.sort();
    this.allEnergies.sort();
    const [statDists, statAngles] = calcStatDistsAngles(this.data);
    const initialPick = Math.floor(Math.random() * this.data.length);
    this.state = {
      curRows: this.data,
      DefectSizeDistribConfig: {labels:[], datasets:[]},
      defectDistances: [],
      defectAngles: [],
      focusOnClusterId: undefined,
      selected: new Set(),
      except: new Set(),
      look: "",
      statDists: statDists,
      statAngles: statAngles,
      mobileOpen: false,
      lookrow: this.data[initialPick],
      singleLocks : [false, true],
      singleRows : [this.data[0], this.data[initialPick]],
      clusterElems : "Click on table action buttons to plot.",
      clusterEnergies : "",
      barData : d[1],
      splomVals : d[0],
      splomKeys: d[2],
      cascadeData: [this.data[initialPick]],
      curXyzCoords: toXyzArSplit(this.data[initialPick]),
      cmpCoords: getClusterCoord(this.data[initialPick], ""),
      cmpVariance: getClusterVar(this.data[initialPick], ""),
      curSelection: getInitialSelection(this.data[initialPick])
    };
  }

  handleCurSelection(cid) {
    cid = '' + cid;
    if (!this.state.lookrow.features.hasOwnProperty(cid)) return;
    this.setState ({
      curSelection: cid,
      cmpCoords: getClusterCoord(this.state.lookrow, cid),
      cmpVariance: getClusterVar(this.state.lookrow, cid)
    });
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
    this.setState({
      cascadeData: getCurrentCascade(this.state.cascadeData, row),
      DefectSizeDistribConfig: temp,
      look: row.name,
      lookrow: row,
      singleRows,  
      clusterElems,
      clusterEnergies,
      defectDistances: addDefectDistance(this.state.defectDistances, row, maxComparePlot),
      defectAngles: addDefectAngles(this.state.defectAngles, row, maxComparePlot),
      curXyzCoords: toXyzArSplit(row),
      cmpCoords: getClusterCoord(row, ""),
      cmpVariance: getClusterVar(row, ""),
      curSelection: getInitialSelection(row)
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
            <Typography className={classes.heading}>Cascades List - {this.state.curRows.length} cascades {(this.state.curRows.length < this.data.length) ? " filtered out of total " + this.data.length : " - Filter View Plot Using Action Buttons"} </Typography>
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
                <p className={classes.cardCategory}>Planarity of Cascades</p>
                <h3 className={classes.cardTitle}>
                  {this.planarCent} 
                  <small> % planar</small>
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
                <p className={classes.cardCategory}>Subcascades</p>
                <h3 className={classes.cardTitle}>
                  {this.maxCluster[0]} <small> in {this.maxCluster[1]} at {this.maxCluster[2]}k</small>
                </h3>
              </CardHeader>
            </Card>
          </GridItem>
        </Grid>
        <ExpansionPanel>
        <ExpansionPanelSummary expandIcon={<ExpandMoreIcon />}>
          <div className={classes.column}>
            <Typography className={classes.heading}>Visualize and Find Patterns</Typography>
          </div>
          <div className={classes.column}>
            <Typography className={classes.secondaryHeading}>Currently Viewing - {this.state.lookrow.name}</Typography>
          </div>
        </ExpansionPanelSummary>
        <ExpansionPanelDetails className={classes.details}>
        <Grid container>
          <GridItem xs={12} sm={12} md={6}>
       <CustomTabs
              title={"3D"}
              headerColor="primary"
              tabs={[
                {
                  tabName: "Clusters",
                  tabIcon: Code,
                  tabContent: (
                  <CascadeVisualizer3D
                    data={this.state.cascadeData}
                    focusOn={this.state.focusOnClusterId}
                    clickHandler={cid=>this.handleCurSelection(cid)}
                  />
                  ),
                  footerContent: (
                    <div className={classes.stats}>
                      <ViewIcon/> Click on a cluster to find similar clusters in the section below
                    </div>
                  )
                },
                {
                  tabName: "Scatter",
                  tabIcon: BugReport,
                  tabContent: (
                  <ScatterPlot
                    coords={this.state.curXyzCoords}
                  />
                  ),
                  footerContent: (
                    <div className={classes.stats}>
                      <ViewIcon/> Uses Eigen basis, Clustered vacs can give measure of subcascades,
                       as given in the subcascade density.
                    </div>
                  )
                },
                {
                  tabName: "Mesh-Subcascades",
                  tabIcon: ListIcon,
                  tabContent: (
                 <ClusterPlot row={this.state.lookrow}/>
                 ),
                  footerContent: (
                    <div className={classes.stats}>
                      <ViewIcon/> Uses Eigen basis, Meshes of vacancies in different subcascades.
                    </div>
                  )
                }
              ]}
            />
          </GridItem>
          <GridItem xs={12} sm={12} md={6}>
       <CustomTabs
              title={"2D Eigen Contours"}
              headerColor="primary"
              tabs={[
                {
                  tabName: "Vac",
                  tabIcon: Code,
                  tabContent: (
                 <HeatMapC coords={this.state.curXyzCoords[1]}/>
                  ),
                  footerContent: (
                    <div className={classes.stats}>
                      <ViewIcon/> Shows density variations, helpful in estimating major subcascades
                    </div>
                  )

                },
                {
                  tabName: "Int",
                  tabIcon: BugReport,
                  tabContent: (
                 <HeatMapC coords={this.state.curXyzCoords[0]}/>
                  ),
                  footerContent: (
                    <div className={classes.stats}>
                      <ViewIcon/> Shows density variations, helpful in estimating major clusters of interstitials.
                    </div>
                  )
                },
                {
                  tabName: "All",
                  tabIcon: ListIcon,
                  tabContent: (
                 <HeatMapC coords={this.state.curXyzCoords[2]}/>
                 ),
                  footerContent: (
                    <div className={classes.stats}>
                      <ViewIcon/> Shows density variations, helpful in estimating size along major principle axes.
                    </div>
                  )
                }
              ]}
            />
          </GridItem>
           <GridItem xs={12} sm={12} md={12}>
            <ClusterCmpPlot classes={classes} row={this.state.lookrow} data={this.data} cids={getCids(this.state.lookrow)} curSelection={this.state.curSelection} curCoords={this.state.cmpCoords} curVar={this.state.cmpVariance} setCurSelection={(cid=>this.handleCurSelection(cid))}/>
           </GridItem>
       </Grid>
        </ExpansionPanelDetails>
        </ExpansionPanel>
        <ExpansionPanel>
        <ExpansionPanelSummary expandIcon={<ExpandMoreIcon />}>
          <div className={classes.column}>
            <Typography className={classes.heading}>View and Compare Cascade Info</Typography>
          </div>
          <div className={classes.column}>
            <Typography className={classes.secondaryHeading}>Plots, Distributions, Info to compare, copy, download</Typography>
          </div>
        </ExpansionPanelSummary>
        <ExpansionPanelDetails className={classes.details}>
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
        </ExpansionPanelDetails>
        </ExpansionPanel>
        <ExpansionPanel>
        <ExpansionPanelSummary expandIcon={<ExpandMoreIcon />}>
          <div className={classes.column}>
            <Typography className={classes.heading}>Statistics over filtered rows</Typography>
          </div>
          <div className={classes.column}>
            <Typography className={classes.secondaryHeading}>{this.state.curRows.length} rows currently filtered</Typography>
          </div>
        </ExpansionPanelSummary>
        <ExpansionPanelDetails className={classes.details}>
        <Grid container justify="center">
          <GridItem xs={12} sm={12} md={12}>
            <Card chart>
              <CardHeader color="info">
               Statistics grouped by Elements and Energy
              </CardHeader>
              <CardBody>
                                <NDefectsPlot
                  data= {this.state.barData}
                />
              </CardBody>
              <CardFooter chart>
                <div className={classes.stats}>
                  <StatsIcon /> Statistical Plot - filter data and remove outliers from table action buttons.
                </div>
              </CardFooter>
            </Card>
          </GridItem>
         <GridItem xs={12} sm={12} md={12}>
            <Card>
              <CardHeader color="primary">
                <h4 className={classes.cardTitleWhite}>Splom Plots - Correlations</h4>
              </CardHeader>
              <CardBody id="splomCardBody">
                <SplomPlot keys={this.state.splomKeys} vals={this.state.splomVals}/>
              </CardBody>
              <CardFooter chart>
                <div className={classes.stats}>
                  <StatsIcon /> Statistical Plot - Correlations, colors represent number of defects, filter data and remove outliers from table action buttons for better idea on correlations.
                </div>
              </CardFooter>
            </Card>
          </GridItem>
 
         <GridItem xs={12} sm={12} md={12}>
       <CustomTabs
              title={"Distribution from PKA"}
              headerColor="warning"
              tabs={[
                {
                  tabName: "Angles",
                  tabIcon: Code,
                  tabContent: (
                <DefectDistancePlot
                  data={this.state.statAngles}
                  title={""}
                  xlabel={"Angstroms"}
                  type={"Statistics"}
                  height={"480px"}
                />
                  ),
                  footerContent: (
                    <div className={classes.stats}>
                      <StatsIcon/> Distribtion of defects distances from PKA position
                    </div>
                  )

                },
                {
                  tabName: "Distances",
                  tabIcon: BugReport,
                  tabContent: (
                <DefectDistancePlot
                  data={this.state.statDists}
                  title={""}
                  xlabel={"Angstroms"}
                  type={"Statistics"}
                  height={"480px"}
                />
                  ),
                  footerContent: (
                    <div className={classes.stats}>
                      <StatsIcon/> Angular Distribtion of defects from PKA direction
                    </div>
                  )

                }
              ]}
            />
          </GridItem>
          </Grid>
          </ExpansionPanelDetails>
          </ExpansionPanel>
        <Footer />
      </div>
    );
  }
}

DashboardSimple.propTypes = {
  classes: PropTypes.object.isRequired
};

//export default withStyles(dashboardStyle)(Dashboard);
export default withStyles(styles)(withStyles(dashboardStyle)(DashboardSimple));
//export default withStyles(dashboardStyle)(DashboardSimple);
// <h4 className={classes.cardTitle}>Defects Count Distributions</h4>
// <p className={classes.cardCategoryWhite}> Splom </p>