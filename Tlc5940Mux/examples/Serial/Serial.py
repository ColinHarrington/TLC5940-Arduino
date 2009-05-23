#!/usr/bin/env python
# Serial communications with the Tlc5940Mux library's Serial Example.

import serial
import time
import Image
from tlcmux import TlcMux

BLEND_STEPS = 5

def main():
    im = Image.open('ArduinoRocksMySocks.png')
    width, height = im.size

    ser = serial.Serial('/dev/ttyUSB0', baudrate=500000)
    print('Serial Port: ' + ser.portstr)
    time.sleep(5)
    tlc = TlcMux(ser)
    tlc.clear()
    if tlc.NUM_ROWS != height:
        print('Warning: img height (%d) doesn\'t match NUM_ROWS (%d)' % (
                height, tlc.NUM_ROWS))
    numChannels = tlc.NUM_TLCS * 16
    tlcData = [None] * tlc.NUM_ROWS
    arrayData = [0] * (tlc.NUM_TLCS * 24 * tlc.NUM_ROWS)
    for i in range(tlc.NUM_ROWS):
        tlcData[i] = [0] * (numChannels)
    
    imgCol = 0
    blend = 0
    while True:
        #t1 = time.time()
        blendPct = (float(BLEND_STEPS - blend) / BLEND_STEPS) ** 2
        blendPctNext = (1.0 - blendPct) ** 2
        #print('%0.3f, %0.3f' % (blendPct, blendPctNext))
        for row in range(tlc.NUM_ROWS):
            for di in range(11):
                px = im.getpixel(((imgCol + di) % width, row))
                nextpx = im.getpixel(((imgCol + di + 1) % width, row))
                tlcData[row][10 - di] = int(round(
                        ((px[0] * blendPct + nextpx[0] * blendPctNext)
                        * 4095.0) / 255.0))
                tlcData[row][10 - di + 16] = int(round(
                        ((px[1] * blendPct + nextpx[1] * blendPctNext)
                        * 4095.0) / 255.0))
                tlcData[row][10 - di + 32] = int(round(
                        ((px[2] * blendPct + nextpx[2] * blendPctNext)
                        * 4095.0) / 255.0))
            arrayDataI = row * tlc.NUM_TLCS * 24
            for di in range(numChannels - 1, 0, -2):
                last = tlcData[row][di]
                next = tlcData[row][di - 1]
                arrayData[arrayDataI] = (last >> 4) & 0xff
                arrayData[arrayDataI + 1] = ((last << 4) | (next >> 8)) & 0xff
                arrayData[arrayDataI + 2] = (next & 0xff) & 0xff
                arrayDataI += 3
        #t2 = time.time()
        tlc.modifyArray(0, arrayData)
        #t3 = time.time()
        #print('data stuff took %0.3f ms' % ((t2 - t1) * 1000.0))
        #print('modifyArray took %0.3f ms' % ((t3 - t2) * 1000.0))
        blend += 1
        if blend == BLEND_STEPS:
            blend = 0
            imgCol += 1
            if imgCol == width:
                imgCol = 0
  
    
if __name__ == '__main__':
    main()
