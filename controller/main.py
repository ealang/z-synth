import sys

import rtmidi


CONTROL_CHANGE = 0xB0
CONTROL_DATA_MSB = 0x06
CONTROL_DATA_LSB = 0x26
CONTROL_NRPN_MSB = 0x63
CONTROL_NRPN_LSB = 0x62


def nrpn_cmd_seq(param_high, param_low, value_high, value_low, channel=0, reset=True):
    yield CONTROL_CHANGE | channel, CONTROL_NRPN_MSB, param_high
    yield CONTROL_CHANGE | channel, CONTROL_NRPN_LSB, param_low
    yield CONTROL_CHANGE | channel, CONTROL_DATA_MSB, value_high
    yield CONTROL_CHANGE | channel, CONTROL_DATA_LSB, value_low
    if reset:
        yield CONTROL_CHANGE | channel, CONTROL_NRPN_MSB, 127
        yield CONTROL_CHANGE | channel, CONTROL_NRPN_LSB, 127


def main(val):
    midiout = rtmidi.MidiOut()

    i = None
    for i, name in enumerate(midiout.get_ports()):
        if "z-synth" in name:
            break
    else:
        raise Exception("Unable to find z_synth")

    midiout.open_port(i)
    with midiout:
        for msg in nrpn_cmd_seq(param_high=0x13, param_low=0x37, value_high=val, value_low=0):
            midiout.send_message(msg)


main(int(sys.argv[1]))
