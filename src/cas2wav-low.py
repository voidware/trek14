#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Copyright 2015 Benoît Canet <benoit.canet@irqsave.net>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
Output a TRS-80 cas file into a wav file using the low speed modulation format.

Documentation of the format was found in "La pratique du TRS-80 Volume III"
from "Edition du PSI" By Pierre Giraud and Alain Pinaud

Chronogram:

Synchro bit n                Data bit n                      Synchro bit n + 1

   t1                             t1                             t1
   --                             --                             --
  |  |                           |  |                           |  |
  |  |                           |  |                           |  |
--   |   ------------------------   |   ------------------------   |   ------
     |  |                           |  |                           |  |
     |  |                           |  |                           |  |
      --                             --                             --
      t2              t3             t2        t3                   t2

t1 = t2 = 125 us
t3 = 750 us

bytes most signifiant bits transfered first """

from optparse import OptionParser
import struct
import os

PCM_FORMAT = 1
CHANNELS = 1
FREQUENCY = 44100
BITS_PER_SAMPLE = 8
BYTES_PER_BLOCK = (BITS_PER_SAMPLE / 8) * CHANNELS
BYTES_PER_SEC = FREQUENCY * BYTES_PER_BLOCK

HZ = 1000                # one bit every 1 ms
PERIOD = FREQUENCY / HZ
HALF_STROBE = PERIOD / 8 # pulse are 125 us long
SILENCE = PERIOD - (HALF_STROBE * 2)

def output_wav_headers(out, size):
    """ Output the WAV file header """
    size = size * 8 * PERIOD * 2
    out.write(struct.pack('<4sI4s', "RIFF", size + 36, 'WAVE'))
    out.write(struct.pack('<4sI', 'fmt ', 0x10))
    out.write(struct.pack('<HHIIHH', PCM_FORMAT,
                          CHANNELS,
                          FREQUENCY,
                          BYTES_PER_SEC,
                          BYTES_PER_BLOCK,
                          BITS_PER_SAMPLE))
    out.write(struct.pack('<4sI', 'data', size))

def bit_pulse(out, bit):
    """ Output a 1 ms bit pulse """
    if bit:
        out.write(chr(0xfe) * HALF_STROBE)
        out.write(chr(0x00) * HALF_STROBE)
        out.write(chr(0x7f) * SILENCE)
    else:
        out.write(chr(0x7f) * PERIOD)

def sync_pulse(out):
    """ Output 1 bit pulse for synchronization """
    bit_pulse(out, 1)

def output_byte(out, byte):
    """ Output a byte """
    for _ in range(8):
        bit = (byte >> 7) & 0x1
        sync_pulse(out)     # sync bit
        bit_pulse(out, bit) # data bit
        byte <<= 1

def convert_to_wav(out, cass):
    """ Output the byte stream in cass to a wav data chunk in out """
    char = cass.read(1)
    while char != "":
        output_byte(out, ord(char))
        char = cass.read(1)

def main():
    """ Main """
    usage = "usage: %prog -o file.wav file.cass"
    parser = OptionParser(usage=usage)
    parser.add_option('-o', dest='wav', help='Output WAV file name',
                      metavar='CASS', default='/dev/stdout')
    (options, args) = parser.parse_args()

    if len(args) != 1:
        exit("Missing input .cass file.")

    cass = open(args[0], "rb")
    out = open(options.wav, "wb")

    output_wav_headers(out, os.path.getsize(args[0]))

    convert_to_wav(out, cass)

    cass.close()
    out.flush()
    out.close()

if __name__ == "__main__":
    main()
