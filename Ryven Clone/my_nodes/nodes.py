from ryven.node_env import *
import random
import time
import ryvencore as rc
import numpy as np
import logging
import threading
# from ryven.test_lsl_in import *
import pylsl
from ryven.Event import creation_event
##### node definitions

class RandNode(Node):
    title = 'Rand'
    tags = ['random','numbers']
    init_inputs=[NodeInputType()]
    init_outputs=[NodeOutputType()]
    
    def update_event(self,inp=-1):
        self.set_output_val(0,Data(random.random()*self.input(0).payload))

class CognixAcquirer:
    #states
    idle = "idle"
    searching = "searching"
    acquiring = "acquiring"
    
    #members
    _state:str
    __acq_events:list[object]
    __acq_thread: threading.Thread
    __inlet: pylsl.StreamInlet
    stream_name:str
    
    @property
    def state(self):
        return self._state
    def __init__(self):
        self.__state = self.idle
        self.__acq_events = []
        self.stream_name = "EEG"
        self.__inlet = None
        #self.__acq_thread = None
        
    def start(self, name:str = None):
        if (self.__state is not self.idle):
            return False
        self.__state = self.searching
        
        if (name is None):
            name = self.stream_name
        self.stream_name = name
        th = self._create_thread()
        self.__acq_thread = th
        th.start()
        return True
    
    def stop(self):
        th =self.__acq_thread
        self.__state = self.idle
        th.join()
        
    def reset(self):
        self.stop()
        self.start()
    
    #Events
    def add_event(self, func):
        self.__acq_events.append(func)
    def remove_event(self, func):
        self.__acq_events.remove(func)
    def clear_events(self):
        self.__acq_events.clear()
    
    def _create_thread(self):
        def thread_func():
            try:
                streams = pylsl.resolve_byprop('type','EEG',timeout=10) ###in seconds
                if (len(streams) == 0):
                    raise Exception("Oopsie")
                stream = streams[0]
                print("Found stream name {}".format(stream.name))
                self.__state = self.acquiring   
                self.__inlet = pylsl.StreamInlet(info = stream, recover = False)
                
                while (self.__state == self.acquiring):
                    chunk, ts = self.__inlet.pull_chunk()
                    if not ts:
                        continue
                    for e in self.__acq_events:
                        e(chunk)
                    time.sleep(0.5)
            except BaseException as e:
                print(e.args)
                pass
            
            self.__state = self.idle
            self.__inlet = None
            
        return threading.Thread(target=thread_func, args=(),daemon=True) 
        
class PrintNode(Node):
    title = 'Print Values'
    init_inputs=[NodeInputType()]
    
    def update_event(self,inp=-1):
        print(self.input(0))

class TestOut(Node):
    title = "Test out"
    init_outputs = [NodeOutputType()]
    
    def initialize(self):
        super().initialize()
        while (True):
            self.set_output_val(0, Data(2))
            time.sleep(5)
        return None
class ThreadNode(Node):
    title = "Thread-Line"
    init_outputs=[NodeOutputType()]
    
    def __init__(self, params):
        super().__init__(params)
        self.acq = CognixAcquirer()
        
        def handle_data(chunk):
            data = Data(chunk)
            #print(data)
            self.set_output_val(0, data)
        
        self.acq.add_event(handle_data)
        try:
            self.acq.start()
        except BaseException as e:
            print(e.args)
    
    def __del__(self):
        self.acq.stop()
######## export nodes
# export_nodes([RandNode,PrintNode,newThreadNode])

export_nodes([RandNode,PrintNode,ThreadNode, TestOut])



