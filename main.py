import serial
import time
import os
import datetime

port = "COM8"
baudrate = 115200


try:
    start = datetime.datetime.now()
    ser = serial.Serial(port, baudrate=baudrate)
    print("Serial connection established.")
    print("Starting uploading firmware")

    command = 'S'
    print("Transmitted: ", command)
    ser.write(command.encode())

    while True:
        line = ser.readline().decode().strip()
        if line:
            print("Received:", line)
            if line == 'C':
                break
    time.sleep(1)

    with open("generic_boot20_pc13blink.bin", "rb") as file:
        size = os.path.getsize("generic_boot20_pc13blink.bin")
        n_size = os.path.getsize("generic_boot20_pc13blink.bin")
        success = True

        while size > 0:
            command = 'G'
            ser.write(command.encode())
            print("Transmitted:", command)

            while True:
                line = ser.readline().decode().strip()
                if line:
                    print(line)
                    if line == 'R':
                        print("Received:", line)
                        break

            ser.write(file.read(4))
            print(f'Packet {n_size - size} - {n_size - size + 4} WENT')
            size -= 4

            while True:
                line = ser.readline().decode().strip()
                if line:
                    print("Received:", line)
                    if line == 'W':
                        break
                    elif line == "FAILED TO WRITE":
                        success = False
                        break

        command = 'E'
        ser.write(command.encode())
        print("Transmitted: ", command)

        while True:
            line = ser.readline().decode().strip()
            if line:
                print(line)
            if line == 'X':
                print("Received:", line)
                print("ALL DONE")
                break

    finish = datetime.datetime.now()

    print('Время работы: ' + str(finish - start))

except serial.SerialException as se:
    print("Serial port error:", str(se))

except KeyboardInterrupt:
    pass

finally:
    if ser.is_open:
        ser.close()
        print("Serial connection closed.")
    file.close()
