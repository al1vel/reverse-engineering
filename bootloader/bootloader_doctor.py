import serial
import time
import os
import datetime
import sys


arguments = sys.argv[1:]

port = arguments[0]
path = arguments[1]

baudrate = 115200
ser = None
success = True

start_command = bytearray(b'\xFE\x43\x44\x45\x46\x47\x48\xEF')

start = datetime.datetime.now()

for i in range(5):
    print(f"Trying to open serial. try {i}.")
    try:
        ser = serial.Serial(port, baudrate=baudrate)
    except serial.SerialException as se:
        print("Serial port error:", str(se))
        ser = None
        time.sleep(1)

if ser is None:
    print("Serial didn't open. Shutting down...")
    sys.exit()

print("Serial opened.")
ser.write(start_command)
ser.close()
print("Start command sent. Serial closed.")
time.sleep(0.5)

try:
    ser = serial.Serial(port, baudrate=baudrate)
except serial.SerialException as e:
    print("Failed to reopen serial: ", e)
    sys.exit()

command_S = bytearray(b'\x7B\x53\x7C')
ser.write(command_S)
print("Command S sent.")

while True:
    line = ser.read(3).decode('utf-8')
    if line:
        if line == "{C|":
            print("Controller memory cleared.")
            break
time.sleep(0.2)

with open(path, "rb") as file:
    size = os.path.getsize(path)
    full_size = os.path.getsize(path)
    print(f"Patch file opened. Size is {full_size} bytes.")

    while size > 0:
        if size >= 64:
            command_A = bytearray(b'\x7B\x41\x7C')
            ser.write(command_A)

            while True:
                line = ser.read(3).decode('utf-8')
                if line == "{R|":
                    break
            ser.write(file.read(64))
            size -= 64

        else:
            command_G = bytearray(b'\x7B\x47\x7C')
            ser.write(command_G)

            while True:
                line = ser.read(3).decode('utf-8')
                if line:
                    if line == '{R|':
                        break

            ser.write(file.read(4))
            size -= 4

        while True:
            line = ser.read(3).decode('utf-8')
            if line:
                if line == '{W|':
                    break
                elif line == "FAILED TO WRITE":
                    success = False
                    break

    if not success:
        print("Write failure occurred. Shutting down...")
        sys.exit()

    file.close()
    command_E = bytearray(b'\x7B\x45\x7C')
    ser.write(command_E)
    finish = datetime.datetime.now()

    while True:
        line = ser.read(3).decode('utf-8')
        if line == '{F|':
            print("Flashing done successfully!")
            break

print('Work time: ' + str(finish - start))
ser.close()
