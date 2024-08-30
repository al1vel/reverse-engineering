import time
import serial

baudrate = 115200
ser = None
success = False
port = "COM10"

header = ["NUM", "DETECTED", "TYPE", "GR 0-7", "GR 8-15", "FadeT", "FadeR", "Failure", "LEVEL"]


try:
    ser = serial.Serial(port, baudrate=baudrate)
    print("Serial connection established.")

    time.sleep(0.5)

    SCAN = bytearray(b'\xAB\x20\x00\x45\x46\x47\x48\x48\x48\x48\x48\x48\x48\x48\x48\xBA')
    DETECT = bytearray(b'\xAB\x21\x00\x45\x46\x47\x48\x48\x48\x48\x48\x48\x48\x48\x48\xBA')
    ON = bytearray(b'\xAB\x22\x51\x02\x46\x47\x48\x48\x48\x48\x48\x48\x48\x48\x48\xBA')
    OFF = bytearray(b'\xAB\x23\x50\x02\x46\x47\x48\x48\x48\x48\x48\x48\x48\x48\x48\xBA')
    ADD_TO_GROUP = bytearray(b'\xAB\x24\x02\x04\x46\x47\x48\x48\x48\x48\x48\x48\x48\x48\x48\xBA')
    REMOVE_FROM_GROUP = bytearray(b'\xAB\x25\x01\x04\x46\x47\x48\x48\x48\x48\x48\x48\x48\x48\x48\xBA')
    QUERY_GROUP = bytearray(b'\xAB\x26\x01\x00\x46\x47\x48\x48\x48\x48\x48\x48\x48\x48\x48\xBA')
    SET_DIM_LEVEL = bytearray(b'\xAB\x27\x52\x04\xfe\x47\x48\x48\x48\x48\x48\x48\x48\x48\x48\xBA')
    SET_FADE_TIME = bytearray(b'\xAB\x28\x50\x01\x06\x47\x48\x48\x48\x48\x48\x48\x48\x48\x48\xBA')
    SET_SHORT_ADDRESS = bytearray(b'\xAB\x29\x00\x04\x02\x47\x48\x48\x48\x48\x48\x48\x48\x48\x48\xBA')
    DELETE_SHORT_ADDRESS = bytearray(b'\xAB\x30\x01\x04\x02\x47\x48\x48\x48\x48\x48\x48\x48\x48\x48\xBA')
    EXTEND = bytearray(b'\xAB\x31\x00\x04\x02\x47\x48\x48\x48\x48\x48\x48\x48\x48\x48\xBA')
    GET_INFO = bytearray(b'\xAB\x32\x02\x04\x02\x47\x48\x48\x48\x48\x48\x48\x48\x48\x48\xBA')
    SET_FADE_RATE = bytearray(b'\xAB\x33\x50\x01\x05\x47\x48\x48\x48\x48\x48\x48\x48\x48\x48\xBA')

    ser.write(GET_INFO)

    while True:
        line = ser.readline().decode().strip()
        if line:
            print("Received:", line)
            if line == "info":
                break
    print("--------------------------------------------------------------------------")
    print("{:<3} | {:<8} | {:<4} | {:<6} | {:<7} | "
          "{:<5} | {:<5} | {:<7} | {:<5}".format(header[0], header[1], header[2], header[3], header[4], header[5],
                                                 header[6], header[7], header[8]))
    print("--------------------------------------------------------------------------")

    for i in range(64):
        device = []
        for j in range(16):
            device.append(ser.read())
        # print(device)
        for h in range(16):
            device[h] = str(device[h])[4:6]
        print("{:<3} | {:<8} | {:<4} | {:<6} | {:<7} | {:<5} | {:<5} |"
              " {:<7} | {:<5}".format(device[1], device[2], device[3], device[4], device[5], device[6], device[7],
                                      device[8], device[9]))
        # print(device)

    success = True


except serial.SerialException as se:
    print("Serial port error:", str(se))

except KeyboardInterrupt:
    pass

finally:
    if not success:
        print("FAILED")
