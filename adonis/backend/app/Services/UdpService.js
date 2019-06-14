const dgram = require("dgram");
const Env = use("Env");

class UdpService {
  constructor() {
    this.server = dgram.createSocket("udp4");
    this.server.bind(Env.get("UDP_PORT"));

    // When udp server started and listening.
    const self = this;
    this.server.on("listening", function() {
      const address = self.server.address();
      console.log(`UDP server listening on ${address.address}:${address.port}`);
    });
  }

  getInstance() {
    return this.server;
  }
}

module.exports = UdpService;
