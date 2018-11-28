import React from "react";
import Button from '@material-ui/core/Button';
// nodejs library that concatenates classes
import classNames from "classnames";
import SwipeableViews from 'react-swipeable-views';
// nodejs library to set properties for components
import PropTypes from "prop-types";

import Tooltip from '@material-ui/core/Tooltip';
// material-ui components
import withStyles from "@material-ui/core/styles/withStyles";
import Tabs from "@material-ui/core/Tabs";
import Tab from "@material-ui/core/Tab";
// core components
import Card from "components/Card/Card.js";
import CardBody from "components/Card/CardBody.js";
import CardHeader from "components/Card/CardHeader.js";
import Zoom from '@material-ui/core/Zoom';
import IconButton from '@material-ui/core/IconButton';
import LockIcon from '@material-ui/icons/Lock';
import LockOpenIcon from '@material-ui/icons/LockOpen';

import customTabsStyle from "assets/jss/material-dashboard-react/components/customTabsStyle.js";

    const transitionDuration = {
      enter: 100,
      exit: 200
    };
class CustomTabs extends React.Component {

  constructor(props) {
    super(props);
    this.state = {
      value: 0
    };
  }

  handleChange = (event, value) => {
    this.setState({ value });
  };

  render() {
    const {
      classes,
      headerColor,
      plainTabs,
      tabs,
      title,
      rtlActive
    } = this.props;
    const cardTitle = classNames({
      [classes.cardTitle]: true,
      [classes.cardTitleRTL]: rtlActive
    });
    return (
      <Card plain={plainTabs}>
        <CardHeader color={headerColor} plain={plainTabs}>
          {title !== undefined ? (
            <div className={cardTitle}>
           <Tooltip id={"tooltip-lk"+this.props.id} title="Lock view for comparisonn" placement="top">
          <Button className="lockButton" size="small" onClick={this.props.handleLock}>
          {this.props.isLocked ? <LockIcon/> : <LockOpenIcon/>}
          </Button>
          </Tooltip>

            {title}</div>
          ) : null}
           {this.props.subtitle !== undefined ? (
                <p className={classes.cardCategoryWhite}>
                  {this.props.subtitle}
                </p>
           ) : null}
 
          <Tabs
            value={this.state.value}
            onChange={this.handleChange}
            classes={{
              root: classes.tabsRoot,
              indicator: classes.displayNone
            }}
            scrollable
            scrollButtons="auto"
          >
            {tabs.map((prop, key) => {
              var icon = {};
              if (prop.tabIcon) {
                icon = {
                  icon: <prop.tabIcon />
                };
              }
              return (
                <Tab
                  classes={{
                    root: classes.tabRootButton,
                    labelContainer: classes.tabLabelContainer,
                    label: classes.tabLabel,
                    selected: classes.tabSelected,
                    wrapper: classes.tabWrapper
                  }}
                  key={key}
                  label={prop.tabName}
                  {...icon}
                />
              );
            })}
          </Tabs>
        </CardHeader>
        <CardBody>
        <SwipeableViews
          index={this.state.value}
          onChangeIndex={this.handleChange}
        >
          {tabs.map((prop, key) => {
            //if (key === this.state.value) {
              return <div key={key}>{prop.tabContent}</div>;
            //}
            //return null;
          })}
        </SwipeableViews>
         {this.props.children.map((fab, index) => (
          <Zoom
            key={index}
            in={this.state.value === index}
            timeout={transitionDuration}
            style={{
              transitionDelay: this.state.value === index ? transitionDuration.exit : 0,
            }}
            unmountOnExit
          >
          {fab}
         </Zoom>
        ))}
        </CardBody>
      </Card>
    );
  }
}

CustomTabs.propTypes = {
  classes: PropTypes.object.isRequired,
  headerColor: PropTypes.oneOf([
    "warning",
    "success",
    "danger",
    "info",
    "primary"
  ]),
  title: PropTypes.string,
  tabs: PropTypes.arrayOf(
    PropTypes.shape({
      tabName: PropTypes.string.isRequired,
      tabIcon: PropTypes.func,
      tabContent: PropTypes.node.isRequired
    })
  ),
  rtlActive: PropTypes.bool,
  plainTabs: PropTypes.bool
};

export default withStyles(customTabsStyle)(CustomTabs);
