#!/usr/bin/python
#
# Copyright 2015 Joseph Landry

import struct

import hexdump

def WaveForm(chunk_data):
    i = 4
    ckid = struct.unpack("<I", chunk_data[i:i+4])[0]
    cksize = struct.unpack("<I", chunk_data[i+4:i+8])[0]
    i += 8
    ckdata = chunk_data[i:i+cksize]
    i += cksize

    if ckid == 0x20746d66: # 'fmt '
        formatTag = struct.unpack("<H", ckdata[0:2])[0]
        nChannels = struct.unpack("<H", ckdata[2:4])[0]
        nSamplesPerSecond = struct.unpack("<I", ckdata[4:8])[0]
        avgBytesPerSecond = struct.unpack("<I", ckdata[8:12])[0]
        alignToSize = struct.unpack("<H", ckdata[12:14])[0]
        cbSize = struct.unpack("<H", ckdata[14:16])[0]

        ckid = struct.unpack("<I", chunk_data[i:i+4])[0]
        cksize = struct.unpack("<I", chunk_data[i+4:i+8])[0]
        i += 8
        ckdata = chunk_data[i:i+cksize]
        print 'FormatTag: 0x%04x, %d' % (formatTag, formatTag)
        print 'nChannels: 0x%04x, %d' % (nChannels, nChannels)
        print 'SamplesPS: 0x%08x, %d' % (nSamplesPerSecond, nSamplesPerSecond)
        print 'BytesPerS: 0x%08x, %d' % (avgBytesPerSecond, avgBytesPerSecond)
        print 'AlignTo  : 0x%04x, %d' % (alignToSize, alignToSize)
        print 'cbSize   : 0x%04x, %d' % (cbSize, cbSize)
        print 'fmt data size: ', len(ckdata)
    else:
        print 'fmt chunk required but missing!'
        return

    if ckid == 0x61746164:
        #hexdump.hexdump(ckdata)
        pass
    else:
        print hex(ckid)



def RiffChunk(chunk_data):
    form = chunk_data[:4]
    if form == 'WAVE':
        print 'wave form'
        WaveForm(chunk_data)
    elif form == 'PAL ':
        print 'pal form'
    elif form == 'RDIB':
        print 'device independant bitmap form'
    elif form == 'RMID':
        print 'midi form'
    elif form == 'RMMP':
        print 'movie form'
    else:
        print 'unknown RIFF form'


def Riff(filedata):
    i = 0;

    while i < len(filedata) - 8:
        ckid = struct.unpack("<I", filedata[i:i+4])[0]
        cksize = struct.unpack("<I", filedata[i+4:i+8])[0]
        i += 8
        ckdata = filedata[i:i+cksize]
        i += cksize

        if ckid == 0x46464952: # 'RIFF'
            RiffChunk(ckdata)
        else:
            print 'not riff chunk'



if __name__ == '__main__':
    with open('sine.wav', 'rb') as f:
        filedata = f.read()
    Riff(filedata)

