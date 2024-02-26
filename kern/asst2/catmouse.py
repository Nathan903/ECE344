import threading
import time

# Constants
NFOODBOWLS = 2
NCATS = 6
NMICE = 2
NMEALS = 4

# Locks
# block_cats= threading.Lock()
# block_mouse = threading.Lock()
# block_everybody = threading.Lock()

def catmouse_eat(who, num, bowl, iteration):
    print(dishes)
    print(f"{who}: {num} starts eating: bowl {bowl}, iteration {iteration}")
    time.sleep(1)
    print(f"{who}: {num} ends eating: bowl {bowl}, iteration {iteration}")
    time.sleep(0.1)

dishes = [0,0]
dishes_lock = threading.Lock()
EMPTY=0
CCC = 1
MMM = 2

def catlock(catnumber):    
    for i in range(4):
        no_mouses = False
        have_space = False
        while 1:
            with dishes_lock:
                no_mouses = dishes[0]!=MMM and dishes[1]!=MMM
                have_space = dishes[0]==EMPTY or dishes[1]==EMPTY
                if (no_mouses and have_space):
                    if dishes[0]==EMPTY: 
                        selected_dish = EMPTY
                        dishes[0]=CCC
                    elif dishes[1]==EMPTY: 
                        selected_dish = 1
                        dishes[1]=CCC
                    break
        catmouse_eat("cat",catnumber,selected_dish,i)
        with dishes_lock:
            dishes[selected_dish]=EMPTY

def mouselock(mousenumber):
    for i in range(4):
        no_cat = False
        have_space = False
        while 1:
            with dishes_lock:
                no_cat = dishes[0]!=CCC and dishes[1]!=CCC
                have_space = dishes[0]==EMPTY or dishes[1]==EMPTY
                if (no_cat and have_space):
                    if dishes[0]==EMPTY: 
                        selected_dish = EMPTY
                        dishes[0]=MMM
                    elif dishes[1]==EMPTY: 
                        selected_dish = 1
                        dishes[1]=MMM
                    break
        catmouse_eat("mouse",mousenumber,selected_dish,i)
        with dishes_lock:
            dishes[selected_dish]=EMPTY
import random
def catmouselock():
    # Start NCATS catlock threads
    cat_threads = []
    for index in range(NCATS):
        thread = threading.Thread(target=catlock, args=(index,))
        cat_threads.append(thread)

    # Start NMICE mouselock threads
    mouse_threads = []
    for index in range(NMICE):
        thread = threading.Thread(target=mouselock, args=(index,))
        mouse_threads.append(thread)
    
    all_threads =  mouse_threads+cat_threads
    random.shuffle(all_threads)
    for thread in all_threads:
        thread.start()
    # Wait until all other threads finish
    for thread in cat_threads + mouse_threads:
        thread.join()

    print("catlock test done")

if __name__ == "__main__":
    catmouselock()

"""
--
c-
m-

mm
mm
mm
mm
"""