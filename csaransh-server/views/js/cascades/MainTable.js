import React from 'react';
import ReactTable from "react-table";
//import "react-table/react-table.css";
//import "../../css/index.css";
//import {InfoModal} from './InfoModal';
import InputRange from 'react-input-range';
import Select from 'react-select';
//import { UncontrolledTooltip } from 'reactstrap';
import Tooltip from '@material-ui/core/Tooltip';
import Button from '@material-ui/core/Button';
import IconButton from '@material-ui/core/IconButton';
import AddButton from '@material-ui/icons/AddCircle';
import MinusButton from '@material-ui/icons/RemoveCircle';
import Visibility from '@material-ui/icons/Visibility';
import HighlightOff from '@material-ui/icons/HighlightOff';

import Gpsoff from '@material-ui/icons/GpsOff';
import Gpsfix from '@material-ui/icons/GpsFixed';

/*
const Match = Object.freeze({
  success:   Symbol("Success"),
  fail:  Symbol("Fail"),
  error: Symbol("Error")
});

const approxMatch = (val1, val2, defaultRel = '=') => {
  if (val1 === "") return Match.success;
  const lt = (v1, v2) => (v2 < v1) ? Match.success : Match.fail;
  const gt = (v1, v2) => (v2 > v1) ? Match.success : Match.fail;
  const eq = (v1, v2) => (v2 == v1) ? Match.success : Match.fail;
  const near = (v1, v2, thresh = 10) => Math.abs(v1 - v2) <= thresh ? Match.success : Match.fail;

  let fn = eq;
  const isRel = ['<', '>', '=', '~'].includes(val1[0]);
  const choice = isRel ? val1[0] : defaultRel;
  switch (choice) {
    case '<' : fn = lt; break;
    case '>' : fn = gt; break;
    case '=' : fn = eq; break;
    case '~' : fn = near; break;
    default : fn = eq; break;
  }
  let x = (isRel) ? parseFloat(val1.substring(1)) : parseFloat(val1);
  if (isNaN(x)) return Match.error;
  return fn(x, val2);
};
*/

class ActionButton extends React.Component {
  constructor(props) {
    super(props);
  }
  render() {
    const isLook = this.props.look === this.props.cellInfo.name;
    const isSelected = this.props.selected.has(this.props.cellInfo.name);
    const isExcept = this.props.except.has(this.props.cellInfo.name);
    const lookButColor = isLook ? 'primary' : 'default';
    const viewButColor = isSelected ? 'primary' : 'default';
    const exButColor = isExcept ? 'secondary' : 'default';

    return (
     <div>

      <Tooltip title="View this cascade" placement="left" id="tooltipLook">
      <IconButton className="tableButton" size="small" color={lookButColor} onClick={() => {
        //const isSelected = this.props.selected.has(this.props.cellInfo.name);
        //let x = !isSelected;
        return this.props.onLookCur(this.props.cellInfo);
      }
      }
        >
        <Visibility/>
      </IconButton>
      </Tooltip>
 
    <Tooltip id="tooltipExcept" title="Remove from statistics" placement="top">
      <IconButton className="tableButton" size="small" color={exButColor} onClick={() => {
        return this.props.onBan(this.props.cellInfo, isExcept);
      }
      }>
        {(isExcept) ? <Gpsoff /> : <Gpsfix/>}
      </IconButton>
      </Tooltip>
  
     </div>
    );
  }
}
/*

      <InfoModal row = {this.props.cellInfo}/>

        <div>
              <Typography className={classes.instructions}>{getStepContent(activeStep)}</Typography>
              <div>
                <Button
                  disabled={activeStep === 0}
                  onClick={this.handleBack}
                  className={classes.button}
                >
                  Back
                </Button>
                <Button
                  variant="contained"
                  color="primary"
                  onClick={this.handleNext}
                  className={classes.button}
                >
                  Next
                </Button>
              </div>
        </div>
 

  <InputRange
    minValue={1}
    maxValue={this.totalCmp}
    value = {this.curShow}
    onChange={val => { return this.onShow(val); }}
    onChangeComplete={val => {
      return this.onShowComplete(val);
    }}
  />
 

      */

