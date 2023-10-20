"""Example program to demonstrate how to send a multi-channel time series to LSL"""

import sys
import time
import random
import argparse

#####StreamInfo:put info into the Stream
#####StreamOutlet: send data Stream out to the network

from pylsl import StreamInfo,StreamOutlet,local_clock

def main(name="LSLExampleAmp",stream_type='EEG',srate=100):
    print(srate) #rate of sending data
    channel_names = ["Fp1", "Fp2", "C3", "C4", "Cz", "P3", "P4", "Pz", "O1", "O2"]
    channel_locations = [
        [-0.0307, 0.0949, -0.0047],
        [0.0307, 0.0949, -0.0047],
        [-0.0742,  4.54343962e-18,  0.0668],
        [0.0743, 4.54956286e-18, 0.0669],
        [0, 6.123234e-18, 0.1],
        [-0.0567, -0.0677,  0.0469],
        [0.0566, -0.0677,  0.0469],
        [8.74397815e-18, -0.0714,  0.0699],
        [-0.0307, -0.0949, -0.0047],
        [0.0307, -0.0949, -0.0047]
    ]
    
    n_channels=len(channel_names)
    
    
    #### Create the stream info
    info = StreamInfo(name,stream_type,n_channels,srate,'float32','myuid34234')
    
    ##### Append some meta-data
    ##### append_child() is the title of the header of the meta-data
    #### append_child_value is the variable of the header of the meta-data in which we insert a value
    
    
    
    info.desc().append_child_value("manufacturer","LSLExampleAmp")
    chns = info.desc().append_child("channels")
    for chan_ix,label in enumerate(channel_names):
        ch = chns.append_child("channel")
        ch.append_child_value("label",label)
        ch.append_child_value("unit","microvolts")
        ch.append_child_value("type","EEG")
        ch.append_child_value("scalling_factor","1")
        loc = ch.append_child("location")  
        for ax_str,pos in zip(["X","Y","Z"],channel_locations[chan_ix]):
            loc.append_child_value(ax_str,str(pos))
    cap = info.desc().append_child("cap")
    cap.append_child_value("name","ComfyCap")
    cap.append_child_value("size","54")
    cap.append_child_value("labelscheme","10-20") 
         
    #### Make a data outlet
    # next make an outlet; we set the transmission chunk size to 32 samples
    # and the outgoing buffer size to 360 seconds (max.)
    outlet = StreamOutlet(info, 1000, 360)
    
    #### Begin the Stream....
    print("now sending data....")
    start_time = local_clock()
    sent_samples = 0
    mychunk=[[]]
    
    while True:
        elapsed_time = local_clock() - start_time
        print(elapsed_time)
        required_samples = int(srate*elapsed_time)- sent_samples
        print(required_samples)
        if required_samples > 0:
            mychunk = [[random.random() for chan_ix in range(n_channels)] for samp_ix in range(required_samples)]
                        
        print(mychunk)
       
        # Get a time stamp in seconds. We pretend that our samples are actually
        # 125ms old, e.g., as if coming from some external hardware with known latency.
        stamp = local_clock() - 0.125
        
        # now send it and wait for a bit
        
        # Note that even though `rand()` returns a 64-bit value, the `push_chunk` method
        #  will convert it to c_float before passing the data to liblsl.
        outlet.push_chunk(mychunk)
        sent_samples+=required_samples
        ##### sent data and wait for 1 second
        time.sleep(0.5)
        

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--name', default='LSLExampleAmp',
                        help="Name of the created stream.")
    parser.add_argument('--type', default='EEG',
                        help="Type of the created stream.")
    parser.add_argument('--srate', default=256.0, help="Sampling rate of the created stream.", type=float)
    arg = parser.parse_args()
    
    main(name=arg.name, stream_type=arg.type, srate=arg.srate)
    
    
