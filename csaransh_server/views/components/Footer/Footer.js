import React from "react";
import PropTypes from "prop-types";
// @material-ui/core components
import withStyles from "@material-ui/core/styles/withStyles";
import ListItem from "@material-ui/core/ListItem";
import List from "@material-ui/core/List";
// core components
import footerStyle from "assets/jss/material-dashboard-react/components/footerStyle";

function Footer({ ...props }) {
  const { classes } = props;
  return (
    <footer className={classes.footer}>
      <div className={classes.container}>
        <div className={classes.left}>
          <List className={classes.list}>
            <ListItem className={classes.inlineBlock}>
            <a href="https://github.com/haptork/csaransh" className={classes.a}>
              Github Repository
            </a>
            </ListItem>
            <ListItem className={classes.inlineBlock}>
            <a href="http://joss.theoj.org/papers/72f2ddde2112497826753319956ea8ab" className={classes.a}>
              For Csaransh citation
            </a>
            </ListItem>
            <ListItem className={classes.inlineBlock}>
            <a href="https://haptork.github.io/csaransh/presentation/index.html" className={classes.a}>
              Cluster classification presentation
            </a>
            </ListItem>
             <ListItem className={classes.inlineBlock}>
            <a href="https://doi.org/10.1016/j.commatsci.2019.109364" className={classes.a}>
              Defect identification & cluster classification paper
            </a>
            </ListItem>
          </List>
       </div>
        <p className={classes.right}>
          <span>
            &copy; {1900 + new Date().getYear()}{" "}
            <a href="/" className={classes.a}>
              CAD BARC
            </a>, India
          </span>
        </p>
      </div>
    </footer>
  );
}
/*
          <List className={classes.list}>
            <ListItem className={classes.inlineBlock}>
              <a href="#home" className={classes.block}>
                About
              </a>
            </ListItem>
            <ListItem className={classes.inlineBlock}>
              <a href="#company" className={classes.block}>
                Contant
              </a>
            </ListItem>
          </List>
*/ 

Footer.propTypes = {
  classes: PropTypes.object.isRequired
};

export default withStyles(footerStyle)(Footer);
