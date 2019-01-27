# z-synth

A minimal synth that uses ALSA. The intention is to provide a synth that can operate with low latency even on low performance devices (e.g. a raspi).

Currently only a square wave and this project is in a hacky state.

Build:

```
make
```

Example usage:

```
# See audio output devices with `aplay -l`
./z-synth --device hw:0,0 --period 10 --buffer 20

# See midi input devices with `aconnect -i`
aconnect 16:0 z-synth:0
```
