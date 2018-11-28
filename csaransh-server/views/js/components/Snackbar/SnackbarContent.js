import React from "react";
import PropTypes from "../../../../../../../Library/Caches/typescript/2.9/node_modules/@types/prop-types./../../../Library/Caches/typescript/2.9/node_modules/@types/prop-types";
import classNames from "../../../../../../../Library/Caches/typescript/2.9/node_modules/@types/classnames./../../../Library/Caches/typescript/2.9/node_modules/@types/classnames";
// @material-ui/core components
import withStyles from "@material-ui/core/styles/withStyles";
import Snack from "@material-ui/core/SnackbarContent";
import IconButton from "@material-ui/core/IconButton";
// @material-ui/icons
import Close from "@material-ui/icons/Close";
// core components
import snackbarContentStyle from "assets/jss/material-dashboard-react/components/snackbarContentStyle.js";

function SnackbarContent({ ...props }) {
  const { classes, message, color, close, icon } = props;
  var action = [];
  const messageClasses = classNames({
    [classes.iconMessage]: icon !== undefined
  });
  if (close !== undefined) {
    action = [
      <IconButton
        className={classes.iconButton}
        key="close"
        aria-label="Close"
        color="inherit"
      >
        <Close className={classes.close} />
      </IconButton>
    ];
  }
  return (
    <Snack
      message={
        <div>
          {icon !== undefined ? <props.icon className={classes.icon} /> : null}
          <span className={messageClasses}>{message}</span>
        </div>
      }
      classes={{
        root: classes.root + " " + classes[color],
        message: classes.message
      }}
      action={action}
    />
  );
}

SnackbarContent.propTypes = {
  classes: PropTypes.object.isRequired,
  message: PropTypes.node.isRequired,
  color: PropTypes.oneOf(["info", "success", "warning", "danger", "primary"]),
  close: PropTypes.bool,
  icon: PropTypes.func
};

export default withStyles(snackbarContentStyle)(SnackbarContent);
