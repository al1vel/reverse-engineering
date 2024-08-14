import serial
import time
import os
import datetime
import serial.tools.list_ports as port_list

ports = list(port_list.comports())
a = ""
for p in ports:
    a = str(p)

port = a.split()[0]
print(port)

baudrate = 115200
ser = None

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
            if line == "67":
                break
    time.sleep(0.2)

    with open("blink1B.bin", "rb") as file:
        size = os.path.getsize("blink1B.bin")
        n_size = os.path.getsize("blink1B.bin")
        success = True

        while size > 0:
            if size >= 64:
                command = "A"
                ser.write(command.encode())
                print("Transmitted:", command)

                while True:
                    line = ser.readline().decode().strip()
                    if line:
                        print("Received:", line)
                        if line == '82':
                            break
                ser.write(file.read(64))
                # print(f'Packet went')
                size -= 64

            else:
                command = 'G'
                ser.write(command.encode())
                print("Transmitted:", command)

                while True:
                    line = ser.readline().decode().strip()
                    if line:
                        print("Received:", line)
                        if line == '82':
                            break

                ser.write(file.read(4))
                size -= 4

            while True:
                line = ser.readline().decode().strip()
                if line:
                    print("Received:", line)
                    if line == '87':
                        break
                    elif line == "FAILED TO WRITE":
                        success = False
                        break

        command = 'E'
        ser.write(command.encode())
        print("Transmitted:", command)
        finish = datetime.datetime.now()

        while True:
            line = ser.readline().decode().strip()
            if line:
                print("Received:", line)
            if line == '88':
                print("ALL DONE")
                break

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
