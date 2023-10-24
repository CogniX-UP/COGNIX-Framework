"""
(*)~----------------------------------------------------------------------------------
 Pupil LSL Relay
 Copyright (C) 2012-2016 Pupil Labs

 Distributed under the terms of the GNU Lesser General Public License (LGPL v3.0).
 License details are in the file license.txt, distributed as part of this software.
----------------------------------------------------------------------------------~(*)
"""

from time import sleep
import logging
import uuid

import numpy as np
import lsl_capture.pylsl as lsl
from lsl_capture.data import *
from plugin import Plugin
from pyglui import ui
from abc import ABC

from pupil.capture_settings.plugins.lsl_capture.data import Keywords

VERSION = '2.1'

logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)


class Pupil_LSL_Relay(Plugin):
    """Plugin to relay Pupil Capture data to LSL"""

    icon_chr = "LR"

    def __init__(self, g_pool, outlet_uuid=None):
        super().__init__(g_pool)
        debug_ts_before = g_pool.get_timestamp()
        time_dif = g_pool.get_now() - lsl.local_clock()
        g_pool.timebase.value = time_dif
        debug_ts_after = g_pool.get_timestamp()
        debug_ts_lsl = lsl.local_clock()
        logger.info("Synchronized time epoch to LSL clock")
        logger.debug("Time before synchronization: {}".format(debug_ts_before))
        logger.debug("Time after synchronization: {}".format(debug_ts_after))
        logger.debug("LabStreamingLayer time: {}".format(debug_ts_lsl))

        w_s = Stream("pupil_capture_world")
        self.world_stream = w_s
        w_s.set_generic_gaze_extraction()
        for i in range(2):
            w_s._eye_channel_collection.norm_pos_channel(i).query = make_extract_normpos(i)
        w_s.construct_stream(VERSION)        

    def recent_events(self, events):
        #for world gaze
        gaze_events = events.get("gaze", [])
        for gaze_event in gaze_events:
            self.push_world_gaze_sample(gaze_event)
        #for surfaces

    def push_world_gaze_sample(self, gaze):
        try:
            sample = [chan.query(gaze) for chan in self.world_stream.q_channels]
        except Exception as exc:
            logger.error(f"Error extracting gaze sample: {exc}")
            logger.debug(str(gaze))
            return
        # push_chunk might be more efficient but does not
        # allow to set explicit timstamps for all samples
        self.world_stream.outlet.push_sample(sample, gaze["timestamp"])

    def init_ui(self):
        """Initializes sidebar menu"""
        self.add_menu()
        self.menu.label = "Pupil LSL Relay"
        self.menu.append(ui.Info_Text("LSL outlet name: `pupil_capture`"))
        self.menu.append(
            ui.Info_Text(
                "LSL outlet format: https://github.com/sccn/xdf/wiki/Gaze-Meta-Data"
            )
        )

    def deinit_ui(self):
        self.remove_menu()

    def get_init_dict(self):
        return {"outlet_uuid": self.world_stream._stream_id}

    def cleanup(self):
        """gets called when the plugin get terminated.
           This happens either voluntarily or forced.
        """
        self.world_stream = None


 
