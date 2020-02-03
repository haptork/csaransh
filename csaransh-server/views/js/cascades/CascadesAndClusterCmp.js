import React from "react";

import ClusterIcon from "@material-ui/icons/BubbleChart";
import ScatterIcon from "@material-ui/icons/ScatterPlot";
import MeshIcon from "@material-ui/icons/HdrStrong";

import IntIcon from "@material-ui/icons/BugReportRounded";
import VacIcon from "@material-ui/icons/CropFree";
import AllIcon from "@material-ui/icons/CenterFocusStrong";
// core components
import Grid from "@material-ui/core/Grid";
import GridItem from "components/Grid/GridItem.js";
import CustomTabs from "components/CustomTabs/CustomTabs.js";
//import CustomTabs from "./WatCustomTab.js";
import ListIcon from '@material-ui/icons/List';
import ViewIcon from '@material-ui/icons/BubbleChart';
import Typography from '@material-ui/core/Typography';

import ExpansionPanel from '@material-ui/core/ExpansionPanel';
import ExpansionPanelDetails from '@material-ui/core/ExpansionPanelDetails';
import ExpansionPanelSummary from '@material-ui/core/ExpansionPanelSummary';
import ExpandMoreIcon from '@material-ui/icons/ExpandMore';

// charts import
import { HeatMapC } from "../cascades/HeatMap";
import { ScatterPlot, ClusterPlot } from "../cascades/3d-plots.js";
import {ClusterCmpPlot} from "../cascades/ClusterCmpPlot";

import { toXyzArSplit, uniqueKey } from "../utils";

import {
  CascadeVisualizer3D,
  //removeCurrentCascade
} from "../cascades/CascadeVisualizer3D";
 
class CascadeViews1 extends React.Component {
  constructor(props) {
    super(props);
  }

  shouldComponentUpdate(nextProps, nextState) {
    return this.props.row.id != nextProps.row.id;
  }

  render() {
    const { classes } = this.props;
    const row = this.props.row;
    const curXyzCoords = toXyzArSplit(row);
    return (
          <GridItem xs={12} sm={12} md={6}>
       <CustomTabs
              title={"3D"}
              headerColor="info"
              tabs={[
                {
                  tabName: "Clusters",
                  tabIcon: ClusterIcon,
                  tabContent: (
                  <CascadeVisualizer3D
                    data={[row]}
                    clickHandler={this.props.handleClusterCmp}
                  />
                  ),
                  footerContent: (
                    <div className={classes.stats}>
                      <ClusterIcon/> Click on a cluster to find similar clusters in the section below
                    </div>
                  )
                },
                {
                  tabName: "Scatter",
                  tabIcon: ScatterIcon,
                  tabContent: (
                  <ScatterPlot
                    coords={curXyzCoords}
                  />
                  ),
                  footerContent: (
                    <div className={classes.stats}>
                      <ScatterIcon/> Uses Eigen basis, Clustered vacs can give measure of subcascades,
                       as given in the subcascade density.
                    </div>
                  )
                },
                {
                  tabName: "Mesh-Subcascades",
                  tabIcon: MeshIcon,
                  tabContent: (
                 <ClusterPlot row={row}/>
                 ),
                  footerContent: (
                    <div className={classes.stats}>
                      <MeshIcon/> Uses Eigen basis, Meshes of vacancies in different subcascades.
                    </div>
                  )
                }
              ]}
            />
          </GridItem>
   );
  }
}

class CascadeViews2 extends React.Component {
  constructor(props) {
    super(props);
  }

  shouldComponentUpdate(nextProps, nextState) {
    return this.props.row.id != nextProps.row.id;
  }

  render() {
    const { classes } = this.props;
    const row = this.props.row;
    const curXyzCoords = toXyzArSplit(row);
    return (
          <GridItem xs={12} sm={12} md={6}>
       <CustomTabs
              title={"2D Eigen Contours"}
              headerColor="info"
              tabs={[
                {
                  tabName: "Vac",
                  tabIcon: VacIcon,
                  tabContent: (
                 <HeatMapC coords={curXyzCoords[1]}/>
                  ),
                  footerContent: (
                    <div className={classes.stats}>
                      <VacIcon/> Shows density variations, helpful in estimating major subcascades
                    </div>
                  )
                },
                {
                  tabName: "Int",
                  tabIcon: IntIcon,
                  tabContent: (
                 <HeatMapC coords={curXyzCoords[0]}/>
                  ),
                  footerContent: (
                    <div className={classes.stats}>
                      <IntIcon/> Shows density variations, helpful in estimating major clusters of interstitials.
                    </div>
                  )
                },
                {
                  tabName: "Both",
                  tabIcon: AllIcon,
                  tabContent: (
                 <HeatMapC coords={curXyzCoords[2]}/>
                 ),
                  footerContent: (
                    <div className={classes.stats}>
                      <AllIcon/> Shows density variations, helpful in estimating size along major principle axes.
                    </div>
                  )
                }
              ]}
            />
          </GridItem>
    );
  }
}

export class CascadesAndClusterCmp extends React.Component {
  constructor(props) {
    super(props);
  }
  
  shouldComponentUpdate(nextProps, nextState) {
    return uniqueKey(this.props.row) != uniqueKey(nextProps.row) || this.props.cid != nextProps.cid;
  }

  /*
  static getDerivedStateFromProps(props, state) {
    if (props.row.id !== this.props.row.id) {
      return {
        prevPropsUserID: props.userID,
        email: props.defaultEmail
      };
    }
    return null;
  }
  */
   
  render() {
    const { classes, row, allCids, cid, data} = this.props;
    return (
      <ExpansionPanel>
        <ExpansionPanelSummary expandIcon={<ExpandMoreIcon />}>
          <div className={classes.column}>
            <Typography className={classes.heading}>Visualize and Find Patterns</Typography>
          </div>
          <div className={classes.column}>
            <Typography className={classes.secondaryHeading}>Currently Viewing - {this.props.shortName(row)}</Typography>
          </div>
        </ExpansionPanelSummary>
        <ExpansionPanelDetails className={classes.details}>
        <Grid container>
          <CascadeViews1 classes={classes} row = {row} handleClusterCmp={this.props.handleClusterCmp}/>
          <CascadeViews2 classes={classes} row = {row} />
          <GridItem xs={12} sm={12} md={12}>
          <ClusterCmpPlot classes={classes} row={row} cid={cid} data={data} handleClusterCmp={this.props.handleClusterCmp} shortName={this.props.shortName} allCids={allCids}/>
          </GridItem>
       </Grid>
        </ExpansionPanelDetails>
        </ExpansionPanel>
    );
  }
}