const path = require('path');
const HardSourceWebpackPlugin = require('hard-source-webpack-plugin');
//const ExtractTextPlugin = require('extract-text-webpack-plugin')

module.exports = {

    mode: 'production',
    //mode: 'development',
    module: {
      rules: [
        {
          test: /\.js$/, 
          exclude: /node_modules/,
          use: [
            {
              loader: 'babel-loader',
              options: {
                presets: ['env', 'react'/*, 'stage3'*/],
                plugins: ["transform-class-properties", "transform-object-rest-spread"]
              }
            }
          ],
        },
        {
        test: /\.css$/,
        use: [
          { loader: "style-loader" },
          { loader: "css-loader" }
        ]
        }
    /*
        { 
          test: /\.css$/, 
          use: ExtractTextPlugin.extract({
            //fallback: "style-loader",
            use: "css-loader"
          }) 
        }
        */
      ]
    },

resolve: {
    alias: {
      components: path.resolve(__dirname, 'views/components/'),
      assets: path.resolve(__dirname, 'views/assets/')
    }
  },
    plugins: [
    //new HardSourceWebpackPlugin()
  ],
    /*
    plugins: [
      new ExtractTextPlugin("../css/styles.css")
    ],
    */
    entry: {
      cascades: './views/js/index.js',
      //statistics: './views/js/statistics/index.js'
    },
    output: {
      filename: '[name].js',
      path: path.resolve(__dirname, 'public/js'),
      library: 'csaar'
    }
  };
