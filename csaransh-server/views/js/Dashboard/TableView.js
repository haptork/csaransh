import React from "react";
import PropTypes from "prop-types";
import ArchiveIcon from '@material-ui/icons/Archive';
import SaveIcon from '@material-ui/icons/Save';
import {CopyToClipboard} from 'react-copy-to-clipboard';
// react plugin for creating charts
// @material-ui/core
import withStyles from "@material-ui/core/styles/withStyles";
// @material-ui/icons
import ContentCopy from "@material-ui/icons/ContentCopy";
import BugReport from "@material-ui/icons/BugReport";
import Code from "@material-ui/icons/Code";
// core components
//import CustomTabs from "components/CustomTabs/CustomTabs.js";
import CustomTabs from "./FabCustomTabs.js";
import ListIcon from '@material-ui/icons/List';

import Button from '@material-ui/core/Button';
import Tooltip from '@material-ui/core/Tooltip';

// charts import
import { InputInfo, OutputInfo, DefectsList } from "../cascades/InfoModal";

class TableView extends React.Component {

  constructor(props) {
    super(props);
  }

  toggleLock = () => {
    this.props.handleLock(this.props.id);
  };

  render() {
    /*
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
    */
   return (
            <CustomTabs
              title={"Details#"+this.props.id}
              isLocked={this.props.lock}
              handleLock={this.toggleLock}
              headerColor={this.props.headerColor}
              tabs={[
                {
                  tabName: "Input",
                  tabIcon: Code,
                  tabContent: (
                    <InputInfo row={this.props.row} />
                  )
                },
                {
                  tabName: "Overview",
                  tabIcon: BugReport,
                  tabContent: (
                    <OutputInfo row={this.props.row} />
                  )
                },
                {
                  tabName: "Defects",
                  tabIcon: ListIcon,
                  tabContent: (
                    <DefectsList row={this.props.row} />
                  )
                }
              ]}
            >
            <div  className="fabButtonsInfo">
           <Tooltip id="tooltip-dlin" title="Download Input file" placement="top">
           <Button style={{marginRight:"4px"}} href={'https://www-amdis.iaea.org/CDB/challenge/data/'+ this.props.row.infile + ".in"} variant="fab">
              <SaveIcon/>
            </Button>
            </Tooltip>

           <Tooltip id="tooltip-dlout" title="Download atomic coordinates file" placement="top">
           <Button style={{marginLeft:"4px"}}href={'https://www-amdis.iaea.org/CDB/challenge/data/'+ this.props.row.infile.replace("md", "fpos") + ".xyz"} variant="fab">
              <ArchiveIcon/>
            </Button>
            </Tooltip>
            </div>
           <Tooltip id="tooltip-cp1" title="Copy all results to clipboard" placement="top">
           <CopyToClipboard className="fabButtonsInfo" text={JSON.stringify(this.props.row)}>
           <Button variant="fab" >
              <ContentCopy/>
            </Button>
            </CopyToClipboard>
            </Tooltip>
           <Tooltip id="tooltip-cp2" title="Copy list to clipboard" placement="top">
           <CopyToClipboard className="fabButtonsInfo" text={JSON.stringify(this.props.row.coords)}>
           <Button variant="fab" >
              <ContentCopy/>
            </Button>
            </CopyToClipboard>
            </Tooltip>

            </CustomTabs>
    );
  }
}

TableView.propTypes = {
  classes: PropTypes.object.isRequired,
  /*
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
  */
};

TableView.propTypes = {
  classes: PropTypes.object.isRequired,
}

const customTableViewStyle = {
  primary: { }
};

export default withStyles(customTableViewStyle)(TableView);
