import React from 'react';
import { getColor } from "../utils";
import { NDefectsPlot } from  "./nDefectsPlot";
import { SplomPlot } from "./SplomPlot";
import { DefectDistancePlot, calcStatDistsAngles } from "../cascades/DefectDistancePlot";
import StatsIcon from '@material-ui/icons/MultilineChart';

import Grid from "@material-ui/core/Grid";
import GridItem from "components/Grid/GridItem.js";

import Card from "components/Card/Card.js";
import CardHeader from "components/Card/CardHeader.js";
import CardBody from "components/Card/CardBody.js";
import CardFooter from "components/Card/CardFooter.js";

import CustomTabs from "components/CustomTabs/CustomTabs.js";

import AngleIcon from "@material-ui/icons/CallSplit";
import DistIcon from "@material-ui/icons/LinearScale";

const accessorDefault = name => x => x[name];
const accessorOned = x => parseFloat(x["eigen_var"][0]);
const accessorTwod = x => parseFloat(x["eigen_var"][0]) + parseFloat(x["eigen_var"][1]);
const accessorSubc = x => (Object.keys(x.dclust_coords).length) <= 1 ? 0 : (Object.keys(x.dclust_coords).length);

const groupBars = (data, fields) => {
  let keys = [];
  let keys2 = [];
  const defaultType = "box";
  let vals = [];
  let mx = [];
  let mn = [];
  for (const key in fields) {
    let field = fields[key];
    if (!field.hasOwnProperty("type")) {
      field.type = defaultType;
    }
    if (!field.hasOwnProperty("accessor")) {
      field.accessor = accessorDefault(field.id);
    }
    vals.push({label:key, values:[]});
    mx.push(0.0);
    mn.push(0.0);
  }
  for (const x of data) {
    keys.push(x.substrate + "_" + x.energy);
    keys2.push(x.name);
    for (let val of vals) {
      const key = val.label;
      val.values.push(fields[key].accessor(x));
    }
    const lastElem = vals[0].values.length - 1;
    for (let i = 0; i < mn.length; ++i) {
      if (lastElem == 0) {
        mn[i] = vals[i].values[lastElem];
        mx[i] = mn[i];
      } else {
        mn[i] = Math.min(mn[i], vals[i].values[lastElem]);
        mx[i] = Math.max(mx[i], vals[i].values[lastElem]);
      }
    }
  }
  let d1 = [];
  let i = 0;

  for (let val of vals) {
    if (fields[val.label].type != "box") continue;
    d1.push(
      {
        y: val.values.slice(),
        x: keys,
        visible: (i == 0),
        name: val.label,
        marker: {color: getColor(i)},
        type: 'box',
        meanline: {
          visible: true
        },
        boxmean: true,
        jitter: 0.3,
        pointpos: -1.5,
        boxpoints: 'all'
      }
    );
    i++;
  }
  // normalization
  for (let i = 0; i < vals.length; ++i) {
    for (let j in vals[i].values) {
      vals[i].values[j] = (vals[i].values[j] - mn[i]) / (mx[i] - mn[i]);
    }
  }
  return [d1, keys2, vals];
};

export class Statistics extends React.Component {
  constructor(props) {
    super(props);
    this.fields = {
      "defects count":{"id":"n_defects"},
      "max int size":{"id":"max_cluster_size_I"}, 
      "max vac size":{"id":"max_cluster_size_V"}, 
      "int in cluster":{"id":"in_cluster_I"}, 
      "vac in cluster":{"id":"in_cluster_V"}, 
      "planarity":{"id":"planarity", "accessor":accessorTwod }, 
      "subcascades":{"id":"subc", "accessor":accessorSubc }, 
      "subcascade impact":{"id":"dclust_sec_impact"},
      "energy":{"id":"energy", "type":"no-box"}
    };
  }

  shouldComponentUpdate(nextProps, nextState){
    return this.props.data != nextProps.data;
  }

  render() {
    const { classes } = this.props;
    const [nDefects, splomKeys, splomVals] = groupBars(this.props.data, this.fields); 
    const [statDists, statAngles] = calcStatDistsAngles(this.props.data);
    return (
      <Grid container justify="center">
        <GridItem xs={12} sm={12} md={12}>
          <Card chart>
            <CardHeader color="info"> Statistics grouped by Elements and Energy </CardHeader>
            <CardBody>
              <NDefectsPlot data= {nDefects} fields = {this.fields}/>
            </CardBody>
            <CardFooter chart>
              <div className={classes.stats}>
                <StatsIcon /> Statistical Plot - Correlations, colors represent number of defects, filter data and remove outliers from table action buttons for better idea on correlations.
              </div>
            </CardFooter>
          </Card>
        </GridItem>
        <GridItem xs={12} sm={12} md={12}>
          <Card>
            <CardHeader color="primary">
              <h4 className={classes.cardTitleWhite}>Splom Plots - Correlations</h4>
            </CardHeader>
            <CardBody id="splomCardBody">
              <SplomPlot keys={splomKeys} vals={splomVals}/>
            </CardBody>
            <CardFooter chart>
              <div className={classes.stats}>
                <StatsIcon /> Statistical Plot - Correlations, colors represent number of defects, filter data and remove outliers from table action buttons for better idea on correlations.
              </div>
            </CardFooter>
          </Card>
        </GridItem>
       <GridItem xs={12} sm={12} md={12}>
       <CustomTabs
              title={"Distribution from PKA"}
              headerColor="warning"
              tabs={[
                {
                  tabName: "Angles",
                  tabIcon: AngleIcon,
                  tabContent: (
                <DefectDistancePlot
                  data={statAngles}
                  title={""}
                  xlabel={"Angstroms"}
                  type={"Statistics"}
                  height={"480px"}
                />
                  ),
                  footerContent: (
                    <div className={classes.stats}>
                      <StatsIcon/> Distribtion of defects distances from PKA position
                    </div>
                  )

                },
                {
                  tabName: "Distances",
                  tabIcon: DistIcon,
                  tabContent: (
                <DefectDistancePlot
                  data={statDists}
                  title={""}
                  xlabel={"Angstroms"}
                  type={"Statistics"}
                  height={"480px"}
                />
                  ),
                  footerContent: (
                    <div className={classes.stats}>
                      <StatsIcon/> Angular Distribtion of defects from PKA direction
                    </div>
                  )

                }
              ]}
            />
          </GridItem>
     </Grid>
    );
  }
}