const RangeFilter = props => {
  let clsName = (props.isFilter) ? 'filter-yes' : 'filter-no';
  return (<span>
    <HighlightOff
      onClick={() => { return props.onChange(props.minMax); }}
      className={`table-btn ${clsName}`}
    />
  <span>
  <InputRange
    minValue={props.minMax.min}
    maxValue={props.minMax.max}
    value = {props.vfilter}
    onChange={val => {
      if (val.min < props.minMax.min) val.min = props.minMax.min;
      if (val.max >= props.minMax.max) val.max = props.minMax.max;
      return props.onChange(val);
    }}
    onChangeComplete={val => {
      return props.onChangeComplete();
    }}
  />
  </span>
  </span>
  );
};

const defaultRangeFilterFn = (filter, row) => row[filter.id] >= filter.value.min && row[filter.id] < filter.value.max;

const minMaxSubc = (ar) => {
  let min = 0, max = 0;
  if (ar.length > 0) {
    const val = ar[0].density_cluster_vac.length;
    min = parseInt(val) == 0 ? 0 : parseInt(val) - 1;
    max = min;
  }
  for (const x of ar) {
    const val = Math.max(0, parseInt(x.density_cluster_vac.length) - 1);
    min = Math.min(min, val);
    max = Math.max(max, val);
  }
  return {min:Math.floor(min), max:Math.ceil(max + 0.01)};
}; 

const minMaxOned = (ar) => {
  let min = 0, max = 0;
  if (ar.length > 0) {
    min = parseFloat(ar[0]["eigen_var"][0]) * 100;
    max = min;
  }
  for (const x of ar) {
    const val = parseFloat(x["eigen_var"][0]) * 100;
    min = Math.min(min, val);
    max = Math.max(max, val);
  }
  return {min:Math.floor(min), max:Math.ceil(max + 0.01)};
}; 

const minMaxTwod = (ar) => {
  let min = 0, max = 0;
  if (ar.length > 0) {
    min = parseInt((parseFloat(ar[0]["eigen_var"][0]) + parseFloat(ar[0]["eigen_var"][1])) * 100);
    max = min;
  }
  for (const x of ar) {
    const val = parseInt((parseFloat(x["eigen_var"][0]) + parseFloat(x["eigen_var"][1])) * 100);
    min = Math.min(min, val);
    max = Math.max(max, val);
  }
  return {min:Math.floor(min), max:Math.ceil(max + 0.01)};
};

const minMaxProb = (ar) => {
  let min = 0, max = 0;
  if (ar.length > 0) {
    const val = ar[0].density_cluster_vac.length <= 1 ? 0 : parseInt(ar[0].clusters[ar[0].density_cluster_vac[1]].length * 100 / ar[0].clusters[ar[0].density_cluster_vac[0]].length);
    min = val;
    max = min;
  }
  for (const x of ar) {
    const val = x.density_cluster_vac.length <= 1 ? 0 : parseInt(x.clusters[x.density_cluster_vac[1]].length * 100 / x.clusters[x.density_cluster_vac[0]].length);
    min = Math.min(min, val);
    max = Math.max(max, val);
  }
  return {min:Math.floor(min), max:Math.ceil(max + 0.01)};
};

const minMaxProps = (ar, name) => {
  if (name === "oned") {
    return minMaxOned(ar);
  } else if (name === "twod") {
    return minMaxTwod(ar);
  } else if (name === "subc") {
    return minMaxSubc(ar);
  } else if (name === "prob") {
    return minMaxProb(ar);
  }
  let min = 0, max = 0;
  if (ar.length > 0) {
    const val = parseFloat(ar[0][name]);
    min = val;
    max = val;
  }
  for (const x of ar) {
    const val = parseFloat(x[name]);
    min = Math.min(min, val);
    max = Math.max(max, val);
  }
  return {min:Math.floor(min), max:Math.ceil(max + 0.01)};
}; 


