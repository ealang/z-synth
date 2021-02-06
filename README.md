# z-synth

Wip synth.

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
