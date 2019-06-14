const path = require('path');
const FtpSrv = require('ftp-srv');
const Env = use('Env');

class FtpService {
  constructor() {
    console.log();

    this.ftp = new FtpSrv({
      url: `ftp://${Env.get('HOST')}:${Env.get('FTP_PORT')}`,
      pasv_url: Env.get('HOST'),
      pasv_min: 11024,
      pasv_max: 11124,
      anonymous: false,
    });

    this.ftp.on('login', ({ username, password }, resolve, reject) => {
      if (username == Env.get('FTP_USERNAME') && password == Env.get('FTP_PASSWORD')) {
        resolve({ root: path.resolve(__dirname + '/../../tmp/uploads') });
      } else {
        reject({});
      }
    });

    this.ftp.listen().then(() => {
      console.log(`FTP Server listening on ${Env.get('HOST')}:${Env.get('FTP_PORT')}`);
    });
  }
}

module.exports = FtpService;
