import React from 'react';
import classnames from 'classnames';
import {
  DefectDistancePlot,
} from "./DefectDistancePlot";


class PKADistributionPlots extends React.Component {
  constructor(props, context) {
    super(props, context);
  }


  render() {
    return (
      <div>

                <DefectDistancePlot
                  data={this.props.statAngles}
                  title={"Angle"}
                  xlabel={"Degree"}
                  type={"Statistics"}
                />
      <DefectDistancePlot
        data={this.props.compareDists}
        title={"Distance"}
        xlabel={"Angstroms"}
        type={"Compare"}
      />
      <DefectDistancePlot
        data={this.props.compareAngles}
        title={"Angle"}
        xlabel={"Degree"}
        type={"Compare"}
      />
      </div>
    );
  }
}

export default PKADistributionPlots;