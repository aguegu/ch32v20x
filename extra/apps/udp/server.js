import dgram from 'node:dgram';

// Define the port to listen on
const PORT = 1234;
const HOST = '127.0.0.1';

// Create the server
const server = dgram.createSocket('udp4');

// Handle incoming messages
server.on('message', (msg, rinfo) => {
  console.log(`Server received: ${msg} from ${rinfo.address}:${rinfo.port}`);

  // Send a response back to the client
  const response = 'Hello from the UDP server!';
  server.send(response, rinfo.port, rinfo.address, (err) => {
    if (err) console.error('Error sending response:', err);
    else console.log('Response sent to client');
  });
});

// Handle server listening event
server.on('listening', () => {
  const address = server.address();
  console.log(`Server listening on ${address.address}:${address.port}`);
});

// Bind the server to a specific port and host
server.bind(PORT, HOST);
