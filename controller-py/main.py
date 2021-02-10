import argparse
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


def send_seq(midiout, msg_seq):
    for msg in msg_seq:
        midiout.send_message(msg)


def main(param_high, param_low, value_high):
    midiout = rtmidi.MidiOut()

    i = None
    for i, name in enumerate(midiout.get_ports()):
        if "z-synth" in name:
            break
    else:
        raise Exception("Unable to find z_synth")

    midiout.open_port(i)
    with midiout:
        send_seq(
            midiout,
            nrpn_cmd_seq(param_high, param_low, value_high, value_low=0),
        )


if __name__ == "__main__":
    parser = argparse.ArgumentParser("Send midi commands")
    parser.add_argument("param", help="Param number in hex, format: HHLL")
    parser.add_argument("value", type=int, help="Param high value in dec")
    args = parser.parse_args()

    if len(args.param) != 4:
        raise Exception("Incorrect param format")

    if args.value > 127 or args.value < 0:
        raise Exception("Incorrect value range")

    main(
        param_high=int(args.param[:2], 16),
        param_low=int(args.param[2:4], 16),
        value_high=args.value,
    )
