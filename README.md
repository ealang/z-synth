# z-synth

A minimal synth that uses ALSA. The intention is to provide a synth that can operate with low latency even on low performance devices (e.g. a raspi).

The current sound is a square wave with some filtering and an amp envelope.

Internally, the project's design allows for audio elements (e.g. generator, amplifier, filter) to be connected together in an acyclic graph to create the final sound.

## Supported Midi Params

| Control Number | Function        |
|----------------|-----------------|
| 1              | Distortion level (Mod wheel)
| 7              | Channel volume |
| 72             | Release time   |
| 73             | Attack time    |
| 74             | Sustain level  |
| 75             | Decay time     |

## Dependencies

Install ALSA dev libs. E.g. for Raspbian:

```
apt install libasound2-dev
```

RxCpp 4.x is required. Follow the instructions [here](https://github.com/ReactiveX/RxCpp) to install it. Increasing system virtual memory may be required when compiling RxCpp on a raspi.

## Build

```
make
```

## Example usage

```
# See audio output devices with `aplay -l`
./z-synth --device hw:0,0 --period 10 --buffer 20

# See midi input devices with `aconnect -i`
aconnect 16:0 z-synth:0
```

## Running tests

Google Test 1.x is required. Follow the instructions [here](https://github.com/google/googletest/blob/master/googletest/README.md) to install it.

```
make z-synth-test && ./z-synth-test
```
