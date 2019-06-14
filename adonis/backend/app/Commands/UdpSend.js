"use strict";

const { Command } = require("@adonisjs/ace");

const dgram = require("dgram");

const UDP_PORT = 37666;
const CMD_LENGTH = 9;

class UdpSend extends Command {
  static get signature() {
    return `
          udp:send
          { --broadcast=@value    : Broadcast IP address      }
          { --cmd=@value          : Instruction (max 10 char) }
          { --samples=@value      : Sample number             }
          { --m_id=@value         : Measurement id            }
    `;
  }

  static get description() {
    return "Send UDP broadcast message.";
  }

  async handle(args, options) {
    if (
      options.broadcast == null ||
      options.cmd == null ||
      options.samples == null ||
      options.m_id == null
    ) {
      this.warn("Missing parameters. Use adonis udp:send --help");
      return 1;
    }

    // Cut command lenght and padding
    let cmd = options.cmd.substring(0, CMD_LENGTH);
    cmd = cmd.padEnd(CMD_LENGTH, " ").toUpperCase();

    const message = `${cmd},${options.samples},${options.m_id}`;
    const buffer = new Buffer.from(message);

    var client = dgram.createSocket("udp4");

    client.on("listening", async () => {
      await client.setBroadcast(true);
    });

    await client.bind(UDP_PORT + Math.floor(Math.random() * 11));

    client.send(
      buffer,
      0,
      buffer.length,
      UDP_PORT,
      options.broadcast,
      (err, bytes) => {
        console.log(`UDP broadcast message: '${message}'`);
        client.close();
      }
    );
  }
}

module.exports = UdpSend;
