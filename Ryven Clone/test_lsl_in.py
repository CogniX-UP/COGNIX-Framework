"""Example program to demonstrate how to receive a multi-channel time series from LSL"""

from pylsl import StreamInlet,resolve_stream
from icecream import ic
import numpy as np

def GetDataStream():
    # fNIRS_data = np.array([])
    # log_data = np.array([])
    # ic("looking for a EEG stream....")
    eeg_stream = resolve_stream('type','EEG')
    info = eeg_stream[0]
    print(info)
    
    ### Creating an inltet for stream 
    eeg_inlet = StreamInlet(eeg_stream[0])
    
    print('Connected to outlet' + info.name() +'@'+info.hostname())
    
    while True:
        print(eeg_inlet.info())
        #### Get new sample with a timestamp
        
        # get a new sample (you can also omit the timestamp part if you're not
        # interested in it)
        
        # offset = eeg_inlet.time_correction()
        # print('Offset:' + str(offset))
        
        chunk,timestamp = eeg_inlet.pull_chunk(max_samples=104857600)
        if timestamp:
            a = np.array(chunk)
            print(a.shape,"Data was sent")
            # return timestamp, a
    
GetDataStream()