import sys
import signal

import serial
import MySQLdb
import time
import threading


wind_speed=0
illuminance=0
vrms=0
irms=0
power=0

# press ctrl+c to stop this handler
def quit(signum, frame):
    print("ctrl+c pressed")
    sys.exit()

def read_meter_task():
    global wind_speed
    global illuminance
    global vrms
    global irms
    global power

    # this will have to be changed to the serial port you are using
    device = 'COM4'  
    
    try:
        print("Trying...", device)
        arduino = serial.Serial(device, 9600)
    except:
        print("Failed to connect on", device)

    while 1:
        try:
        # read the data from the arduino
            data = arduino.readline()  
            print(data.decode("utf-8"))
            
            array = data.decode("utf-8").split(",")
            if array[0] == "wind speed":
                wind_speed = int(array[1])
            elif array[0] == "illuminance":
                illuminance = int(array[1])
            elif array[0] == "meter":
                vrms = float(array[1])
                irms = float(array[2])
                power = float(array[3])
        except:
            print("value is error")

def update_sql(timer):
    global wind_speed
    global illuminance
    global meter

    dbConn = MySQLdb.connect("localhost", "root", "00000000", "123") or die(
        "could not connect to database")
    # open a cursor to the database
    cursor = dbConn.cursor()

    while 1:
        time.sleep(timer)
        print("update data to sql")

        # Here we are going to insert the data into the Database
        try:
            cursor.execute(
                "INSERT INTO test (wind_speed, illuminance, vrms, irms, power) VALUES (%s, %s, %s, %s, %s)", (wind_speed, illuminance, vrms, irms, power))
            dbConn.commit()  

            # close the cursor
            # cursor.close()  
        except MySQLdb.IntegrityError:
            print("failed to insert data")
        # finally:
            # cursor.close() 

def main():
    signal.signal(signal.SIGINT, quit)
    signal.signal(signal.SIGTERM, quit)

    t1 = threading.Thread(target=read_meter_task)   
    t2 = threading.Thread(target=update_sql, args=(10, ))   
    t1.setDaemon(True)
    t2.setDaemon(True)
    t1.start()
    t2.start()

    while 1:
        time.sleep(1)

if __name__ == '__main__': 
    main()