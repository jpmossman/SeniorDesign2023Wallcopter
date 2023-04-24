from djitellopy import tello
import KeyPress as kp
from time import sleep

me = tello.Tello()

def getKeyBoardInput():
    lr, fb, ud, yv = 0, 0, 0, 0
    speed = 50
    if kp.getKey("LEFT"): lr = -speed
    elif kp.getKey("RIGHT"): lr = speed

    if kp.getKey("UP"): fb = speed
    elif kp.getKey("DOWN"): fb = -speed

    if kp.getKey("w"): ud = speed
    elif kp.getKey("s"): ud = -speed

    if kp.getKey("a"): yv = speed
    elif kp.getKey("d"): yv = -speed

    if kp.getKey("q"):
        me.land()
        return 0,0,0,0

    if kp.getKey("e"):
        me.takeoff()
        return 0,0,0,0

    return lr, fb, ud, yv


def main():
    kp.init()
    me.connect()
    print(me.get_battery())

    while True:
        me.send_rc_control(*getKeyBoardInput())
        sleep(0.05)

if __name__ == "__main__":
    main()
