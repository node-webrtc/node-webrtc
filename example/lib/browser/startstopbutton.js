'use strict';

function createStartStopButton(onStart, onStop) {
  const startButton = document.createElement('button');
  startButton.innerText = 'Start';
  document.body.appendChild(startButton);

  const stopButton = document.createElement('button');
  stopButton.innerText = 'Stop';
  stopButton.disabled = true;
  document.body.appendChild(stopButton);

  startButton.addEventListener('click', async () => {
    startButton.disabled = true;
    try {
      await onStart();
      stopButton.disabled = false;
    } catch (error) {
      startButton.disabled = false;
      throw error;
    }
  });

  stopButton.addEventListener('click', async () => {
    stopButton.disabled = true;
    try {
      await onStop();
      startButton.disabled = false;
    } catch (error) {
      stopButton.disabled = false;
      throw error;
    }
  });
}

module.exports = createStartStopButton;
