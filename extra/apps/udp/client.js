import dgram from 'dgram';
const client = dgram.createSocket('udp4');

// Define the server address and port
const SERVER_HOST = '127.0.0.1';
const SERVER_PORT = 1234;

// Message to send to the server
const message = Buffer.from('Hello from the UDP client!');

// Send the message to the server
client.send(message, SERVER_PORT, SERVER_HOST, (err) => {
  if (err) console.error('Error sending message:', err);
  else console.log('Message sent to server');
});

// Handle response from the server
client.on('message', (msg, rinfo) => {
  console.log(`Received from server: ${msg}`);
  client.close(); // Close the client after receiving the response
});
