# featherTuner

## What it does

It tells the frequency/pitch of a guitar string.

## How it does it

Using a piezo sensor, the frequency at which the guitar headstock vibrates is turned into a voltage, which the arduino (here a Feather esp32 S2 Reverse TFT) can read and perform the Fast Fourier Transform on. The output is the frequency in hz.

