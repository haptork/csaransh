
/* eslint-disable */
import React from "react";
import PropTypes from "prop-types";
// creates a beautiful scrollbar
import PerfectScrollbar from "perfect-scrollbar";
import "perfect-scrollbar/css/perfect-scrollbar.css";
// @material-ui/core components
import withStyles from "@material-ui/core/styles/withStyles";
// core components
import Header from "components/Header/Header.js";
import Footer from "components/Footer/Footer.js";
import Sidebar from "components/Sidebar/Sidebar.js";
import Dashboard from "./Dashboard.js";

import dashboardStyle from "assets/jss/material-dashboard-react/layouts/dashboardStyle.js";

//import image from "images/sidebar-2.jpg";
//import logo from "images/reactlogo.png";

class App extends React.Component {
  state = {
    mobileOpen: false
  };
  handleDrawerToggle = () => {
    this.setState({ mobileOpen: !this.state.mobileOpen });
  };
  getRoute() {
    return this.props.location.pathname !== "/maps";
  }
  componentDidMount() {
    if (navigator.platform.indexOf("Win") > -1) {
      const ps = new PerfectScrollbar(this.refs.mainPanel);
    }
  }
  componentDidUpdate(e) {
    if (e.history.location.pathname !== e.location.pathname) {
      this.refs.mainPanel.scrollTop = 0;
      if(this.state.mobileOpen){
        this.setState({mobileOpen: false});
      }
    }
  }
  render() {
    const { classes, ...rest } = this.props;
    return (
      <div className={classes.wrapper}>
        <Sidebar
          logoText={"Creative Tim"}
          logo={"images/reactlogo.png"}
          image={""}
          handleDrawerToggle={this.handleDrawerToggle}
          open={this.state.mobileOpen}
          color="blue"
          {...rest}
        />
        <div className={classes.mainPanel} ref="mainPanel">
          <Header
            handleDrawerToggle={this.handleDrawerToggle}
            {...rest}
          />

          <Dashboard/>
          <Footer />
        </div>
      </div>
    );
  }
}

App.propTypes = {
  classes: PropTypes.object.isRequired
};

export default withStyles(dashboardStyle)(App);
