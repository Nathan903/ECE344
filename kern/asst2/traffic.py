import random
import threading

NCARS = 1

directions = ["N", "E", "S", "W"]
msgs = [
    "approaching:",
    "region1:    ",
    "region2:    ",
    "region3:    ",
    "leaving:    "
]

# Use these constants for the first parameter of the message
APPROACHING, REGION1, REGION2, REGION3, LEAVING = range(5)

def message(msg_nr, carnumber, cardirection, destdirection):
    print(
        f"{msgs[msg_nr]} car = {carnumber}, direction = {directions[cardirection]}, destination = {directions[destdirection]}")

def gostraight(cardirection, carnumber):
  print('straight')
  nturns = 2
  destdirection = (cardirection+nturns)%4
  for reg in range(nturns+1):
    message(reg, carnumber,  cardirection,destdirection)
  message(LEAVING, carnumber,  cardirection,destdirection)

def turnleft(cardirection, carnumber):
  print('left')
  nturns = 3
  destdirection = (cardirection+nturns)%4
  for reg in range(nturns+1):
    message(reg, carnumber,  cardirection,destdirection)
  message(LEAVING, carnumber,  cardirection,destdirection)

def turnright(cardirection, carnumber):
  print('right')
  nturns = 1
  destdirection = (cardirection+nturns)%4
  for reg in range(nturns+1):
    message(reg, carnumber,  cardirection,destdirection)
  message(LEAVING, carnumber,  cardirection,destdirection)

def approachintersection(carnumber):
    cardirection = random.randint(0, 3)
    way = random.randint(0, 2)

    if way == 0:
        gostraight(cardirection, carnumber)
    elif way == 1:
        turnleft(cardirection, carnumber)
    else:
        turnright(cardirection, carnumber)

import time
def createcars():
    # Start NCARS approachintersection() threads.
    threads = []
    for index in range(NCARS):
        thread = threading.Thread(target=approachintersection, args=(index,))
        thread.start()
        threads.append(thread)

    # Wait until all other threads finish
    for thread in threads:
        thread.join()

    print("stoplight test done")
start = time.time()
createcars()
end = time.time()
print(end-start)