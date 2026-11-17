#!/usr/bin/env python3
"""Read one FFT block from debugSer and plot its spectrum.

Install dependencies with: python -m pip install pyserial matplotlib
Run with: python plot_adcmxl1021_fft.py COM5
"""

import argparse
import sys

import matplotlib.pyplot as plt
import serial


def read_fft(port: str, baud: int, timeout: float):
    storage_mode = None
    record_index = None
    sample_rate = None
    expected_bins = None
    bins = []

    with serial.Serial(port, baudrate=baud, timeout=timeout) as ser:
        print(f"Listening on {port} at {baud} baud; reset the flight computer if needed.")
        while True:
            raw = ser.readline()
            if not raw:
                raise TimeoutError("Timed out waiting for a complete FFT block")
            line = raw.decode("ascii", errors="replace").strip()

            if line.startswith("FFT_BEGIN,"):
                fields = line.split(",")
                if len(fields) != 5:
                    print(f"Ignoring malformed header: {line}", file=sys.stderr)
                    continue
                storage_mode = fields[1]
                record_index = int(fields[2])
                sample_rate = int(fields[3])
                expected_bins = int(fields[4])
                bins = []
                label = f"flash record {record_index}" if storage_mode == "FLASH" else "RAM capture"
                print(f"Receiving {label}: {expected_bins} bins at {sample_rate} SPS")
            elif line.startswith("FFT_BIN,") and expected_bins is not None:
                fields = line.split(",")
                if len(fields) == 3:
                    bin_index, value = int(fields[1]), int(fields[2])
                    if bin_index != len(bins):
                        raise ValueError(f"Expected bin {len(bins)}, received {bin_index}")
                    bins.append(value)
            elif line == "FFT_END" and expected_bins is not None:
                if len(bins) != expected_bins:
                    raise ValueError(f"Expected {expected_bins} bins, received {len(bins)}")
                return storage_mode, record_index, sample_rate, bins
            elif line and not line.startswith("FFT_"):
                print(line)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("port", help="Serial port for debugSer, e.g. COM5")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--timeout", type=float, default=30.0,
                        help="seconds to wait without receiving a line")
    parser.add_argument("--max-frequency", type=float, default=10000.0,
                        help="plot limit in Hz (default: 10 kHz)")
    args = parser.parse_args()

    try:
        storage_mode, record_index, sample_rate, magnitudes = read_fft(args.port, args.baud, args.timeout)
    except (serial.SerialException, TimeoutError, ValueError) as exc:
        parser.error(str(exc))

    bin_width = sample_rate / 4096.0
    frequencies = [i * bin_width for i in range(len(magnitudes))]
    limit = min(args.max_frequency, sample_rate / 2.0)
    visible = [i for i, f in enumerate(frequencies) if f <= limit]

    plt.figure(figsize=(11, 5))
    plt.plot([frequencies[i] for i in visible], [magnitudes[i] for i in visible], linewidth=0.8)
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("FFT magnitude (raw unsigned register units)")
    source = f"flash record {record_index}" if storage_mode == "FLASH" else "RAM capture"
    plt.title(f"ADcmXL1021 FFT {source} — {bin_width:.2f} Hz/bin")
    plt.xlim(0, limit)
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    main()
