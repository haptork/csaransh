import "babel-polyfill";
import React from "react";
import { render } from "react-dom";
import { getData } from "../utils";
import { MainTable } from "../cascades/MainTable";
import { Grid, Row, Col, Panel } from "react-bootstrap";
import {
  ClusterSizePlot,
  addDefectsSizeDistrib,
  removeDefectsSizeDistrib
} from "../cascades/ClusterSizePlot";
import {
  DefectDistancePlot,
  addDefectDistance,
  addDefectAngles,
  removeDefectDistance,
} from "../cascades/DefectDistancePlot";
import {
  CascadeVisualizer3D,
  getCurrentCascade,
  removeCurrentCascade
} from "../cascades/CascadeVisualizer3D";

class App extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      data: getData(),
      loadedCascades: [],
      curRows: getData().slice(),
      DefectSizeDistribConfig: {},
      defectDistances: [],
      defectAngles: [],
      focusOnClusterId: undefined,
      selected: new Set()
    };
  }

  setRows(curRows) {
    console.log(curRows);
    this.setState({ curRows });
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
        cascadeData: getCurrentCascade(this.state.loadedCascades, row),
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
        cascadeData: removeCurrentCascade(this.state.loadedCascades, row),
        selected
      });
    }
  }

  render() {
    return (
      <Grid>
        <Row>
          <Col sm={6}>
            <Panel defaultExpanded bsStyle="warning">
              <Panel.Heading>
                <Panel.Title toggle>Cascade Visualizer</Panel.Title>
              </Panel.Heading>
              <Panel.Body collapsible>
                <CascadeVisualizer3D
                  data={this.state.cascadeData}
                  focusOn={this.state.focusOnClusterId}
                />
              </Panel.Body>
            </Panel>
          </Col>
          <Col sm={6}>
            <Panel defaultExpanded bsStyle="warning">
              <Panel.Heading>
                <Panel.Title toggle>Cluster Size Distribution</Panel.Title>
              </Panel.Heading>
              <Panel.Body collapsible>
                <ClusterSizePlot data={this.state.DefectSizeDistribConfig} />
              </Panel.Body>
            </Panel>
          </Col>
        </Row>
        <Row>
          <Panel defaultExpanded bsStyle="info">
            <Panel.Heading>
              <Panel.Title toggle>List of Cascades</Panel.Title>
            </Panel.Heading>
            <Panel.Body collapsible>
              <MainTable
                data={this.state.data}
                selected={this.state.selected}
                onViewCur={o => this.viewCurRowHandler(o)}
                setRows={rows => this.setRows(rows)}
              />
            </Panel.Body>
          </Panel>
        </Row>
        <Row>
          <Panel defaultExpanded bsStyle="warning">
            <Panel.Heading>
              <Panel.Title toggle>
                Distribution of Defects from PKA origin
              </Panel.Title>
            </Panel.Heading>
            <Panel.Body collapsible>
              <Col sm={6}>
                <DefectDistancePlot
                  data={this.state.defectDistances}
                  title={"Distance"}
                  xlabel={"Angstroms"}
                />
              </Col>
              <Col sm={6}>
                <DefectDistancePlot
                  data={this.state.defectAngles}
                  title={"Angle"}
                  xlabel={"Degree"}
                />
              </Col>
            </Panel.Body>
          </Panel>
        </Row>
      </Grid>
    );
  }
}

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
