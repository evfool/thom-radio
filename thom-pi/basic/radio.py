import RPi.GPIO as GPIO          #Import GPIO library
import time                      #Import time library
from mpd import MPDClient, ConnectionError # import mpd-related classes

# create local mpd client
client = MPDClient()
client.timeout = 10
client.idletimeout = None
client.connect("localhost", 6600)

# number of tries to start playing the next song
MAX_RETRIES=10

GPIO.setmode(GPIO.BOARD)         #Set GPIO pin numbering
GPIO.setup(12, GPIO.IN, pull_up_down=GPIO.PUD_UP) #Enable input and pull up resistors

# handle play/pause button
def handlePlay(channel):
    retry = True
    tries = 0
    while retry and tries<MAX_RETRIES:
        tries = tries + 1
        try:
            # if playing, then pause
            if client.status()["state"] == 'play':
                client.pause()
                print("Pause playback")
            else: # if paused or not started, play the next song
                client.play()
                print(client.playlistinfo()[0]['name'])
            retry = False
        except ConnectionError as connerr:
            # on connection error try reconnecting and retry after 2 seconds
            print("Connection lost, reconnecting")
            client.connect("localhost", 6600)
            retry = True
            time.sleep(2) # wait 2 seconds before trying another connection

# add interrupt-based handler for the button
GPIO.add_event_detect(12, GPIO.FALLING, callback=handlePlay, bouncetime=300);

try:
    # endless loop with huge delay to not consume the CPU a lot
    while True:
        time.sleep(60)
except KeyboardInterrupt:
    print("Exiting")
# on keyboard interrupt cleanup GPIO
GPIO.cleanup()
