import React from "react";
import withStyles from "@material-ui/core/styles/withStyles";
import PropTypes from "prop-types";
// core components
import Grid from "@material-ui/core/Grid";
import GridItem from "components/Grid/GridItem.js";
import Footer from "components/Footer/Footer.js";
import Typography from '@material-ui/core/Typography';
import IconButton from '@material-ui/core/IconButton';
import classNames from 'classnames';
import ClickAwayListener from '@material-ui/core/ClickAwayListener';
import AppBar from "@material-ui/core/AppBar";
import Toolbar from "@material-ui/core/Toolbar";
import ExpansionPanel from '@material-ui/core/ExpansionPanel';
import ExpansionPanelDetails from '@material-ui/core/ExpansionPanelDetails';
import ExpansionPanelSummary from '@material-ui/core/ExpansionPanelSummary';
import ExpandMoreIcon from '@material-ui/icons/ExpandMore';
import dashboardStyle from "assets/jss/material-dashboard-react/views/dashboardStyle.js";
// charts import
import { getData, uniqueKey, getAllCol} from "../utils";
import { MainTable } from "../cascades/MainTable";
import {getCids, getInitialSelection} from "../cascades/ClusterCmpPlot";
import {ClusterClassesPlot} from "../ClusterClasses.js";
import {OutlineCards} from "../cascades/OutlineCards";
import { getCurrentCascade } from "../cascades/CascadeVisualizer3D";
import { Statistics } from "../statistics/Statistics";
import { Comparison } from "../Comparison/Comparison";
import {CascadesAndClusterCmp} from "../cascades/CascadesAndClusterCmp";
//other components
import Select from 'react-select';
import Paper from '@material-ui/core/Paper';

import {ClusterClassesTrends} from "../ClusterClassesTrends.js";

const styles = theme => ({
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
});

const overallStats = data => {
  let elemSet = new Set();
  let energySet = new Set();
  let pc = 0.0;
  const planarCutoff = 0.95;
  for (const x of data) {
    if ((x.eigen_var[0] + x.eigen_var[1]) > planarCutoff) pc++;
  }
  const planarCent = parseInt(pc * 100 / data.length);  // for 90% threshold in PCA
  for (const x of data) {
    elemSet.add(x.substrate);
    energySet.add(parseFloat(x.energy));
  }
  let allElems = [...elemSet];
  let allEnergies = energySet;
  allElems.sort();
  const classes = window.cluster_classes; // TODO: get data from same fn.
  const classKeys = Object.keys(classes);
  let totalClasses = Object.keys(classes[classKeys[0]].tags).length;
  if (totalClasses > 0) totalClasses -= 1;
  return [
    {"title": "Elements", "label": allElems.length, "labelSm": allElems.join(", ")},
    {"title": "Energies", "label": allEnergies.length, "labelSm": "From " + Math.min(...allEnergies) + "k to " + Math.max(...allEnergies) + "k"},
    {"title": "Planarity of Cascades", "label": planarCent, "labelSm": " % planar"},
    {"title": "Classes of Clusters", "label": totalClasses, "labelSm": " different shapes"},
  ];
};


class DashboardSimple extends React.Component {
  constructor(props) {
    super(props);
    this.data = getData();//.slice(0,4);
    this.dataOutline = overallStats(this.data);
    this.allCols = getAllCol();
    const initialPick = 0;//Math.floor(Math.random() * this.data.length);
    this.state = {
      curRows: this.data,
      compareRows: new Set(),
      except: new Set(),
      look: "",
      mobileOpen: false,
      lookrow: this.data[initialPick],
      cidCmp: getInitialSelection(this.data[initialPick]),
      allCids: getCids(this.data[initialPick]),
      showCol: this.allCols.filter(x => x['isShow'])//.map(x => {return {'value':x['value'], 'label':x['label']}; })
    };
  }

  shortName = (row) => {
    let name = row.id;
    for (const x of this.state.showCol) {
      if (x.type != 'input') continue;
      name += "-" + x.parseFn(x.accessor(row));
    }
    return name;
  }

  handleShowCols = showCol => {
   this.setState(
      { showCol }
    );
  };

