import net from 'node:net';

// Define the port to listen on
const PORT = 1234;

let cnt = 0;

// Create the server
const server = net.createServer((socket) => {
  console.log('Client connected:', socket.remoteAddress);
  socket.write('Hello\n');

  // Handle incoming data from the client
  socket.on('data', (data) => {
    console.log('Received from client:', data.toString());

    // Send a response back to the client
    setTimeout(() => {
      socket.write(`Hello, ${cnt++}`);
    }, 1000);
  });

  // Handle the client disconnecting
  socket.on('end', () => {
    console.log('Client disconnected');
  });

  // Handle errors
  socket.on('error', (err) => {
    console.error('Socket error:', err);
  });
});

// Start the server
server.listen(PORT, () => {
  console.log(`Server listening on port ${PORT}`);
});