const uniqueAr = (ar, key) => {
  let s = new Set();
  for (const x of ar) {
    s.add(x[key]);
  }
  const resAr = [...s]; 
  let res = [];
  for (const x of resAr) {
    res.push({label:x, value:x});
  }
  return res;
}
/*
const uniqueAr = (ar, key) => {
  let s = new Set();
  for (const x of ar) {
    s.add(x[key]);
  }
  const resAr = [...s]; 
  let res = [];
  for (const x of resAr) {
    res.push({label:x, value:x});
  }
  return res;
}
*/


export class MainTable extends React.Component {
  constructor(props) {
    super(props);
    this.rows = this.props.data;
    this.filters = this.defaultFilterBounds();
    this.due = "";
    this.state = {
      vfilters : this.defaultFilterBounds(),
      isFilter : this.defaultIsFilter(),
      substrate : uniqueAr(this.props.data, "substrate"),
      energy : uniqueAr(this.props.data, "energy"),
      filtered : []
    };
  }

  defaultFilterBounds() {
    return {
      n_defects : minMaxProps(this.props.data, "n_defects"),
      max_cluster_size :  minMaxProps(this.props.data, "max_cluster_size"),
      in_cluster : minMaxProps(this.props.data, "in_cluster"),//minMaxClusterCent(this.props.data),
      rectheta : minMaxProps(this.props.data, "rectheta"),
      recphi : minMaxProps(this.props.data, "recphi"),
      oned : minMaxProps(this.props.data, "oned"),
      twod : minMaxProps(this.props.data, "twod"),
      subc : minMaxProps(this.props.data, "subc"),
      prob : minMaxProps(this.props.data, "prob")
    };
  }

  defaultIsFilter() {
    return {
      n_defects : false,
      max_cluster_size : false,
      in_cluster : false,
      rectheta : false,
      recphi : false,
      oned : false,
      twod : false,
      substrate : false,
      energy : false,
      prob : false,
      subc : false
    };
  }

  finalFilters() {
   let vfilters = JSON.parse(JSON.stringify(this.state.vfilters));
    for (const key in vfilters) {
      vfilters[key] = minMaxProps(this.rows, key);
    }
    this.setState({ vfilters });
  }

  banHandler(cellInfo, isExcept) {
    if (isExcept) {
      this.rows.push(cellInfo);
    } else {
      for (const i in this.rows) {
        if (cellInfo.name === this.rows[i].name) {
          this.rows.splice(i, 1);
        }
      }
    }
    this.updateFilters(this.state.filtered, "");
    this.props.onExceptCur(cellInfo);
  }

  updateFilters (filtered, curFilter) {
    let vfilters = {};
    let isFilter = this.defaultIsFilter();
    for (const x of filtered) {
      if (x.id === "substrate" || x.id === "energy") {
        if (x.value.length > 0) {
          isFilter[x.id] = true;
        }
      } else {
        if (x.value.min > this.filters[x.id].min || x.value.max < this.filters[x.id].max) {
          isFilter[x.id] = true;
        }
        if (x.id === curFilter) {
          if (isFilter[x.id]) vfilters[x.id] = x.value;
          else {
            vfilters[x.id] = minMaxProps(this.rows, x.id);
          }
        }
      }
    }
    let energy = isFilter.energy ? this.state.energy : uniqueAr(this.rows, "energy");
    let substrate = isFilter.substrate ? this.state.substrate : uniqueAr(this.rows, "substrate");
    for (const key in isFilter) {
      if (key === "substrate" || key === "energy") continue;
      if (key !== curFilter) {
        vfilters[key] = minMaxProps(this.rows, key);
      }
    }
    //this.setState({vfilters, filtered, isFilter, substrate, energy}, () => this.props.setRows(this.rows));
    this.props.setRows(this.rows);
    this.setState({vfilters, filtered, isFilter, substrate, energy});
  }
 
