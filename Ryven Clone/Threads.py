import logging
import threading
import time
import test_lsl_in
########## One thread

def thread_function(name):
    logging.info('Thread %s: starting',name)
    time.sleep(2)
    logging.info('Thread %s: finishing',name)



if __name__ == '__main__':
    format = "%(asctime)s:%(message)s"
    logging.basicConfig(format=format,level=logging.INFO,datefmt="%H:%M:%S")
    
    logging.info("Main    : before creating thread")
    x = threading.Thread(target=thread_function,args=(1,),daemon=True)
    logging.info("Main    : before running thread")
    x.start()
    logging.info("Main    : wait for the thread to finish")
    x.join()

    logging.info("Main    : all done")


###### Multiple threads
# import logging
# import threading
# import time

# def thread_function(name):
#     logging.info("Thread %s: starting", name)
#     time.sleep(2)
#     logging.info("Thread %s: finishing", name)

# if __name__ == "__main__":
#     format = "%(asctime)s: %(message)s"
#     logging.basicConfig(format=format, level=logging.INFO,
#                         datefmt="%H:%M:%S")

#     threads = list()
#     for index in range(3):
#         logging.info("Main    : create and start thread %d.", index)
#         x = threading.Thread(target=thread_function, args=(index,))
#         threads.append(x)
#         x.start()

#     for index, thread in enumerate(threads):
#         logging.info("Main    : before joining thread %d.", index)
#         thread.join()
#         logging.info("Main    : thread %d done", index)

##--- Using ThreadPoolExecutor
# import concurrent.futures
# import logging
# import threading
# import time

# def thread_function(name):
#     logging.info("Thread %s: starting", name)
#     time.sleep(2)
#     logging.info("Thread %s: finishing", name)

# if __name__ == "__main__":
#     format = "%(asctime)s: %(message)s"
#     logging.basicConfig(format=format, level=logging.INFO,
#                         datefmt="%H:%M:%S")

#     with concurrent.futures.ThreadPoolExecutor(max_workers=3) as executor:
#         executor.map(thread_function, range(3))


#### Race conditions
# import concurrent.futures
# import logging
# import threading
# import time
# class FakeDatabase:
#     def __init__(self):
#         self.value = 0

#     def update(self, name):
#         logging.info("Thread %s: starting update", name)
#         local_copy = self.value
#         local_copy += 1
#         time.sleep(0.1)
#         self.value = local_copy
#         logging.info("Thread %s: finishing update", name)

# if __name__ == "__main__":
#     format = "%(asctime)s: %(message)s"
#     logging.basicConfig(format=format, level=logging.INFO,
#                         datefmt="%H:%M:%S")

#     database = FakeDatabase()
#     logging.info("Testing update. Starting value is %d.", database.value)
#     with concurrent.futures.ThreadPoolExecutor(max_workers=2) as executor:
#         for index in range(2):
#             executor.submit(database.update, index)
#     logging.info("Testing update. Ending value is %d.", database.value)


import numpy as np
import random

class Button:
    def __init__(self):
        self.display_title = ''
        self.values = None
        
        start_wait = 1.0
        aqcuire_wait = 2.0
        recording_seconds = 5
        while(True):
            x = threading.Thread(target=self.thread_function,args=(1,recording_seconds,start_wait,aqcuire_wait),daemon=True)        
            x.start()
            x.join()
    
    def thread_function(self,name,recording_seconds,start_wait:float,aqcuire_wait:float,stop:int):
        ####Connection / Problem what happens if connections closes 
        logging.info('Thread %s: start waiting',name)
        time.sleep(start_wait)
        logging.info('Thread %s: finished waiting',name)
        while(True):
            self.timestamp,self.chunk = test_lsl_in.GetDataStream()
            # self.test = self.dinit(recording_seconds)
            self.test = self.chunk
            print(self.test)
            # print("New array",self.test)
            # logging.info('Thread %s: acquired',self.test)
            # self.update_event()
            time.sleep(aqcuire_wait)
        
    # def initialize(self,recording_seconds):
    #     L = np.zeros((32,256*recording_seconds))
    #     for j in range(32):
    #         for i in range(256*recording_seconds):
    #             L[j,i]=random.randint(0,10)
    #     return L


Button1 = Button()
