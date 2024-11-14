import dgram from 'node:dgram';

// Define the port to listen on
const PORT = 1234;
const HOST = '0.0.0.0';
// Create the server
const server = dgram.createSocket('udp4');

let cnt = 0;

// Handle incoming messages
server.on('message', (msg, rinfo) => {
  console.log(`Server received: ${msg} from ${rinfo.address}:${rinfo.port}`);

  // Send a response back to the client
  const response = `Hello ${cnt++}`;
  server.send(response, rinfo.port, rinfo.address, (err) => {
    if (err) console.error('Error sending response:', err);
    else console.log('Response sent to client');
  });
});

// Handle server listening event
server.on('listening', () => {
  const address = server.address();
  console.log(`Server listening on ${address.address}:${address.port}`);

  server.send(Buffer.from('Hello'), 1000, '192.168.0.32', (err) => {
    if (err) console.error('Error sending hello:', err);
    else console.log('hello sent to client');
  });
});

// Bind the server to a specific port and host
server.bind(PORT, HOST);
