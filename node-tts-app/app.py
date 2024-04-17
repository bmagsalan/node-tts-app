from flask import Flask, render_template, request, Response
from flask_socketio import SocketIO
import subprocess
import os

app = Flask(__name__)
socketio = SocketIO(app)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/text-to-speech', methods=['POST'])
def text_to_speech():
    try:
        text = request.json.get('text')
        output_file = 'output.wav'
        subprocess.run(['espeak-ng', '-w', output_file, text], check=True)
        audio_data = open(output_file, 'rb').read()
        os.remove(output_file)  # Remove the output file after reading its contents

        return Response(audio_data, mimetype='audio/wav')
    except Exception as e:
        print('Error converting text to speech:', e)
        return 'Error converting text to speech', 500

@socketio.on('convert_text_to_speech')
def convert_text_to_speech(data):
    try:
        text = data['text']
        output_file = 'output.wav'
        subprocess.run(['espeak-ng', '-w', output_file, text], check=True)
        audio_data = open(output_file, 'rb').read()
        os.remove(output_file)  # Remove the output file after reading its contents

        socketio.emit('audio_data', audio_data)
    except Exception as e:
        print('Error converting text to speech:', e)

@socketio.on('stop_speech')
def stop_speech():
    # Add logic to stop speech if needed
    print('Stop speech requested')

if __name__ == '__main__':
    socketio.run(app, debug=True)
