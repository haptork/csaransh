import React from 'react';
import ReactTable from "react-table";
import "react-table/react-table.css";
import "../css/index.css";
import {InfoModal} from './InfoModal';
import { clusterCent, clusterSizes} from "./utils";
import InputRange from 'react-input-range';
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
const RangeFilter = props =>
  (<span>
    <span
      onClick={props.onRemove}
      className={`glyphicon glyphicon-remove-circle filter-icon`}
    />
  <span>
  <InputRange
    minValue={props.minMax.min}
    maxValue={props.minMax.max}
    value = {props.filter ? props.filter.value : {min:props.minMax.min, max:props.minMax.max}}
    onChange={val => {
      /*
      console.log(val);
      console.log(props.minMax[0]);
      console.log(props.minMax[1]);
      */
      if (val.min < props.minMax.min) val.min = props.minMax.min;
      if (val.max >= props.minMax.max) val.max = props.minMax.max;
      return props.onChange(val);
    }}
  />
  </span>
  </span>
  );
const uniqueElems = ar => {
  var s = new Set();
  for (const x of ar) {
    s.add(x.substrate);
  }
  return [...s];
}; 

const defaultRangeFilterFn = (filter, row) => row[filter.id] >= filter.value.min && row[filter.id] < filter.value.max;

const uniqueEnergies = ar => {
  var s = new Set();
  for (const x of ar) {
    s.add(x.energy);
  }
  return [...s];
}

const minMaxDefects = ar => {
  let min = 0, max = 0;
  if (ar.length > 0) {
    min = ar[0].last_frame.n_defects;
    max = ar[0].last_frame.n_defects;
  }
  for (const x of ar) {
    min = Math.min(min, x.last_frame.n_defects);
    max = Math.max(max, x.last_frame.n_defects);
  }
  return {min:min, max:max + 1};
}; 

const minMaxClusterSize = ar => {
  let min = 0, max = 0;
  if (ar.length > 0) {
    min = Math.max(...(clusterSizes(ar[0])));
    max = min;
  }
  for (const x of ar) {
    let val = Math.max(...(clusterSizes(x)));
    min = Math.min(min, val);
    max = Math.max(max, val);
  }
  return {min:min, max:max + 1};
}; 

const minMaxClusterCent = ar => {
  let min = 0, max = 0;
  if (ar.length > 0) {
    min = clusterCent(ar[0]);
    max = min;
  }
  for (const x of ar) {
    let val = clusterCent(x);
    min = Math.min(min, val);
    max = Math.max(max, val);
  }
  return {min:Math.floor(min), max:Math.ceil(max + 0.01)};
}; 

const minMaxProps = (ar, name) => {
  let min = 0, max = 0;
  if (ar.length > 0) {
    min = parseFloat(ar[0][name]);
    max = parseFloat(ar[0][name]);
  }
  for (const x of ar) {
    min = Math.min(min, parseFloat(x[name]));
    max = Math.max(max, parseFloat(x[name]));
  }
  return {min:Math.floor(min), max:Math.ceil(max + 0.01)};
}; 

const minMaxPropsInt = (ar, name) => {
  let min = 0, max = 0;
  if (ar.length > 0) {
    min = parseInt(ar[0][name]);
    max = parseInt(ar[0][name]);
  }
  for (const x of ar) {
    min = Math.min(min, parseInt(x[name]));
    max = Math.max(max, parseInt(x[name]));
  }
  return {min:min, max:max + 1};
}; 

export class MainTable extends React.Component {
  constructor(props) {
    super(props);
    this.rows = [];
    this.filtered = [];
    this.state = {
      filters : {
        substrate : uniqueElems(this.props.data),
        energy : uniqueEnergies(this.props.data),
        n_defects : minMaxDefects(this.props.data),
        max_size : minMaxClusterSize(this.props.data),
        in_cluster : minMaxClusterCent(this.props.data),
        rectheta : minMaxProps(this.props.data, "rectheta"),
        recphi : minMaxProps(this.props.data, "recphi")
      }
    };
    this.vfilters = {
        substrate : uniqueElems(this.props.data),
        energy : uniqueEnergies(this.props.data),
        n_defects : minMaxDefects(this.props.data),
        max_size : minMaxClusterSize(this.props.data),
        in_cluster : minMaxClusterCent(this.props.data),
        rectheta : minMaxProps(this.props.data, "rectheta"),
        recphi : minMaxProps(this.props.data, "recphi")
    };
  }
/*
  onChange(val, next) {
    this.setState({ elems : ["new"] });
    next(val);
  }
  */

