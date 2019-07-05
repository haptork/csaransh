import React from 'react';
import ReactTable from "react-table";
import InputRange from 'react-input-range';
import Select from 'react-select';
import Tooltip from '@material-ui/core/Tooltip';
import IconButton from '@material-ui/core/IconButton';
import Visibility from '@material-ui/icons/Visibility';
import HighlightOff from '@material-ui/icons/HighlightOff';

import Gpsoff from '@material-ui/icons/GpsOff';
import Gpsfix from '@material-ui/icons/GpsFixed';

class ActionButton extends React.Component {
  constructor(props) {
    super(props);
  }
  render() {
    const isLook = this.props.look === this.props.cellInfo.name;
    const isExcept = this.props.except.has(this.props.cellInfo.name);
    const lookButColor = isLook ? 'primary' : 'default';
    const exButColor = isExcept ? 'secondary' : 'default';

    return (
     <div>

      <Tooltip title="View this cascade" placement="left" id="tooltipLook">
      <IconButton className="tableButton" size="small" color={lookButColor} onClick={() => {
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

const accessorDefault = name => x => x[name];
const accessorOned = x => parseFloat(x["eigen_var"][0]) * 100;
const accessorTwod = x => (parseFloat(x["eigen_var"][0]) + parseFloat(x["eigen_var"][1])) * 100;
const accessorSubc = x => (Object.keys(x.dclust_coords).length) <= 1 ? 0 : (Object.keys(x.dclust_coords).length);

const minMaxPropsMaker = fields => (ar, name) => {
  let min = 0, max = 0;
  if (ar.length > 0) {
    const val = fields[name].accessor(ar[0]);
    min = val;
    max = val;
  }
  for (const x of ar) {
    const val = fields[name].accessor(x);
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

export class MainTable extends React.Component {
 constructor(props) {
    super(props);
    const roundOff = x => +parseFloat(x).toFixed(2);
    const defaultParse = parseInt;
    const defaultType = "output";
    const both = (f, g) => x => f(g(x));
    this.fields = {
      "n_defects":{"name":"Defects Count"},
      "max_cluster_size":{"name":"Max cluster size"}, 
      "in_cluster":{"name":"% defects in cluster"}, 
      "rectheta":{ "type": "input", "parseFn": roundOff}, 
      "recphi":{"type": "input",  "parseFn": roundOff}, 
      //"oned":{"name":"1st dim var", "accessor":accessorOned, "parseFn":parseInt}, 
      "twod":{"name":"Planarity", "accessor":accessorTwod, "parseFn":parseInt}, 
      "subc":{"name":"Subcascades", "accessor":accessorSubc, "parseFn":parseInt}, 
      "dclust_sec_impact":{"name":"Impact of 2nd big subcascade"},
      "hull_vol":{"name":"hVol"}, 
      //"hull_area":{"name":"hArea"}, 
      //"hull_nvertices":{"name":"hNVertex"}, 
      "hull_nsimplices":{"name":"hNSimplix"}
      //"hull_density":{"name":"hDensity"}
    };
    for (const key in this.fields) {
      let x = this.fields[key];
      if (!x.hasOwnProperty("type")) {
        x.type = defaultType;
      }
      if (x.hasOwnProperty("accessor")) {
        if (x.hasOwnProperty("parseFn")) {
          x['accessor'] = both(x.parseFn, x['accessor']);
        }
        continue;
      }
      if (!x.hasOwnProperty("parseFn")) x['parseFn'] = defaultParse;
      x['accessor'] = both(x.parseFn, accessorDefault(key));
    }
    this.minMaxProps = minMaxPropsMaker(this.fields);
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
    let res = {};
    for (let x in this.fields) {
      res[x] = this.minMaxProps(this.props.data, x);
    }
    return res;
  }

  defaultIsFilter() {
    let res = {};
    for (let x in this.fields) {
      res[x] = false;
    }
    return res;
  }

  finalFilters() {
   let vfilters = JSON.parse(JSON.stringify(this.state.vfilters));
    for (const key in vfilters) {
      vfilters[key] = this.minMaxProps(this.rows, key);
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
            vfilters[x.id] = this.minMaxProps(this.rows, x.id);
          }
        }
      }
    }
    let energy = isFilter.energy ? this.state.energy : uniqueAr(this.rows, "energy");
    let substrate = isFilter.substrate ? this.state.substrate : uniqueAr(this.rows, "substrate");
    for (const key in isFilter) {
      if (key === "substrate" || key === "energy") continue;
      if (key !== curFilter) {
        vfilters[key] = this.minMaxProps(this.rows, key);
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

  filterMethodSelect() {
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
    let resultCols = [];
    for (const key in this.fields) {
      if (this.fields[key].type != "output") continue;
      const field = this.fields[key];
      resultCols.push(
        {
          Header: field.name,
          id: key,
          accessor: field.accessor,
          Filter: ({filter, onChange}) => <RangeFilter filter={filter} vfilter={this.state.vfilters[key]} onChangeComplete={() => this.finalFilters()}  onChange={onChange} isFilter={this.state.isFilter[key]} minMax={this.filters[key]}/>,
          filterAll: true,
          filterMethod: this.defaultRangeFilterAllFn()
        }
      )
    }
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
              accessor: cellInfo => [this.props.look === cellInfo.name, this.props.except.has(cellInfo.name), false, cellInfo],//cellInfo,
              Cell: props => <ActionButton cellInfo={props.value[3]} except = {this.props.except} look={this.props.look} onBan={(info, is) => this.banHandler(info, is)} onLookCur={this.props.onLookCur}/>, 
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
                  filterMethod: this.filterMethodSelect(),
                  Filter: this.filterSelect("substrate")
                },
                {
                  Header: "Energy (keV)",
                  accessor: "energy",
                  id: "energy",
                  filterAll: true,
                  filterMethod: this.filterMethodSelect(),
                  Filter: this.filterSelect("energy")
                },
                {
                  Header: "PKA (θ)",
                  id: "rectheta",
                  accessor: this.fields['rectheta'].accessor,
                  Filter: ({filter, onChange}) => <RangeFilter filter={filter} vfilter={this.state.vfilters.rectheta} onChangeComplete={() => this.finalFilters()}  onChange={onChange} isFilter={this.state.isFilter.rectheta} minMax={this.filters.rectheta}/>,
                  filterAll: true,
                  filterMethod: this.defaultRangeFilterAllFn()
                },
                {
                  Header: "PKA (φ)",
                  id: "recphi",
                  accessor: this.fields['recphi'].accessor,
                  Filter: ({filter, onChange}) => <RangeFilter filter={filter} vfilter={this.state.vfilters.recphi} onChangeComplete={() => this.finalFilters()}  onChange={onChange} isFilter={this.state.isFilter.recphi} minMax={this.filters.recphi}/>,
                  filterAll: true,
                  filterMethod: this.defaultRangeFilterAllFn()
                }
              ]
            },
            {
              Header: "Output Defects Information",
              columns: resultCols
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