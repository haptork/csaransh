import React from "react";
// nodejs library that concatenates classes
import classNames from "../../../../../../../Library/Caches/typescript/2.9/node_modules/@types/classnames./../../../Library/Caches/typescript/2.9/node_modules/@types/classnames";
// nodejs library to set properties for components
import PropTypes from "../../../../../../../Library/Caches/typescript/2.9/node_modules/@types/prop-types./../../../Library/Caches/typescript/2.9/node_modules/@types/prop-types";
// @material-ui/core components
import withStyles from "@material-ui/core/styles/withStyles";
// @material-ui/icons

// core components
import cardFooterStyle from "assets/jss/material-dashboard-react/components/cardFooterStyle.js";

function CardFooter({ ...props }) {
  const {
    classes,
    className,
    children,
    plain,
    profile,
    stats,
    chart,
    ...rest
  } = props;
  const cardFooterClasses = classNames({
    [classes.cardFooter]: true,
    [classes.cardFooterPlain]: plain,
    [classes.cardFooterProfile]: profile,
    [classes.cardFooterStats]: stats,
    [classes.cardFooterChart]: chart,
    [className]: className !== undefined
  });
  return (
    <div className={cardFooterClasses} {...rest}>
      {children}
    </div>
  );
}

CardFooter.propTypes = {
  classes: PropTypes.object.isRequired,
  className: PropTypes.string,
  plain: PropTypes.bool,
  profile: PropTypes.bool,
  stats: PropTypes.bool,
  chart: PropTypes.bool
};

export default withStyles(cardFooterStyle)(CardFooter);