      //this.setState({ elems : ["new"] });

  resetFilter(id) {
    return () => {
      let filtered = this.state.filtered.filter(x => x.id !== id);
      let filters = {
        substrate : this.state.filters.substrate,
        energy : this.state.filters.energy,
        n_defects : this.state.filters.n_defects,
        max_size : this.state.filters.max_size,
        in_cluster : this.state.filters.in_cluster,
        rectheta : this.state.filters.rectheta,
        recphi : this.state.filters.recphi
      }
      filters[id] = this.filters[id];
      //if (filtered.length < this.state.filtered.length) this.setState({ filtered, filters }, this.updateFilters(filtered, id)udateFilters()
    };
  };

  updateFilters (filtered, curFilter) {
    //this.sayIt();
    const filters  = [
          //{ id: "substrate" value : uniqueElems(this.rows) }
          //energy : uniqueEnergies(this.rows),
          //n_defects : (curFilter === "n_defects") ? this.filters[curFilter] : minMaxPropsInt(this.rows, "n_defects"),
          //max_size : (curFilter === "max_size") ? this.filters[curFilter] :minMaxPropsInt(this.rows, "max_size"),
          //in_cluster : (curFilter === "in_cluster") ? this.filters[curFilter] :minMaxProps(this.rows, "in_cluster"),
          {id: "rectheta", value : (curFilter === "rectheta") ? {} : minMaxProps(this.rows, "rectheta")},
          {id: "recphi", value: (curFilter === "recphi") ? {} : minMaxProps(this.rows, "recphi")}
    ];
    for (let x of filtered) {
      if (x.id === curFilter) {
        for (let y of filters) {
          if (y.id === curFilter) {
            y.value = x.value;
          }
        }
      } 
      /*
      if (x.id === "substrate" || x.id === "id" || x.id == curFilter) continue;
      filters[x.id] = this.state.filters[x.id];
      */
      //if (x.value.min < filters[x.id].min) x.value.min = filters[x.id].min;
      //if (x.value.max >= filters[x.id].max) x.value.max = filters[x.id].max;
    }
    this.setState({
      //filters,
      filtered:filters
    });
  }
 
/*
  updateFilters (filtered, curFilter) {
    //this.sayIt();
    const filters  = {
          substrate : uniqueElems(this.rows),
          energy : uniqueEnergies(this.rows),
          n_defects : (curFilter === "n_defects") ? this.filters[curFilter] : minMaxPropsInt(this.rows, "n_defects"),
          max_size : (curFilter === "max_size") ? this.filters[curFilter] :minMaxPropsInt(this.rows, "max_size"),
          in_cluster : (curFilter === "in_cluster") ? this.filters[curFilter] :minMaxProps(this.rows, "in_cluster"),
          rectheta : (curFilter === "rectheta") ? this.filters[curFilter] :minMaxProps(this.rows, "rectheta"),
          recphi : (curFilter === "recphi") ? this.filters[curFilter] :minMaxProps(this.rows, "recphi")
    };
    for (let x of filtered) {
      if (x.id === "substrate" || x.id === "id" || x.id == curFilter) continue;
      filters[x.id] = this.state.filters[x.id];
      //if (x.value.min < filters[x.id].min) x.value.min = filters[x.id].min;
      //if (x.value.max >= filters[x.id].max) x.value.max = filters[x.id].max;
    }
    this.setState({
      //filters,
      filtered
    });
  }
  */
  
  defaultRangeFilterAllFn () {
    return (filter, rows) => {
      const ans = rows.filter(row => defaultRangeFilterFn(filter, row));
      this.rows = ans;
      return ans;
    };
  }

