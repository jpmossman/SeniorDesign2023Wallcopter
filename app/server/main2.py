import paho.mqtt.client as mqtt
import pygame
from djitellopy import tello
import cv2
import numpy as np
from time import time
from dataclasses import dataclass


class Settings:
    # Enable/disable certain flight modes
    STICK_ENABLED = False
    RISE_ENABLED = False
    CRAWL_ENABLED = False

    moe: int = 5 # Margin of error on staying near the ceiling
    broker = "192.168.10.3"
    port = 1883
    client_id = "WallCopter Control"

class OurGlobals:
    mqtt_client: mqtt.Client
    esp_frame = 0 # Most recent frame sent by ESP
    drone:tello.Tello
    dist:int = 0 # Most recent distance sent by ESP

class UserControls:
    # Six axis movement commands
    up: str = "w"
    down: str = "s"
    left: str = "LEFT"
    right: str = "RIGHT"
    yawl: str = "a"
    yawr: str = "d"
    # State change commands
    land: str = "q"
    takeoff: str = "e"
    rise: str = "t"
    crawl: str = "y"
    stick: str = "u"

@dataclass
class UserInput:
    # Six axis movement commands
    up: bool = False
    down: bool = False
    left: bool = False
    right: bool = False
    yawl: bool = False
    yawr: bool = False
    # State change commands
    land: bool = False
    takeoff: bool = False
    rise: bool = False
    crawl: bool = False
    stick: bool = False

@dataclass
class FSM_State:
    state: str
    keys: pygame.key.ScancodeWrapper
    dist: int
    time: float
    time_lock: float
    queued_order: str

def getKeyBoardInput(speed:int = 50):
    def getKey(keyName):
        ans = False
        for eve in pygame.event.get(): pass
        keyInput = pygame.key.get_pressed()
        myKey = getattr(pygame, f'K_{keyName}')
        if keyInput[myKey]:
            ans = True
        pygame.display.update()
        return ans
    lr, fb, ud, yv = 0, 0, 0, 0
    if getKey("LEFT"): lr = -speed
    elif getKey("RIGHT"): lr = speed

    if getKey("UP"): fb = speed
    elif getKey("DOWN"): fb = -speed

    if getKey("w"): ud = speed
    elif getKey("s"): ud = -speed

    if getKey("a"): yv = speed
    elif getKey("d"): yv = -speed

    return lr, fb, ud, yv

def get_state(keys, state:str, time:int, time_lock:int, dist:int) -> str:
    new_state = ""

    # q to land
    if keys[pygame.K_q]:
        return "land"
    # e to takeoff
    if keys[pygame.K_e]:
        new_state = "takeoff"
    # t to rise to the ceiling
    if keys[pygame.K_t]:
        pass
    # y to crawl
    if keys[pygame.K_y]:
        pass
    # u to stick
    if keys[pygame.K_u]:
        pass
     
    return state if not new_state else new_state

def get_next_state(state:FSM_State) -> FSM_State:
    """
    For a given state, return what the next state of the FSM would be
    """
    # Has the time lock not yet passed?
    if state.time_lock > state.time:
        return state

    # The new state we will return
    new_state = FSM_State(
        "",                 # State name has not yet been determined
        {},                 # Next state needs to fetch its own keys
        state.dist,         # FSM does not change distance
        state.time,         # FSM does not change time
        state.time_lock,    # FSM does not change the time lock
        state.queued_order  # FSM does not change the next order
    )
    
    # Based off of current state name, check conditions
    if state.state == "land":
        new_state.state = "land"
        # press e to takeoff
        if state.keys[pygame.K_e]:
            new_state.state = "takeoff"
            new_state.time_lock = new_state.time + 3
        # press t to rise
        if state.keys[pygame.K_t] and Settings.RISE_ENABLED:
            new_state.state = "takeoff"
            new_state.time_lock = new_state.time + 3
            new_state.queued_order = "rise"

    elif state.state == "takeoff":
        pass

    elif state.state == "normal":
        new_state.state = "normal"
        # press q to land
        if state.keys[pygame.K_q]:
            new_state.state = "land"
        # press t to rise
        if state.keys[pygame.K_t] and Settings.RISE_ENABLED:
            # TODO: Check if we are already with in range of ceiling
            new_state.state = "rise"

    elif state.state == "rise":
        pass

    elif state.state == "crawl":
        pass

    elif state.state == "stick":
        pass

    else:
        pass

    return new_state

def main():
    # Start mqtt client
    client = mqtt.Client(Settings.client_id)
    OurGlobals.mqtt_client = client
    def on_message(client, userdata, msg):
        if msg.topic == "wc/frame":
            imgNp = np.array(bytearray(msg.payload),dtype=np.uint8)
            img = cv2.imdecode(imgNp,-1)
            cv2.imshow("Image",img)
        if msg.topic == "wc/dist":
            int_val = int.from_bytes(msg.payload, "little")
            print(f"Distance received: {int_val}")
    client.on_message = on_message
    client.connect(Settings.broker,Settings.port)
    client.subscribe("#")
    client.loop_forever() # Should be loop_start, but for demo we want to block here

    # Init pygame
    pygame.init()
    win = pygame.display.set_mode((400,400))

    # Connect to tello and start stream
    drone = tello.Tello()
    OurGlobals.drone = drone
    drone.connect()
    drone.streamon()
    print(f"Drone connected. Battery at {drone.get_battery()}%")

    # Determine initial state
    drone_status = "landed"
    state = FSM_State("land", {}, -1, -1, -1, None)
    # Main event loop
    while True:
        # Update system state with external data
        state.time = time()
        state.dist = OurGlobals.dist

        # Get controls and update system state
        for event in pygame.event.get(): pass
        state.keys = pygame.key.get_pressed()

        # Determine new state
        state = get_next_state(state)
        
        # Act based on system state
        if state.state == "land":
            # We have been instructed to land
            if drone_status != "landed":
                drone.send_rc_control(0,0,0,0)
                drone.land()
                drone_status = "landed"

        elif state.state == "takeoff":
            if drone_status == "landed":
                drone.takeoff()
                drone_status = "flying"

        elif state.state == "normal":
            if drone_status == "landed":
                drone.takeoff()
                drone_status = "flying"
            drone.send_rc_control(*getKeyBoardInput())

        elif state.state == "rise":
            pass

        elif state.state == "crawl":
            pass

        elif state.state == "stick":
            # Stick to the ceiling
            if drone_status != "stuck":
                drone.send_rc_control(0,0,50,0)

        else:
            drone.land()
            raise ValueError(f"Invalid internal state encountered: {state.state}")
        
        # Update user display
        img = drone.get_frame_read().frame
        img = cv2.resize(img, (360, 240))
        cv2.imshow("Image", img)
        cv2.waitKey(1)
        

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        OurGlobals.drone.land()
