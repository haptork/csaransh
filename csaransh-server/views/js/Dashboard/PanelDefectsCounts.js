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

class PanelDefectsCount extends React.Component {
  constructor(props) {
    super(props);
    this.data = props.data();//.slice(0,4);
    this.state = {
      curRows: this.props.curRows,
      DefectSizeDistribConfig: {labels:[], datasets:[]},
      look: "",
      lookrow: this.props.lookRow,
      clusterElems : "Click on table action buttons to plot.",
      clusterEnergies : "",
    };
  }

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
/*
  render() {
    return (
    );
  }
  */
}

Dashboard.propTypes = {
  classes: PropTypes.object.isRequired
};

export default withStyles(dashboardStyle)(Dashboard);
