import React from "react";
import Grid from "@material-ui/core/Grid";
import GridItem from "components/Grid/GridItem.js";
import Card from "components/Card/Card.js";
import CardHeader from "components/Card/CardHeader.js";
import CardIcon from "components/Card/CardIcon.js";
// Icons
import ViewIcon from '@material-ui/icons/BubbleChart';
import WbIridescentIcon from "@material-ui/icons/WbIridescent";
import Accessibility from "@material-ui/icons/Accessibility";
import Store from "@material-ui/icons/Store";

const Item = props => {
  const classes = props.classes;
  const Icon = props.icon;
  return (
    <GridItem xs={12} sm={6} md={3}>
      <Card>
        <CardHeader color={props.color} stats icon>
          <CardIcon color={props.color}>
            <Icon />
          </CardIcon>
          <p className={classes.cardCategory}>{props.val["title"]}</p>
          <h3 className={classes.cardTitle}>
            {props.val["label"]} <small>{props.val["labelSm"]}</small>
          </h3>
        </CardHeader>
      </Card>
    </GridItem>
  );
};

export class OutlineCards extends React.Component {
  constructor(props) {
    super(props);
  }

  shouldComponentUpdate(nextProps, nextState){
    return false;
  }

  render () {
    const props = this.props;
    const colors = ["success", "warning", "primary", "info"];
    const icons = [Store, WbIridescentIcon, Accessibility, ViewIcon];
    const x = colors.map((c, i) => { return ( <Grid container className="content"> <Item color={colors[0]} icon={icons[0]} val={props.values[0]} classes={props.classes}/> </Grid>);});
    return (
      <Grid container className="content"> 
      {colors.map((c, i) => <Item color={c} icon={icons[i]} val={props.values[i]} classes={props.classes}/>)}
      </Grid>
    );
  }

}
    //<Grid container className="content"> <Item color={colors[0]} icon={icons[0]} val={props.values[0]} classes={props.classes}/> </Grid>