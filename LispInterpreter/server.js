var express = require("express");
var app = express();
var expressWs = require("express-ws");
expressWs(app);
var os = require("os");
var pty = require("node-pty");

var terminals = {},
  logs = {};

app.use("/build", express.static(__dirname + "/../build"));

app.get("/", function(req, res) {
  res.sendFile(__dirname + "/index.html");
});

app.get("/style.css", function(req, res) {
  res.sendFile(__dirname + "/style.css");
});

app.get("/dist/client_bundle.js", function(req, res) {
  res.sendFile(__dirname + "/dist/client-bundle.js");
});

app.post("/terminals", function(req, res) {
  var cols = parseInt(req.query.cols),
    rows = parseInt(req.query.rows),
    term = pty.spawn(process.platform === "win32" ? "cmd.exe" : "bash", [], {
      name: "xterm-color",
      cols: cols || 80,
      rows: rows || 24,
      cwd: process.env.PWD,
      env: process.env
    });

  console.log("Created terminal with PID: " + term.pid);
  terminals[term.pid] = term;
  logs[term.pid] = "";
  term.on("data", function(data) {
    logs[term.pid] += data;
  });
  res.send(term.pid.toString());
  res.end();
});

app.post("/terminals/:pid/size", function(req, res) {
  var pid = parseInt(req.params.pid),
    cols = parseInt(req.query.cols),
    rows = parseInt(req.query.rows),
    term = terminals[pid];

  term.resize(cols, rows);
  console.log(
    "Resized terminal " + pid + " to " + cols + " cols and " + rows + " rows."
  );
  res.end();
});

app.ws("/terminals/:pid", function(ws, req) {
  var term = terminals[parseInt(req.params.pid)];
  console.log("Connected to terminal " + term.pid);
  ws.send(logs[term.pid]);

  function buffer(socket, timeout) {
    let s = "";
    let sender = null;
    return data => {
      s += data;
      if (!sender) {
        sender = setTimeout(() => {
          socket.send(s);
          s = "";
          sender = null;
        }, timeout);
      }
    };
  }
  const send = buffer(ws, 5);

  term.on("data", function(data) {
    try {
      send(data);
    } catch (ex) {
      // Websocket not open, ignore
    }
  });
  ws.on("message", function(msg) {
    term.write(msg);
  });
  ws.on("close", function() {
    term.kill();
    console.log("Closed terminal " + term.pid);
    delete terminals[term.pid];
    delete logs[term.pid];
  });
});

var port = process.env.PORT || 8000,
  host = os.platform() === "win32" ? "127.0.0.1" : "0.0.0.0";

console.log("App listening to http://" + host + ":" + port);
app.listen(port, host);
