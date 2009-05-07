#!/usr/bin/env python
# Serial communications with the Tlc5940Mux library's Serial Example.

import array
import serial
import time

def main():
    ser = serial.Serial('/dev/ttyUSB0', baudrate=57600)
    print('Serial Port: ' + ser.portstr)
    time.sleep(0.1)
    tlc = TlcMux(ser)
    tlc.clear()
    tlc.set(0, 0, 4095)
    tlc.setRow(1, [2048] * (tlc.NUM_TLCS * 16))
    time.sleep(1)
    tlc.clearRow(0)
    time.sleep(1)
    tlc.setAll(1000)
    time.sleep(1)
    tlc.clearRow(1)
    tlc.setRowAll(7, 1)
    time.sleep(1)
    tlc.setRow(5, [20] * tlc.NUM_TLCS * 16)
    time.sleep(1)
    print(tlc.get(0, 0))
    print(tlc.getRow(5))
    tlc.modifyRow(4, [10] * tlc.NUM_TLCS * 24)
    time.sleep(1)
    tlc.modifyArray(0, [5] * tlc.NUM_TLCS * 24)
    ser.close()

class TlcMux:
    def __init__(self, ser):
        self.ser = ser
        info = self.getInfo()
        self.version = info['version']
        self.NUM_TLCS = info['NUM_TLCS']
        self.NUM_ROWS = info['NUM_ROWS']
        self.channelBytes = info['TLC_CHANNEL_TYPE_STR']
        print('NUM_TLCS: %d' % self.NUM_TLCS)
        print('NUM_ROWS: %d' % self.NUM_ROWS)
        print('protocol version: %s' % self.version)
        print('channel byte width: %d' % self.channelBytes)
        
    def getInfo(self):
        self.ser.write('i')
        resp = array.array('B', self.ser.read(5))
        if chr(resp[4]) != 'i':
            raise ValueError(
                    'ERROR: invalid response to info query: ' + repr(resp))
        return {'version': chr(resp[0]),
                'NUM_TLCS': resp[1],
                'NUM_ROWS': resp[2],
                'TLC_CHANNEL_TYPE_STR': int(chr(resp[3]))
               }
    
    def clear(self):
        self.ser.write('C')
        resp = self.ser.read(1)
        if resp != 'C':
            raise ValueError(
                    'ERROR: invalid response to clear: ' + repr(resp))
        
    def clearRow(self, row):
        self.ser.write('c' + chr(row))
        resp = self.ser.read(1)
        if resp != 'c':
            raise ValueError(
                    'ERROR: invalid response to clearRow: ' + repr(resp))
                    
    def set(self, row, channel, value):
        if not self.isValidRow(row):
            raise ValueError('ERROR: invalid row %d' % row)
        if not self.isValidChannel(channel):
            raise ValueError('ERROR: invalid channel %d' % channel)
        if not self.isValidValue(value):
            raise ValueError('ERROR: invalid channel value %d' % value)
        s = 's' + chr(row) + self.channelToStr(channel) + self.valueToStr(value)
        self.ser.write(s)
        resp = self.ser.read(1)
        if resp != 's':
            raise ValueError(
                    'ERROR: invalid response to set: ' + repr(resp))
    
    def setRow(self, row, values):
        if not self.isValidRow(row):
            raise ValueError('ERROR: invalid row %d' % row)
        if len(values) != self.NUM_TLCS * 16:
            raise ValueError('ERROR: wrong number of values (%d) for setRow' % (
                    len(values)))
        s = 'S' + chr(row) + ''.join([self.valueToStr(v) for v in values])
        self.ser.write(s)
        resp = self.ser.read(1)
        if resp != 'S':
            raise ValueError(
                    'ERROR: invalid response to setRow: ' + repr(resp))
    
    def setAll(self, value):
        if not self.isValidValue(value):
            raise ValueError('ERROR: invalid channel value %d' % value)
        s = 't' + self.valueToStr(value)
        self.ser.write(s)
        resp = self.ser.read(1)
        if resp != 't':
            raise ValueError(
                    'ERROR: invalid response to setAll: ' + repr(resp))
                    
    def setRowAll(self, row, value):
        if not self.isValidRow(row):
            raise ValueError('ERROR: invalid row %d' % row)
        if not self.isValidValue(value):
            raise ValueError('ERROR: invalid channel value %d' % value)
        s = 'T' + chr(row) + self.valueToStr(value)
        self.ser.write(s)
        resp = self.ser.read(1)
        if resp != 'T':
            raise ValueError(
                    'ERROR: invalid response to setRowAll: ' + repr(resp))
    
    def get(self, row, channel):
        if not self.isValidRow(row):
            raise ValueError('ERROR: invalid row %d' % row)
        if not self.isValidChannel(channel):
            raise ValueError('ERROR: invalid channel %d' % channel)
        s = 'g' + chr(row) + self.channelToStr(channel)
        self.ser.write(s)
        resp = array.array('B', self.ser.read(3))
        if chr(resp[2]) != 'g':
            raise ValueError(
                    'ERROR: invalid response to get: ' + repr(resp))
        return (resp[0] << 8) | resp[1]
        
    def getRow(self, row):
        if not self.isValidRow(row):
            raise ValueError('ERROR: invalid row %d' % row)
        s = 'G' + chr(row)
        self.ser.write(s)
        numBytes = 2 * (self.NUM_TLCS * 16) + 1
        resp = array.array('B', self.ser.read(numBytes))
        if chr(resp[numBytes - 1]) != 'G':
            raise ValueError(
                    'ERROR: invalid response to getRow: ' + repr(resp))
        result = []
        for i in range(0, numBytes - 1, 2):
            result.append((resp[i] << 8) | resp[i + 1])
        return result
        
    def modifyRow(self, row, arrayData):
        if not self.isValidRow(row):
            raise ValueError('ERROR: invalid row %d' % row)
        if len(arrayData) != self.NUM_TLCS * 24:
            raise ValueError(
                    'ERROR: wrong number of array values for modifyRow: %d' % (
                    len(arrayData)))
        s = 'm' + chr(row) + ''.join([chr(v) for v in arrayData])
        self.ser.write(s)
        resp = self.ser.read(1)
        if resp != 'm':
            raise ValueError(
                    'ERROR: invalid response to modifyRow: ' + repr(resp))
    
    def modifyArray(self, offset, arrayData):
        arrayLen = self.NUM_TLCS * 24 * self.NUM_ROWS
        arrayDataLen = len(arrayData)
        if offset < 0 or offset >= arrayLen:
            raise ValueError('ERROR: invalid offset: %d' % offset)
        if arrayDataLen == 0 or arrayDataLen + offset > arrayLen:
            raise ValueError('ERROR: invalid array data length %d' % (
                             arrayDataLen))
        s = 'M' + chr(offset >> 8) + chr(offset & 0xff) \
                + chr(arrayDataLen >> 8) + chr(arrayDataLen & 0xff) \
                + ''.join([chr(v) for v in arrayData])
        self.ser.write(s)
        resp = self.ser.read(1)
        if resp != 'M':
            raise ValueError(
                    'ERROR: invalid response to modifyArray: ' + repr(resp))
            
    
    
            
    
    def isValidRow(self, row):
        return row >= 0 and row < self.NUM_ROWS
        
    def isValidChannel(self, channel):
        return channel >= 0 and channel < self.NUM_TLCS * 16
        
    def isValidValue(self, value):
        return value >= 0 and value <= 4095
        
    def channelToStr(self, channel):
        if self.channelBytes == 1:
            return chr(channel)
        else:
            return chr(channel >> 8) + chr(channel & 0xff)
    
    def valueToStr(self, value):
        return chr(value >> 8) + chr(value & 0xff)
        
    

if __name__ == '__main__':
    main()
    