  renderActionButtons(cellInfo) {
    return (
     <div>
      <InfoModal row = {cellInfo}/>
      <button onClick={() => this.props.onViewCur(cellInfo)}
        className={`btn btn-sm btn-primary table-btn`}
      >
        <span
         className={`glyphicon glyphicon-edit table-glyph`}
         />
      </button>
     </div>
    );
  }

  render() {
    return (
      <div>
        <ReactTable
          data={this.props.data}
          filterable
          defaultFilterMethod={(filter, row) =>
            String(row[filter.id]) === filter.value}
          columns={[
            {
              Header: "Inputs",
              columns: [
                {
                  Header: "Material",
                  accessor: "substrate",
                  filterAll: true,
                  filterMethod: (filter, rows) => {
                    //console.log("filtering: " + filter.value);
                    return rows.filter(row => filter.value == "all" || filter.value == row.substrate);
                    //return filter.value == "all" || filter.value == row.substrate;
                  },
                  Filter: ({ filter, onChange }) =>
                    <select
                      onChange={event => onChange(event.target.value)}
                        //this.setState({energies: [50,100,70]}, onChange(event.target.value));
                        //onChange(event.target.value);
                      style={{ width: "100%" }}
                      value={filter ? filter.value : "all"}
                    >
                    <option value="all">All</option>
                    {this.state.filters.substrate.map((o, i) => <option value={o} key={i}>{o} </option>)}
                    </select>
                },
                {
                  Header: "Energy (keV)",
                  accessor: "energy",
                  filterMethod: (filter, row) => filter.value == "all" || filter.value == row.energy,
                  Filter: ({ filter, onChange }) =>
                    <select
                      onChange={event => onChange(event.target.value)}
                      style={{ width: "100%" }}
                      value={filter ? filter.value : "all"}
                    >
                    <option value="all">All</option>
                    {this.state.filters.energy.map((o, i) => <option value={o} key={i}>{o} </option>)}
                    </select>
                },
                {
                  Header: "PKA (θ)",
                  accessor: "rectheta",
                  Filter: ({filter, onChange}) => <RangeFilter onRemove={this.resetFilter("rectheta")} filter={filter} onChange={onChange} minMax={this.state.filters.rectheta}/>,
                  filterAll: true,
                  filterMethod: this.defaultRangeFilterAllFn()
                },
                {
                  Header: "PKA (φ)",
                  accessor: "recphi",
                  Filter: ({filter, onChange}) => <RangeFilter filter={filter} onChange={onChange} minMax={this.state.filters.recphi}/>,
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
                  accessor: d => d.last_frame.n_defects,
                  Filter: ({filter, onChange}) => <RangeFilter filter={filter} onChange={onChange} minMax={this.state.filters.n_defects}/>,
                  filterAll: true,
                  filterMethod: this.defaultRangeFilterAllFn()
                },
                {
                  Header: "Max cluster size",
                  id: "max_size",
                  accessor: d => Math.max(...(clusterSizes(d))),
                  Filter: ({filter, onChange}) => <RangeFilter filter={filter} onChange={onChange} minMax={this.state.filters.max_size}/>,
                  filterAll: true,
                  filterMethod: this.defaultRangeFilterAllFn()
                },
                {
                  Header: "% Defects in Cluster",
                  id: "in_cluster",
                  accessor: d => clusterCent(d),
                  Filter: ({filter, onChange}) => <RangeFilter filter={filter} onChange={onChange} minMax={this.state.filters.in_cluster}/>,
                  filterAll: true,
                  filterMethod: this.defaultRangeFilterAllFn()
                }
              ]
            },
            {
              Header: "Actions",
              id: "actions",
              accessor: cellInfo => this.renderActionButtons(cellInfo),
              filterable: false
            }
          ]}
          defaultPageSize={5}
          filtered={this.state.filtered}
          onFilteredChange={(filtered, column) => {
            return this.updateFilters(filtered, column.id);
            //return this.setState({energies: [50,100,70], filtered});
          }}
          className="-striped -highlight"
        />
      </div>
    );
  }
}