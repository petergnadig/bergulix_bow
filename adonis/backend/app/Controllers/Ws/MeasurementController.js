'use strict';

const fs = require('fs');
const dgram = require('dgram');
const csv = require('csvtojson');
const readLastLines = require('read-last-lines');

const Env = use('Env');
const Helpers = use('Helpers');
const Measurement = use('App/Models/Measurement');

const UdpService = use('App/Services/UdpService');
const FtpService = use('App/Services/FtpService');

class MeasurementController {
  constructor({ socket, request }) {
    this.socket = socket;
    this.request = request;
  }

  async sendUdpStartMessage(samples, measurement_id) {
    const udpMessage = `commander,START,${samples},${measurement_id}`;
    const buffer = new Buffer.from(udpMessage);
    const client = dgram.createSocket('udp4');

    client.on('listening', async () => {
      await client.setBroadcast(true);
    });

    const udpPort = Env.get('UDP_PORT');
    const broadcast = Env.get('BROADCAST');
    await client.bind(udpPort + Math.floor(Math.random() * 11));

    await client.send(buffer, 0, buffer.length, udpPort, broadcast, (err, bytes) => {
      console.log(`UDP broadcast message sent: '${udpMessage}'`);
      client.close();
    });
  }

  // This method will run when we have message in the measurement websocket channel
  onMessage(wsMessage) {
    console.log(wsMessage);

    // parse measurement phase
    switch (wsMessage.phase) {
      case 'START':
        this.onMeasurementStart(wsMessage);
        break;
      case 'FTP_UPLOAD_COMPLETE':
        this.onFtpUploadComplete(wsMessage);
        break;
      case 'FTP_UPLOAD_FAILED':
        break;
    }

    this.socket.broadcastToAll('message', wsMessage);
  }

  onClose() {
    console.log('socket closed.');
  }

  onError(error) {
    console.log('socket error:', error);
  }

  async onMeasurementStart(wsMessage) {
    //create new measurement in the database
    const measurement = await Measurement.create({
      bow_id: 1,
      person_id: 1,
    });

    // populate id of the measurement
    measurement.reload();

    // send UDP broadcast message to all hw device
    await this.sendUdpStartMessage(wsMessage.samples, measurement.id);
  }

  // This method will run when the ftp transfer was success
  async onFtpUploadComplete(wsMessage) {
    const { measurement_id, sender } = wsMessage;
    const csvFilePath = `${Helpers.tmpPath('uploads')}/${sender}.csv`;
    try {
      // check last line of the csv file
      const lastLine = await readLastLines.read(csvFilePath, 1);

      // // if it's contains END, save data to database
      if (lastLine.includes('END')) {
        // convert csv format to json object
        const data = await csv().fromFile(csvFilePath);

        // remove last object from data which is the END.
        data.splice(-1, 1);

        // save all data to database
        await this.saveDataToDatabase(measurement_id, sender, data);

        // delete the file
        fs.unlinkSync(csvFilePath);
      }
    } catch (error) {
      console.log(error);
    }
  }

  // // This method will run after ftp transfer
  async saveDataToDatabase(measurement_id, sender, data) {
    const measurement = await Measurement.find(measurement_id);

    await measurement.data().createMany(data);

    // send all websocket station
    this.socket.broadcastToAll('message', {
      sender: 'commander',
      phase: `DATA_SAVED_TO_DATABASE`,
      device: sender,
      measurement_id,
      records: data.length,
    });
  }
}

module.exports = MeasurementController;
