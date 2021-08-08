/*jshint esversion: 6 */
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
import matchSorter from 'match-sorter';

import { uniqueKey, getAllCol } from "../utils";

//import basename from 'path';

class ActionButton extends React.Component {
  constructor(props) {
    super(props);
  }
  render() {
    const isLook = this.props.look === uniqueKey(this.props.cellInfo);
    const isExcept = this.props.except.has(uniqueKey(this.props.cellInfo));
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
      {this.props.cellInfo.id}
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

const minMaxPropsMaker = (ar, fieldAccessor) => {
  let min = 0, max = 0;
  if (ar.length > 0) {
    const val = fieldAccessor(ar[0]);
    min = val;
    max = val;
  }
  for (const x of ar) {
    const val = fieldAccessor(x);
    min = Math.min(min, val);
    max = Math.max(max, val);
  }
  return {min:Math.floor(min), max:Math.ceil(max + 0.01)};
}; 

const uniqueAr = (ar, fieldAccessor) => {
  let s = new Set();
  for (const x of ar) {
    s.add(fieldAccessor(x));
  }
  const resAr = [...s]; 
  let res = [];
  for (const x of resAr) {
    res.push({label:x, value:x});
  }
  return res;
}

const isShowCol = (key, showCol) => {
  for (const x of showCol) {
    if (key == x.value) return true
  }
  return false;
}

export class MainTable extends React.Component {
 constructor(props) {
    super(props);
    this.fields = getAllCol();
    this.keyPos = {}
    let i = 0;
    for (const x of this.fields) {
      this.keyPos[x['value']] = i++;
    }
    this.rows = this.props.data;
    this.filters = this.defaultFilterBounds();
    this.state = {
      vfilters : this.defaultFilterBounds(),
      isFilter : this.defaultIsFilter(),
      filterSelectAr: this.defulatFilterSelectAr(),
      filtered : []
    };
  }
  
  defulatFilterSelectAr() {
    let res = {};
    for (const field of this.fields) {
      if (field.filterType != 'select') continue;
      res[field['value']] = uniqueAr(this.props.data, field['accessor'])
    }
    return res;
  }

  defaultFilterBounds() {
    let res = {};
    for (const field of this.fields) {
      if (field.filterType != 'range') continue;
      res[field['value']] = minMaxPropsMaker(this.props.data, field['accessor']);
    }
    return res;
  }

  defaultIsFilter() {
    let res = {};
    for (let field of this.fields) {
      res[field['value']] = false;
    }
    return res;
  }

  finalFilters() {
   let vfilters = JSON.parse(JSON.stringify(this.state.vfilters));
    for (const key in vfilters) {
      vfilters[key] = minMaxPropsMaker(this.rows, this.fields[this.keyPos[key]]['accessor']);
    }
    this.setState({ vfilters });
  }

  banHandler(cellInfo, isExcept) {
    if (isExcept) {
      this.rows.push(cellInfo);
    } else {
      for (const i in this.rows) {
        if (uniqueKey(cellInfo) === uniqueKey(this.rows[i])) {
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
      const filterType = this.fields[this.keyPos[x.id]].filterType;
      if (filterType == "select" || filterType == "text") {
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
            vfilters[x.id] = minMaxPropsMaker(this.rows, this.fields[this.keyPos[x.id]]['accessor']);
          }
        }
      }
    }
    let filterSelectAr = {}
    /*
    for (const field in this.fields) {
      const filterType = this.fields[this.keyPos[x.id]].filterType;
      if (filterType == "select") {
        filterSelectAr[x.id] = isFilter[x.id] ? this.state.filterSelectAr[x.id] : uniqueAr(this.rows, this.fields[this.keyPos[x.id]]['accessor']);
        if (isFilter[x.id]) {

        } else {
          console.log("what");
          console.log(filterSelectAr[x.id]);
        }
      }
    }
    */
    /*
    let energy = isFilter.energy ? this.state.energy : uniqueAr(this.rows, "energy");
    let substrate = isFilter.substrate ? this.state.substrate : uniqueAr(this.rows, "substrate");
    */
    for (const key in isFilter) {
      const filterType = this.fields[this.keyPos[key]].filterType;
      if (filterType == "select")  {
        filterSelectAr[key] = isFilter[key] ? this.state.filterSelectAr[key] : uniqueAr(this.rows, this.fields[this.keyPos[key]]['accessor']);
      }
      else if (key !== curFilter) {
        vfilters[key] = minMaxPropsMaker(this.rows,  this.fields[this.keyPos[key]]['accessor']);
      }
    }
    //this.setState({vfilters, filtered, isFilter, substrate, energy}, () => this.props.setRows(this.rows));
    this.props.setRows(this.rows);
    this.setState({vfilters, filtered, isFilter, filterSelectAr});
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

  filterMethodText() {
    return ((filter, rows) => {
      if (filter.value.length === 0) {
        //this.rows = rows;
        return rows;
      }
      //const ans = rows.filter(row => row[filter.id].indexOf(filter.value) != -1);
      //const ans = rows.filter(row => row[filter.id].indexOf(filter.value) != -1);
      return matchSorter(rows, filter.value, {"keys": [filter.id]});
      //this.rows = ans;
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
                        options={this.state.filterSelectAr[id]}
                      />);
                    });
  }

  filterRange(id) {
    return (({ filter, onChange }) => {
                    return (
                      <RangeFilter 
                        filter={filter} 
                        vfilter={this.state.vfilters[id]} 
                        onChangeComplete={() => this.finalFilters()}  
                        onChange={onChange} 
                        isFilter={this.state.isFilter[id]} 
                        minMax={this.filters[id]}
                      />
                    );
                  });
  }

  TextColumnFilter({ filter, onChange}) {
    return (
      <input
        value={filter ? filter.value : ''}
        onChange={e => {
          setFilter(e.target.value || undefined) // Set undefined to remove the filter entirely
        }}
        placeholder={`Search records...`}
      />
    )
  }

  render() {
    let resultCols = [];
    let inputCols = [];
    for (const field of this.fields) {
      const key = field.value;
      let isFilterable = true;
      let filter = undefined;
      let filterMethod = undefined;
      let filterAll = true;
      if (field.filterType == "range") {
        isFilterable = true;
        filterAll = true;
        filter = this.filterRange(key); 
        filterMethod = this.defaultRangeFilterAllFn(); 
      } else if (field.filterType == 'select') {
        isFilterable = true;
        filterAll = true;
        filter = this.filterSelect(key); 
        filterMethod = this.filterMethodSelect(); 
      } else if (field.filterType == "text") {
        filterMethod = this.filterMethodText();
      }
      const col = {
            Header: field.label,
            id: key,
            accessor: field.accessor,
            filterable: isFilterable,
            Filter: filter,
            filterAll: filterAll,
            filterMethod: filterMethod,
            show: isShowCol(key, this.props.showCol)
          };
      if (field.type == "input") {
        inputCols.push(col)
      } else {
        resultCols.push(col)
      }
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
              accessor: cellInfo => [this.props.look === uniqueKey(cellInfo), this.props.except.has(uniqueKey(cellInfo)), parseInt(cellInfo.id), false, cellInfo],//cellInfo,
              Cell: props => <ActionButton cellInfo={props.value[4]} except = {this.props.except} look={this.props.look} onBan={(info, is) => this.banHandler(info, is)} onLookCur={this.props.onLookCur}/>, 
              sortMethod: (a, b) => (a[2] > b[2]) ? 1 : -1,
              filterable: false,
              /*
              Filter: this.TextColumnFilter,
              filterMethod: (filter, value) => true,
              */
              sortable: true
            },
            {
              Header: "Inputs",
              columns: inputCols
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
            if (this.props.except.has(x.actions[4].name)) continue;
            rows.push(x.actions[4])
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