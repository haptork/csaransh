import "babel-polyfill";
import React from "react";
import { render } from "react-dom";
import PropTypes from "prop-types";
import Dashboard from "./Dashboard/Dashboard";

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


  render() {
    const { classes, ...rest } = this.props;
    return (
      <div>
       <Dashboard />
        </div>
      );
  }
}

App.propTypes = {
  classes: PropTypes.object.isRequired
};
render(<App />, document.getElementById("root"));