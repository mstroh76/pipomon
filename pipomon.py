#!/usr/bin/env python3

# ISKRA MT174 Smartmeter, Optical serial power consumption monitor
#
# Hardware: 
#  - Energiesensor für Smart Meter ES-IEC at /dev/ttyUSB0
#     https://de.elv.com/elv-homematic-energiesensor-fuer-smart-meter-es-iec-komplettbausatz-142148
#  - 4 digit TM1637 display
#
# need library/packages:
#  sudo apt install python3 python3-serial
#  sudo pip3 install raspberrypi-tm1637

import serial, fcntl, tm1637, time

consumptioncode="16.7.0"
encoding = 'utf-8'
tm = tm1637.TM1637(clk=23, dio=24)
tm.write([0, 0, 0, 0])
tm.show('run')

print("power monitor with led sensor head at ttyUSB0")
ser = serial.Serial(port='/dev/ttyUSB0', baudrate=300, timeout=1, bytesize=serial.SEVENBITS, parity=serial.PARITY_EVEN)
print("search for code " + consumptioncode)

while True:
    ser.write(b'\x2F\x3F\x21\x0D\x0A')
    time.sleep(0.5)
    numberOfLine = 0

    while True:
        response = ser.readline()

        pstr=response.decode(encoding)
#        pstr="1-0:16.7.0*255(0.305*kW)"
        if pstr.find(consumptioncode) != -1:
            powerstr=pstr[pstr.find("(")+1:pstr.find("*kW")]
            power = int(float(powerstr)*1000)
            print("'" + pstr + "' found string '" + powerstr + "' = " + str(power) + "W")
            tm.number(power)
#        else:
#            print(responseutf8)
        numberOfLine = numberOfLine + 1
        if (numberOfLine >= 18):
            break
ser.close()
