"use strict";

/*
|--------------------------------------------------------------------------
| Websocket
|--------------------------------------------------------------------------
|
| This file is used to register websocket channels and start the Ws server.
| Learn more about same in the official documentation.
| https://adonisjs.com/docs/websocket
|
| For middleware, do check `wsKernel.js` file.
|
*/

const { ioc } = require("@adonisjs/fold");

const FtpService = use("App/Services/FtpService");
const UdpService = use("App/Services/UdpService");

ioc.singleton("App/Services/FtpService", function(app) {
  return new FtpService();
});

ioc.singleton("App/Services/UdpService", function(app) {
  return new UdpService();
});

const Ws = use("Ws");
Ws.channel("measurement", "MeasurementController");
