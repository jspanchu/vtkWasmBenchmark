const path = require('path')
const HtmlWebpackPlugin = require('html-webpack-plugin');
const CopyPlugin = require('copy-webpack-plugin');

const outputPath = path.join(__dirname, "./dist");

module.exports = {
  entry: path.join(__dirname, "web", "index.js"),
  output: {
    clean: true,
    path: outputPath,
    filename: "index.js",
    library: {
      type: "umd",
      name: "vtk-wasm-benchmark-web-app",
    },
  },
  module: {
    rules: [
      {
        test: /\.css$/,
        exclude: /\.module\.css$/,
        use: [
          { loader: 'style-loader' },
          { loader: 'css-loader' },
          { loader: 'postcss-loader' },
        ],
      },
      {
        test: /\.module\.css$/,
        use: [
          { loader: 'style-loader' },
          {
            loader: 'css-loader',
            options: {
              modules: {
                localIdentName: '[name]-[local]_[sha512:hash:base64:5]',
              },
            },
          },
          { loader: 'postcss-loader' },
        ],
      },
    ],
  },
  // for wasm, glue js
  plugins: [
    new HtmlWebpackPlugin({
      hash: true,
      title: 'VTK Rendering Application',
      header: 'VTK Rendering Application',
      metaDesc: 'A VTK C++ WebAssembly rendering application',
      template: './web/index.html',
      filename: 'index.html',
      inject: 'body'
    }),
    new CopyPlugin({
      patterns: [
        {
          from: path.join(
            __dirname, "build-emscripten", "vtkRenderingApplication.js"
          ),
          to: path.join(__dirname, "dist", "vtkRenderingApplication.js")
        },
        {
          from: path.join(
            __dirname, "build-emscripten", "vtkRenderingApplication.wasm"
          ),
          to: path.join(__dirname, "dist", "vtkRenderingApplication.wasm")
        }
      ],
    })
  ],
}
