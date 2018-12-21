const cp = require("child_process");
const path = require("path");
const webpack = require("webpack");

cp.spawn("node", [path.resolve(__dirname, "server.js")], { stdio: "inherit" });

const clientConfig = {
  entry: path.resolve(__dirname, "client.ts"),
  devtool: "inline-source-map",
  module: {
    rules: [
      {
        test: /\.ts?$/,
        use: "ts-loader",
        exclude: /node_modules/
      }
    ]
  },
  resolve: {
    extensions: [".tsx", ".ts", ".js"]
  },
  output: {
    filename: "client-bundle.js",
    path: path.resolve(__dirname, "dist")
  },
  mode: "development",
  watch: true
};
const compiler = webpack(clientConfig);

compiler.watch(
  {
    aggregateTimeout: 300,
    poll: undefined
  },
  (err, stats) => {
    console.log(stats.toString({ colors: true }));
  }
);
