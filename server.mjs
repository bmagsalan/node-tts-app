import express from 'express';
import { exec } from 'child_process';
import fs from 'fs';
import path from 'path';

const app = express();
const port = 3000;

// Middleware to parse JSON bodies
app.use(express.json());

// Serve static files from the 'public' directory
app.use(express.static('public'));

// Endpoint to convert text to speech
app.post('/text-to-speech', async (req, res) => {
  try {
    const text = req.body.text;
    const outputFile = 'output.wav'; // Output audio file

    // Use espeak-ng to convert text to speech
    exec(`espeak-ng -w ${outputFile} "${text}"`, (error, stdout, stderr) => {
      if (error) {
        console.error('Error converting text to speech:', error);
        res.status(500).send('Error converting text to speech');
        return;
      }

      console.log('Audio file saved successfully!');

      // Read the audio file
      const audioData = fs.readFileSync(outputFile);

      // Set the response headers to indicate raw audio data
      res.setHeader('Content-Type', 'audio/wav');
      res.setHeader('Content-Length', audioData.length);
      res.send(audioData);
    });
  } catch (error) {
    console.error('Error converting text to speech:', error);
    res.status(500).send('Error converting text to speech');
  }
});

// Start the server
app.listen(port, () => {
  console.log(`Server is listening at http://localhost:${port}`);
});
