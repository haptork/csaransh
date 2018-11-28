import "babel-polyfill";
import React from "react";
import { render } from "react-dom";
import { getData } from "../utils";
/*
import { MainTable } from "./MainTable";
import { NDefectsPlot } from "../statistics/nDefectsPlot";
import {
  ClusterSizePlot,
  addDefectsSizeDistrib,
  removeDefectsSizeDistrib
} from "./ClusterSizePlot";
import {
  DefectDistancePlot,
  addDefectDistance,
  addDefectAngles,
  removeDefectDistance,
  calcStatDistsAngles,
} from "./DefectDistancePlot";
import {
  CascadeVisualizer3D,
  getCurrentCascade,
  //removeCurrentCascade
} from "./CascadeVisualizer3D";
*/
import Grid from "@material-ui/core/Grid";
import Card from "components/Card/Card";
import CardHeader from "components/Card/CardHeader";
import CardIcon from "components/Card/CardIcon";
import CardBody from "components/Card/CardBody";
import CardFooter from "components/Card/CardFooter";
import GridItem from "components/Grid/GridItem";
import Table from "components/Table/Table";
import Tasks from "components/Tasks/Tasks";
import CustomTabs from "components/CustomTabs/CustomTabs";
import Danger from "components/Typography/Danger";
import Header from "components/Header/Header.js";
import Footer from "components/Footer/Footer.js";
import AppBar from "@material-ui/core/AppBar";
import Toolbar from "@material-ui/core/Toolbar";
import PropTypes from "prop-types";
import dashboardStyle from "assets/jss/material-dashboard-react/layouts/dashboardStyle.js";
import Dashboard from "../Dashboard/Dashboard.js";
//-import DashboardLayout from "../Dashboard/DashboardLayout.js";
//import Sidebar from "components/Sidebar/Sidebar.js";

//import PKADistributionPlots from "./PKADistributionPlots";

class App extends React.Component {
  constructor(props) {
    super(props);
    const data = [];//getData();
    this.state = {
      data: data,
      loadedCascades: [],
      curRows: data,
      DefectSizeDistribConfig: {},
      defectDistances: [],
      defectAngles: [],
      focusOnClusterId: undefined,
      selected: new Set(),
      except: new Set(),
      look: "",
      statDists: [],
      statAngles: []
    };
  }

/*
  setRows(curRows) {
    const [statDists, statAngles] = calcStatDistsAngles(curRows);
    this.setState({
      curRows,
      statDists,
      statAngles
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
    this.setState({
      cascadeData: getCurrentCascade(this.state.loadedCascades, row),
      look: row.name
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
      <Toolbar>
        <div>
        what
        </div>
      </Toolbar>
    </AppBar>
    <div>
      what
      </div>
      </h2>);

*/

render() {
  const { classes, ...rest } = this.props;
  return (
    <div>
     <Dashboard />
      </div>
    );


    /*
      <div className="content-wrapper">
        <Header/>
        <Grid container>
          <GridItem xs={12} sm={6}>
            <Card outline color="success">
              <CardBody id="VizGl">
                <div>
                  hey
                </div>
              </CardBody>
            </Card>
          </GridItem>
        </Grid>
        <Footer />
      </div>
          );
          */
 }
   
/*
  render() {
    return (
      <div className="content-wrapper">
        <Grid container>
          <GridItem xs={12} sm={6}>
            <Card outline color="success">
              <CardBody id="VizGl">
                <CardTitle>3D Visualizer - Cascade View</CardTitle>
                  <CascadeVisualizer3D
                    data={this.state.cascadeData}
                    focusOn={this.state.focusOnClusterId}
                  />
              </CardBody>
            </Card>
          </GridItem>
          <GridItem xs={12} sm={6}>
            <Card outline color="warning">
              <CardBody id="ClusterSizeCompare">
                <CardTitle>Compare Defect Sizes in Cascades</CardTitle>
                <ClusterSizePlot data={this.state.DefectSizeDistribConfig} height={296}
                />
              </CardBody>
            </Card>
          </GridItem>
        </Grid>
        <Grid container>
          <GridItem xs={12}>
            <Card outline color="primary">
              <CardBody>
                <CardTitle> List of Cascades
                </CardTitle>
                <MainTable
                  data={this.state.data}
                  selected={this.state.selected}
                  except={this.state.except}
                  look={this.state.look}
                  onViewCur={o => this.viewCurRowHandler(o)}
                  onLookCur={o => this.lookCurRowHandler(o)}
                  onExceptCur={o => this.exceptCurRowHandler(o)}
                  setRows={(rows) => this.setRows(rows)}
                />
              </CardBody>
            </Card>
          </GridItem>
        </Grid>
        <Grid container>
          <GridItem>
            <Card outline color="info">
              <CardBody>
                <CardTitle>Statistics</CardTitle>
                <NDefectsPlot
                  data= {this.state.curRows}
                />
              </CardBody>
            </Card>
          </GridItem>
        </Grid>


        <Grid container>
          <GridItem xs={12}>
            <PKADistributionPlots
              statDists={this.state.statDists}
              statAngles={this.state.statAngles}
              compareDists={this.state.defectDistances}
              compareAngles={this.state.defectAngles}
            />
         </GridItem>
        </Grid>
 
      </div>
    );
  }
  */
}

App.propTypes = {
  classes: PropTypes.object.isRequired
};


render(<App />, document.getElementById("root"));

//import { Popover, Tooltip, Button, Modal, OverlayTrigger } from 'react-bootstrap';
/*
.wrapper {
    position: relative;
    overflow: hidden;
    width: 100px;
    height: 100px; 
    border: 1px solid black;
}

#slide {
    position: absolute;
    left: -100px;
    width: 100px;
    height: 100px;
    background: blue;
    transition: 1s;
}

.wrapper:hover #slide {
    transition: 1s;
    left: 0;
}

<div class="wrapper">
    <img id="slide" src="http://lorempixel.com/output/cats-q-c-100-100-4.jpg" />
</div>
*/