  defaultRangeFilterAllFn() {
    return ((filter, rows) => {
      const ans = rows.filter(row => defaultRangeFilterFn(filter, row));
      //this.rows = ans;
      return ans;
    })
  }

  filterMethosSelect() {
    return ((filter, rows) => {
      if (filter.value.length === 0) {
        //this.rows = rows;
        return rows;
      }
      const ans = rows.filter(row => {
        for (const x of filter.value) {
          if (x.value === row[filter.id]) return true;
        }
        return false;
      });
      //this.rows = ans;
      return ans;
    });
  }

  filterSelect(id) {
    return (({ filter, onChange }) => {
                    return (
                      <Select
                        name={id}
                        value={filter ? filter.value : ""}
                        closeOnSelect={false}
                        multi
                        onChange={val => {
                          return onChange(val);
                        }}
                        options={this.state[id]}
                      />);
                    });
  }

  render() {
    return (
      <div style={{width:"100%"}}>
        <ReactTable
          data={this.props.data}
          filterable
          defaultFilterMethod={(filter, row) =>
            String(row[filter.id]) === filter.value}
          columns={[
            {
              Header: "Actions",
              id: "actions",
              accessor: cellInfo => [this.props.look === cellInfo.name, this.props.except.has(cellInfo.name), !this.props.selected.has(cellInfo.name), cellInfo],//cellInfo,
              Cell: props => <ActionButton cellInfo={props.value[3]} except = {this.props.except} selected={this.props.selected} look={this.props.look} onBan={(info, is) => this.banHandler(info, is)} onLookCur={this.props.onLookCur} onViewCur={this.props.onViewCur}/>, 
              filterable: false,
              sortable: true
            },
            {
              Header: "Inputs",
              columns: [
                {
                  Header: "Material",
                  accessor: "substrate",
                  id: "substrate",
                  filterAll: true,
                  filterMethod: this.filterMethosSelect(),
                  Filter: this.filterSelect("substrate")
                },
                {
                  Header: "Energy (keV)",
                  accessor: "energy",
                  id: "energy",
                  filterAll: true,
                  filterMethod: this.filterMethosSelect(),
                  Filter: this.filterSelect("energy")
                },
                {
                  Header: "PKA (θ)",
                  accessor: "rectheta",
                  Filter: ({filter, onChange}) => <RangeFilter filter={filter} vfilter={this.state.vfilters.rectheta} onChangeComplete={() => this.finalFilters()}  onChange={onChange} isFilter={this.state.isFilter.rectheta} minMax={this.filters.rectheta}/>,
                  filterAll: true,
                  filterMethod: this.defaultRangeFilterAllFn()
                },
                {
                  Header: "PKA (φ)",
                  accessor: "recphi",
                  Filter: ({filter, onChange}) => <RangeFilter filter={filter} vfilter={this.state.vfilters.recphi} onChangeComplete={() => this.finalFilters()}  onChange={onChange} isFilter={this.state.isFilter.recphi} minMax={this.filters.recphi}/>,
                  filterAll: true,
                  filterMethod: this.defaultRangeFilterAllFn()
                }
              ]
            },
            {
              Header: "Output Defects Information",
              columns: [
                {
                  Header: "Defects Count",
                  id: "n_defects",
                  accessor: d => d.n_defects,
                  Filter: ({filter, onChange}) => <RangeFilter filter={filter} vfilter={this.state.vfilters.n_defects} onChangeComplete={() => this.finalFilters()}  onChange={onChange} isFilter={this.state.isFilter.n_defects} minMax={this.filters.n_defects}/>,
                  filterAll: true,
                  filterMethod: this.defaultRangeFilterAllFn()
                },
                {
                  Header: "Max cluster size",
                  id: "max_cluster_size",
                  accessor: d => d.max_cluster_size,
                  Filter: ({filter, onChange}) => <RangeFilter filter={filter} vfilter={this.state.vfilters.max_cluster_size} onChangeComplete={() => this.finalFilters()}  onChange={onChange} isFilter={this.state.isFilter.max_cluster_size} minMax={this.filters.max_cluster_size}/>,
                  filterAll: true,
                  filterMethod: this.defaultRangeFilterAllFn()
                },
                {
                  Header: "% Defects in Cluster",
                  id: "in_cluster",
                  accessor: d => d.in_cluster,
                  Filter: ({filter, onChange}) => <RangeFilter filter={filter} vfilter={this.state.vfilters.in_cluster} onChangeComplete={() => this.finalFilters()} onChange={onChange} isFilter={this.state.isFilter.in_cluster} minMax={this.filters.in_cluster}/>,
                  filterAll: true,
                  filterMethod: this.defaultRangeFilterAllFn()
                },
                {
                  Header: "1D-variance",
                  id: "oned",
                  accessor: d => Math.floor(d.eigen_var[0] * 100),
                  Filter: ({filter, onChange}) => <RangeFilter filter={filter} vfilter={this.state.vfilters.oned} onChangeComplete={() => this.finalFilters()} onChange={onChange} isFilter={this.state.isFilter.oned} minMax={this.filters.oned}/>,
                  filterAll: true,
                  filterMethod: this.defaultRangeFilterAllFn()
                }, {
                  Header: "2D-variance",
                  id: "twod",
                  accessor: d => Math.floor((d.eigen_var[0] + d.eigen_var[1]) * 100)/* + d.eigen_var[1]*/,
                  Filter: ({filter, onChange}) => <RangeFilter filter={filter} vfilter={this.state.vfilters.twod} onChangeComplete={() => this.finalFilters()} onChange={onChange} isFilter={this.state.isFilter.twod} minMax={this.filters.twod}/>,
                  filterAll: true,
                  filterMethod: this.defaultRangeFilterAllFn()
                }, {
                  Header: "Subcasde density regions",
                  id: "subc",
                  accessor: d => d.density_cluster_vac.length == 0 ? 0 : d.density_cluster_vac.length - 1,
                  Filter: ({filter, onChange}) => <RangeFilter filter={filter} vfilter={this.state.vfilters.subc} onChangeComplete={() => this.finalFilters()} onChange={onChange} isFilter={this.state.isFilter.subc} minMax={this.filters.subc}/>,
                  filterAll: true,
                  filterMethod: this.defaultRangeFilterAllFn()
                }, {
                  Header: "Impact measure of second big subcascade",
                  id: "prob",
                  accessor: x =>  x.density_cluster_vac.length <= 1 ? 0 : parseInt(x.clusters[x.density_cluster_vac[1]].length * 100 / x.clusters[x.density_cluster_vac[0]].length),
                  Filter: ({filter, onChange}) => <RangeFilter filter={filter} vfilter={this.state.vfilters.prob} onChangeComplete={() => this.finalFilters()} onChange={onChange} isFilter={this.state.isFilter.prob} minMax={this.filters.prob}/>,
                  filterAll: true,
                  filterMethod: this.defaultRangeFilterAllFn()
                }
              ]
            }
          ]}
          defaultPageSize={5}
          defaultSorted={[
            {
              id: "substrate",
              desc: false
            },
            {
              id: "energy",
              desc: false
            }
          ]}
          style={{
            height: "350px"
          }}
          filtered={this.state.filtered}
          onFilteredChange={(filtered, column) => {
            //console.log("onFilterChange");
            //console.log(filtered);
            return this.updateFilters(filtered, column.id);
            //return this.setState({energies: [50,100,70], filtered});
          }}
          className="-striped -highlight"
        >
        {(state, makeTable, instance) => {
          let rows = [];
          for (const x of state.sortedData) {
            if (this.props.except.has(x.actions[3].name)) continue;
            rows.push(x.actions[3])
          }
          this.rows = rows;//state.sortedData;
          return (
                <div>
                  {makeTable()}
                </div>
              )
        }} 
        </ReactTable>
      </div>
    );
  }
}