# TODO
#
# new data files
# read and write persistent data
# update html and CSV re case lower or upper
#
#

# imports
import binascii

import os
import os.path
import serial
import subprocess # allows calls to the OS with subprocess.call(['C:\\Temp\\a b c\\Notepad.exe'])
#import random

from datetime import datetime, date, time, timedelta
from time import strftime
import sqlite3

import re

# class TemperatureData is simple class to hold temperature data and process it with a median filter
class TemperatureData:
  def __init__(self, name):
    self.name = name
    self.sampleCount = 0
    self.data = []

  def update(self, value):
    self.data.append(value)
    self.sampleCount += 1

  def value(self):
    # compute and return median value
    values = len(self.data)
    trim = values//10
    self.data.sort()
    if trim >= 1:
      print("middle array length", values - 2*trim)
      middleValues = self.data[trim:-trim]
    else:
      middleValues = self.data
    return sum(middleValues)/len(middleValues)

  def dataCount(self):
    return len(self.data)

  def reset(self):
    self.sampleCount = 0
    self.data = []


# clas Serial Port is a simple class to hold serial port configuration and handle read and writes
class SerialPort:
  def __init__(self, port, speed):
    # port = '/dev/ttyACM0' RPi
    # speed  = 19200
    self.port = port
    self.speed = speed
    self.connection = None
    self.openConnection()

  def openConnection(self):
    self.connection = serial.Serial(self.port, self.speed)

  def readline(self):
    try:
      rawline = self.connection.readline()
    except:
      self.openConnection()
      rawline = ""

    try:
      line = rawline.rstrip().decode("utf-8")
    except UnicodeDecodeError as inst:
      print(type(inst))    # the exception instance
      print(inst.args)     # arguments stored in .args
      print(inst)
      print(rawline)
      line = ""

    ##print(line)
    if len(line) > 15:
      baseline = line[:-8]
      checksum = line[-8:]
    else:
      return 'X', 'failed transfer'

    # check chekcsum
    if validateCheckSum(baseline, checksum):
      tag  = baseline[0]
      data = baseline[1:]
    else:
      tag = 'X'
      data = 'unrecogmised tag'
    return tag, data


# Calculate checksum for text data
def getCheckSum(data):
    bindata = bytearray(data, 'UTF-8')
    checksum = binascii.crc32(bindata)
    #print(checksum)
    xchecksum = format(checksum, 'x')

    #print(data, bindata, checksum, xchecksum)
    
    return xchecksum.lower()

# Check if the passed checksum matches for the data
def validateCheckSum(data, checkStr):
    #print(data,":", checkStr)
    bFlag = False
    if (len(data) > 0) and (len(checkStr) > 0):
        checkSum = getCheckSum(data)
        #print(passedCheck)
        if checkSum == checkStr.lower():
            bFlag = True

#    print(checkSum, checkStr.lower(), data)
    return bFlag


def logdata(dtStamp, temp):
  # ensure there is data to process in the temperature list
  print("{:} -> {:6.2f}".format(dtStamp, temp))

# main loop
ser = SerialPort('COM5', 19200)
#ser = SerialPort('/dev/ttyACM0', 19200)

temperatureTemp = TemperatureData('temperature')


# throw away starting data
tag, data = ser.readline();
tag, data = ser.readline();

# allow the temperature list to fill
dtMarker = datetime.now().replace(microsecond=0, second=0) + timedelta(seconds = 60)

while 1 :
#  try:
    tag, data = ser.readline();
    print('serial data => ',tag, data)
    #print(tag, ":", data)
    #print(type(tag), ":", type(data))

    if tag == 'A':
        temperatureTemp.update(float(data))
    else:
        print("Unusal data")
    # finished processing data, now update logs if required

    if dtMarker < datetime.now():
      logdata(dtMarker, temperatureTemp.value())
      dtMarker += timedelta(seconds = 30) # set to next 5 minute marker
      temperatureTemp.reset()

#  except: # catch *all* exceptions
#    print('Exception: ', datetime.now())
