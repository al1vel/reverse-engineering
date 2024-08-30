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
success = False

buffer = bytearray(b'\xFE\x43\x44\x45\x46\x47\x48\xEF')

try:
    start = datetime.datetime.now()
    ser = serial.Serial(port, baudrate=baudrate)
    print("Serial connection established.")

    ser.write(buffer)
    ser.close()
    time.sleep(0.5)

    ser = serial.Serial(port, baudrate=baudrate)
    command = 'S'
    # print("Transmitted: ", command)
    ser.write(command.encode())

    while True:
        line = ser.read().decode('utf-8')
        if line:
            # print("Received:", line)
            if line == "C":
                print("Memory cleared")
                break
    time.sleep(0.2)

    with open(path, "rb") as file:
        size = os.path.getsize(path)
        full_size = os.path.getsize(path)

        percent = 0
        old_percent = 0

        while size > 0:
            if size >= 64:
                command = "A"
                ser.write(command.encode())
                # print("Transmitted:", command)

                while True:
                    line = ser.read().decode('utf-8')
                    if line == "R":
                        # print("Received:", line)
                        break
                ser.write(file.read(64))
                # print(f'Packet went')
                size -= 64

            else:
                command = 'G'
                ser.write(command.encode())
                # print("Transmitted:", command)

                while True:
                    line = ser.read().decode('utf-8')
                    if line:
                        # print("Received:", line)
                        if line == 'R':
                            break

                ser.write(file.read(4))
                size -= 4

            while True:
                line = ser.read().decode('utf-8')
                if line:
                    # print("Received:", line)
                    if line == 'W':
                        break
                    elif line == "FAILED TO WRITE":
                        success = False
                        break

            percent = ((full_size - size) / full_size) * 100
            if percent != 100:
                print(f"{percent:.1f}%", end='\r')
            else:
                print(f"{percent:.1f}%")

        file.close()
        command = 'E'
        ser.write(command.encode())
        # print("Transmitted:", command)
        finish = datetime.datetime.now()

        while True:
            line = ser.read().decode('utf-8')
            # if line:
            #     print("Received:", line)
            if line == 'F':
                print("ALL DONE")
                break

    success = True
    print('Work time: ' + str(finish - start))
    ser.close()

except serial.SerialException as se:
    print("Serial port error:", str(se))

except KeyboardInterrupt:
    pass

finally:
    if not success:
        print("FAILED")
