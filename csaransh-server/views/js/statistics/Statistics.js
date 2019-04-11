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
import Code from "@material-ui/icons/Code";
import BugReport from "@material-ui/icons/BugReport";

export const groupBars = (data) => {
  let keys = [];
  let keys2 = [];
  /* n_defects = []; max_sizeI = []; max_sizeV = []; in_clusterI = []; in_clusterV = [];  */
  const names = ["defects count", "max int cluster", "max vac size", "int in cluster", "vac in cluster", "subcascades", "subcasd impact", "eigen dim1", "eigen dim2", "eigen dim1+2", "energy"];

  let vals = [{label:names[0], values:[]}, {label:names[1], values:[]}, {label:names[2], values:[]},{label:names[3], values:[]},
              {label:names[4], values:[]}, {label:names[5], values:[]}, {label:names[6], values:[]},{label:names[7], values:[]},
              {label:names[8], values:[]}, {label:names[9], values:[]}, {label:names[10], values:[]}];

  let mx = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0];
  let mn = [100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0, 100000.0]; // valid assumption
  for (const x of data) {
    keys.push(x.substrate + "_" + x.energy);
    keys2.push(x.name);
    vals[0].values.push(x.n_defects);
    vals[1].values.push(x.max_cluster_size_I);
    vals[2].values.push(x.max_cluster_size_V);
    vals[3].values.push(x.in_cluster_I);
    vals[4].values.push(x.in_cluster_V);
    vals[5].values.push(x.density_cluster_vac.length); // s
    vals[6].values.push(parseInt(x.density_cluster_vac.length <= 1 ? 0 : x.clusters[x.density_cluster_vac[1]].length * 100 / x.clusters[x.density_cluster_vac[0]].length));
    vals[7].values.push(x.eigen_var[0]);
    vals[8].values.push(x.eigen_var[1]);
    vals[9].values.push(x.eigen_var[0] + x.eigen_var[1]);
    vals[10].values.push(x.energy);
    const lastElem = vals[0].values.length - 1;
    for (let i = 0; i < mn.length; ++i) {
      mn[i] = Math.min(mn[i], vals[i].values[lastElem]);
      mx[i] = Math.max(mx[i], vals[i].values[lastElem]);
    }
  }
  let d1 = [];
  let i = 0;

  for (let val of vals) {
    if (i == 10) break;
    d1.push(
      {
        y: val.values.slice(),
        x: keys,
        visible: (i == 0),
        name: names[i],
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
  }

  shouldComponentUpdate(nextProps, nextState){
    return this.props.data != nextProps.data;
  }

  render() {
    const { classes } = this.props;
    const [nDefects, splomKeys, splomVals] = groupBars(this.props.data); 
    const [statDists, statAngles] = calcStatDistsAngles(this.props.data);
    return (
      <Grid container justify="center">
        <GridItem xs={12} sm={12} md={12}>
          <Card chart>
            <CardHeader color="info"> Statistics grouped by Elements and Energy </CardHeader>
            <CardBody>
              <NDefectsPlot data= {nDefects} />
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
                  tabIcon: Code,
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
                  tabIcon: BugReport,
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