  handleClusterCmp(cid) {
    cid = '' + cid;
    if (!this.state.lookrow.features.hasOwnProperty(cid)) return;
    this.setState ({
      cidCmp : cid
    });
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

  setRows(curRows) {
    this.setState({
      curRows,
    });
  }

  exceptCurRowHandler(row) {
    const isExcepted = this.state.except.has(uniqueKey(row));
    let except = new Set(this.state.except);
    if (isExcepted) {
      except.delete(uniqueKey(row));
    } else {
      except.add(uniqueKey(row));
    }
    this.setState({
      except
    });
  }

  lookCurRowHandler(row) {
    const isLooking = (this.state.look === row.name)
    if (isLooking) return;
    const compareRows = new Set(this.state.compareRows);
    compareRows.add(uniqueKey(row));
   this.setState({
      cascadeData: getCurrentCascade(this.state.cascadeData, row),
      look: uniqueKey(row),
      lookrow: row,
      compareRows,
      cidCmp: getInitialSelection(row),
      allCids: getCids(row)
    });
  }

  render() {
    const { classes } = this.props;
    return (
      <div className="main-panel">
    <AppBar>
            <Toolbar disableGutters={!open}>
             <Typography variant="title" color="inherit" noWrap>
                <a id="logoTitle" href="https://github.com/haptork/csaransh">CSaransh</a>
              </Typography>
  
              <IconButton
                color="inherit"
                aria-label="open drawer"
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
            <Typography className={classes.heading}>Cascades List - {this.state.curRows.length} cascades {(this.state.curRows.length < this.data.length) ? " filtered out of total " + this.data.length : " - Filter, View, Plot Using Action Buttons"} 
           </Typography>
          </ExpansionPanelSummary>
          <ExpansionPanelDetails>
            <div style={{display:"block", width:"100%"}}>
              <Select
                value={this.state.showCol}
                closeOnSelect={false}
                multi
                options={this.allCols}
                onChange={this.handleShowCols}
              />
                <MainTable
                  data={this.data}
                  except={this.state.except}
                  look={this.state.look}
                  setRows={(rows) => this.setRows(rows)}
                  onLookCur={o => this.lookCurRowHandler(o)}
                  onExceptCur={o => this.exceptCurRowHandler(o)}
                  showCol={this.state.showCol}
                />
            </div>
          </ExpansionPanelDetails>
       </ExpansionPanel>
          </GridItem>
        </ClickAwayListener>
          </Grid>
          <OutlineCards values= {this.dataOutline} classes={classes}/>

       <CascadesAndClusterCmp classes={classes} row={this.state.lookrow} cid={this.state.cidCmp} allCids={this.state.allCids} handleClusterCmp={(cid)=>this.handleClusterCmp(cid)} data={this.data} shortName={this.shortName}/>
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
            <Statistics classes={classes} data={this.state.curRows}/>
        </ExpansionPanelDetails>
        </ExpansionPanel>

        <ExpansionPanel>
        <ExpansionPanelSummary expandIcon={<ExpandMoreIcon />}>
          <div className={classes.column}>
            <Typography className={classes.heading}>View Cluster Classes</Typography>
          </div>
          <div className={classes.column}>
            <Typography className={classes.secondaryHeading}>Based on the Shapes of the Clusters</Typography>
          </div>
        </ExpansionPanelSummary>
        <ExpansionPanelDetails className={classes.details}>
        <Grid container justify="center">
          <GridItem xs={12} sm={12} md={12}>
            <ClusterClassesPlot classes={classes} data={this.data} shortName={this.shortName}/>
          </GridItem>
         <GridItem xs={12} sm={12} md={10}>
          <Paper>
            <img style={{width:"100%"}} src="images/drawing.png" />
            <div className={this.props.classes.stats}>
              A schematic diagram of all the classes used for supervised learning. The typical cluster shape for each class is also shown. The classes are placed in a way similar to where dimensionality reduction above places them relative to each other.
            </div>
          </Paper>
          </GridItem>
          <GridItem xs={12} sm={12} md={12}>
            <ClusterClassesTrends classes={classes} allData={this.data} data={this.state.curRows}/>
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
            <Typography className={classes.secondaryHeading}>Plots, Distributions, Info to view, copy, download</Typography>
          </div>
        </ExpansionPanelSummary>
        <ExpansionPanelDetails className={classes.details}>
            <Comparison classes={classes} data = {this.state.lookrow}/>
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

export default withStyles(styles)(withStyles(dashboardStyle)(DashboardSimple));
/*
      <ExpansionPanel>
        <ExpansionPanelSummary expandIcon={<ExpandMoreIcon />}>
          <div className={classes.column}>
            <Typography className={classes.heading}>Cluster Trends</Typography>
          </div>
          <div className={classes.column}>
            <Typography className={classes.secondaryHeading}>Plots, Distributions, Info to compare, copy, download</Typography>
          </div>
        </ExpansionPanelSummary>
        <ExpansionPanelDetails className={classes.details}>
        <Grid container>
          <GridItem xs={12} sm={12} md={12}>
            <ClusterClassesTrends classes={classes} tags={window.cluster_classes['supervised (kNN)']['tags']} data={this.data}/>
          </GridItem>
          </Grid>
        </ExpansionPanelDetails>
        </ExpansionPanel>
*/