#Stream Definitions and Data. This acts as a container mostly, with some helper methods.
class Stream(ABC):
    _eye_channel_collection:EyeChannelCollection
    _stream_outlet:lsl.StreamOutlet
    _stream_name:str
    _stream_id:str
    q_channels:list[Channel]
    
    def __init__(self, stream_name:str, stream_id:str = None, pos_coord_space:str = Keywords.worldCoord):
        super().__init__()
        self._eye_channel_collection = EyeChannelCollection(pos_coord_space = pos_coord_space)
        self._stream_name = stream_name
        self._stream_id = stream_id or str(uuid.uuid4())
    
    def __del__(self):
        self._eye_channel_collection = None
        self._stream_outlet = None
        
    @property
    def eye_channels(self) -> EyeChannelCollection:
        return self._eye_channel_collection
    @property
    def outlet(self) -> lsl.StreamOutlet:
        return self._stream_outlet
    
    #Sets queries based on gaze events. Ignores the normalized position, which should be set externally.
    def set_generic_gaze_extraction(self):
        eye_c = self._eye_channel_collection
        eye_c.confidence_channel.query = extract_confidence
        for i in range(3):
            eye_c.gaze_point_3d_channel(i).query = make_extract_gaze_point_3d(i)
        
        for eye in range(2):
            for i in range(3):
                eye_c.eye_center_channel(eye, i).query = make_extract_eye_center_3d(eye, i)

        for eye in range(2):
            for i in range(3):
                eye_c.gaze_normal_chanel(eye, i).query = make_extract_gaze_normal_3d(eye, i)
        
        for eye in range(2):
            eye_c.diameter_2d_channel(eye).query = make_extract_diameter_2d(eye)
                 
        for eye in range(2):
            eye_c.diameter_3d_channel(eye).query = make_extract_diameter_3d(eye)
            
        

    #Constructs the stream. Should be called after every query we want has been initialized.
    def construct_stream(self, plugin_version:str):
        self.q_channels = self._eye_channel_collection.queriable_channels()
        info = lsl.StreamInfo(
            name= self._stream_name,
            type="Gaze",
            channel_count=len(self.q_channels),
            channel_format=lsl.cf_double64,
            source_id= self._stream_id,
        )
        
        info.desc().append_child_value("pupil_lsl_relay_version", plugin_version)
        xml_channels = info.desc().append_child("channels")
        for chan in self.q_channels:
            chan.append_to(xml_channels)
        
        self._stream_outlet = lsl.StreamOutlet(info)

def extract_confidence(gaze):
    return gaze["confidence"]


def make_extract_normpos(dim):
    return lambda gaze: gaze["norm_pos"][dim]


def make_extract_gaze_point_3d(dim):
    return (
        lambda gaze: gaze["gaze_point_3d"][dim] if "gaze_point_3d" in gaze else np.nan
    )


def make_extract_eye_center_3d(eye, dim):
    def extract_eye_center_3d(gaze):
        topic = gaze["topic"]
        if topic.endswith("3d.01."):
            if eye in gaze["eye_centers_3d"]:
                return gaze["eye_centers_3d"][eye][dim]
            elif str(eye) in gaze["eye_centers_3d"]:
                return gaze["eye_centers_3d"][str(eye)][dim]
            else:
                raise KeyError(f"Expected field `{eye}` in {gaze['eye_centers_3d']}")
        elif topic.endswith(f"3d.{eye}."):
            return gaze["eye_center_3d"][dim]
        else:
            return np.nan

    return extract_eye_center_3d


def make_extract_gaze_normal_3d(eye, dim):
    def extract_gaze_normal_3d(gaze):
        topic = gaze["topic"]
        if topic.endswith("3d.01."):
            if eye in gaze["gaze_normals_3d"]:
                return gaze["gaze_normals_3d"][eye][dim]
            elif str(eye) in gaze["gaze_normals_3d"]:
                return gaze["gaze_normals_3d"][str(eye)][dim]
            else:
                raise KeyError(f"Expected field `{eye}` in {gaze['gaze_normals_3d']}")
        elif topic.endswith(f"3d.{eye}."):
            return gaze["gaze_normal_3d"][dim]
        else:
            return np.nan

    return extract_gaze_normal_3d


def make_extract_diameter_2d(eye):
    def extract_diameter_2d(gaze):
        base_data = gaze["base_data"]
        for pupil in base_data:
            if pupil["id"] == eye:
                return pupil["diameter"]
        else:
            return np.nan

    return extract_diameter_2d


def make_extract_diameter_3d(eye):
    def extract_diameter_3d(gaze):
        base_data = gaze["base_data"]
        for pupil in base_data:
            if pupil["id"] == eye and "diameter_3d" in pupil:
                return pupil["diameter_3d"]
        else:
            return np.nan

    return extract_diameter_3d
