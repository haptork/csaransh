import React from 'react';

import Grid from "@material-ui/core/Grid";
import GridItem from "components/Grid/GridItem.js";

import Table from '@material-ui/core/Table';
import TableBody from '@material-ui/core/TableBody';
import TableCell from '@material-ui/core/TableCell';
import TableRow from '@material-ui/core/TableRow';
import withStyles from "@material-ui/core/styles/withStyles";
import Tabley from "components/Table/Table.js";

const CustomTableCell = withStyles(theme => ({
  head: {
    backgroundColor: theme.palette.common.black,
    color: theme.palette.common.white,
  },
  body: {
    color:"#888"
  },
}))(TableCell);


const inputData1 = (row) => [
                    ["File", row.xyzFilePath],
                    ["Substrate", row.substrate],
                    ["Energy", row.energy],
                    ["Cells", row.ncell]
];

const inputData2 = (row) => [
                    ["Box-size", row.boxSize],
                    ["PKA Angles(θ, φ)", row.rectheta + ", " + row.recphi],
                    ["PKA Position", row.xrec + ", " + row.yrec + ", " + row.zrec],
                    ["Origin", row.origin]
                  ];

const outputData1 = (row) => [
                    ["Number of defects", row.n_defects],
                    ["Max interestitial cluster size", row.max_cluster_size_I],
                    ["% interstitials in clusters", row.in_cluster_I],
                    ["Extra Interstitial densities", row.dclustI_count],
                    ["Eigen Dimensions Variance", row.eigen_var[0] + ", " + row.eigen_var[1] + ", " + row.eigen_var[2]]
];
const outputData2 = (row) => [
                    ["Number of cluster", row.n_clusters],
                    ["Max vacancy cluster size", row.max_cluster_size_V],
                    ["% vacancy in clusters", row.in_cluster_V],
                    ["Extra Vac. densities (Subcascades)", Math.max(0, Object.keys(row.dclust_coords).length - 1)],
                    ["Second Vac. density Impact", row.dclust_sec_impact]
];

const GetTable = props => {
  return (
                <GridItem xs={6}>
                <Table>
<TableBody>
          {props.inputData(props.row).map((n, i) => {
            return (
              <TableRow key={i}>
                <CustomTableCell component="th" scope="row">
                  {n[0]}
                </CustomTableCell>
                <TableCell>{n[1]}</TableCell>
              </TableRow>
            );
          })}
</TableBody>
</Table>
</GridItem>

  );
};

export const InputInfo = props => {
  return(
                <Grid container justify="flex-end">
                <GetTable inputData = {inputData1} row = {props.row} />
                <GetTable inputData = {inputData2} row = {props.row} />
                </Grid>
   );
}

export const OutputInfo = props => {
  return(
                <Grid container justify="flex-end">
                <GetTable inputData = {outputData1} row = {props.row} />
                <GetTable inputData = {outputData2} row = {props.row} />
                </Grid>
   );
}

const getDefects = row=> {
  return row.coords.filter(d => d[5] === 1).map(d => [d[0]+"", d[1]+"", ""+d[2], (d[3] === 1) ? "interstitial" : "vacancy", ""+d[4]]); 
}

export const DefectsList = props => {
  return(
                <Grid container style={{maxHeight:"286px"}}>
                <Tabley
                  tableHeaderColor="warning"
                  tableHead={["x", "y", "z", "Type", "Cluster-ID"]}
                  tableData={getDefects(props.row)}
                />
                </Grid>
   );
}