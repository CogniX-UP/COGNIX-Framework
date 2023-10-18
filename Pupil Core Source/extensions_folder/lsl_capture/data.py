class Keywords:
    bothEyes = "both"
    leftEye = "left"
    rightEye = "right"
    worldCoord = "world-space"
    objCoord = "object-space"
    normUnit = "normalized"
    pixelUnit = "pixels"
    mmUnit = "mm"
    
class Channel:
    def __init__(self, label, eye, metatype, unit, coordinate_system = None, query = None):
        self.label = label
        self.eye = eye
        self.metatype = metatype
        self.unit = unit
        self.coordinate_system = coordinate_system
        self.query = query

    def append_to(self, channels):
        chan = channels.append_child("channel")
        chan.append_child_value("label", self.label)
        chan.append_child_value("eye", self.eye)
        chan.append_child_value("type", self.metatype)
        chan.append_child_value("unit", self.unit)
        if self.coordinate_system:
            chan.append_child_value("coordinate_system", self.coordinate_system)

class EyeChannelCollection:
    confidence_channel:Channel
    norm_pos_channels: list[Channel]
    gaze_point_3d_channels: list[Channel]
    eye_center_channels: list[list[Channel]]
    gaze_normal_channels: list[list[Channel]]
    diameter_2d_channels: list[Channel]
    diameter_3d_channels: list[Channel]
    
    def __init__(self, pos_coord_space:str = Keywords.worldCoord):
        self.confidence_channel = confidence_channel()
        self.norm_pos_channels = [norm_pos_channel(i, pos_coord_space) for i in range(2)]
        self.gaze_point_3d_channels = [gaze_point_3d_channel(i) for i in range(2)]
        
        right_eye_center = [eye_center_channel(0, i) for i in range(3)]
        left_eye_center = [eye_center_channel(1, i) for i in range(3)]
        self.eye_center_channel = [right_eye_center, left_eye_center]
        
        right_eye_normal = [gaze_normal_channel(0, i) for i in range(3)]
        left_eye_normal = [gaze_normal_channel(1, i) for i in range(3)]
        self.gaze_normal_channels = [right_eye_normal, left_eye_normal]
        
        self.diameter_2d_channels = [diameter_2d_channel(eye) for eye in range(2)]
        self.diameter_3d_channels = [diameter_3d_channel(eye) for eye in range(2)]
    
    def norm_pos_channel(self, xy: int) -> Channel:
        index = get_pos_index(xy)
        return self.norm_pos_channels[index]
    def gaze_point_3d_channel(self, xyz: int) -> Channel:
        return self.gaze_point_3d_channels(get_pos_index(xyz))
    def eye_center_channel(self, eye: int, xyz: int) -> Channel:
        return self.eye_center_channels[get_eye_index(eye)][get_pos_index(xyz)]
    def gaze_normal_chanel(self, eye: int, xyz: int) -> Channel:
        return self.gaze_normal_channels[get_eye_index(eye)][get_pos_index(xyz)]
    def diameter_2d_channel(self, eye: int) -> Channel:
        return self.diameter_2d_channel[get_eye_index(eye)]
    def diameter_3d_channel(self, eye: int) -> Channel:
        return self.diameter_3d_channels[eye]
    def all_channels(self, buffer: list[Channel] = None) -> list[Channel]:
        if buffer is None:
            buffer = []
        buffer.append(self.confidence_channel)
        for c in self.norm_pos_channels:
            buffer.append(c)
        for c in self.gaze_point_3d_channels:
            buffer.append(c)
        for eye in self.eye_center_channels:
            for c in eye:
                buffer.append(c)
        for eye in self.eye_center_channels:
            for c in eye:
                buffer.append(c)
        for c in self.diameter_2d_channels:
            buffer.append(c)
        for c in self.diameter_3d_channels:
            buffer.append(c)
        return buffer 
def confidence_channel(query = None) -> Channel:
    return Channel("confidence", Keywords.bothEyes, "Confidence", Keywords.normUnit, query)

def norm_pos_channel(i:int, query = None, pos_coord_system:str = Keywords.worldCoord) -> Channel:
    return Channel("norm_pos_" + "xy"[i], Keywords.bothEyes, get_screen_meta(i), Keywords.normUnit, pos_coord_system, query)

def gaze_point_3d_channel(channel:int, query = None) -> Channel:
    return Channel("gaze_point_3d_" + "xyz"[channel], 
                   Keywords.bothEyes, get_dir_meta(channel), Keywords.mmUnit, Keywords.worldCoord, query)
    
def eye_center_channel(eye:int, channel:int, query = None) -> Channel:
    return  Channel("eye_center{}_3d_{}".format(eye, "xyz"[channel]),
                    get_eye_type(eye), get_position_meta(channel), Keywords.mmUnit, Keywords.worldCoord, query)
def gaze_normal_channel(eye:int, channel:int, query = None) -> Channel:
    return Channel("gaze_normal{}_{}".format(eye, "xyz"[channel]), get_eye_type(eye), get_position_meta(channel), Keywords.mmUnit, Keywords.worldCoord, query)

def diameter_2d_channel(eye:int, query = None) -> Channel:
    return Channel("diameter{}_2d".format(eye), get_eye_type(eye), "Diameter", Keywords.pixelUnit, "eye{}".format(eye), query)

def diameter_3d_channel(eye:int, query = None) -> Channel:
    return Channel("diameter{}_3d".format(eye), get_eye_type(eye), "Diameter", Keywords.mmUnit, "eye{}".format(eye), query)

def get_eye_type(eye: int) -> str:
    (Keywords.rightEye, Keywords.leftEye)[eye]
def get_position_meta(channel: int) -> str:
    return "Position" + "XYZ"[channel]
def get_screen_meta(channel: int) ->str:
    return "Screen" + "XY"[channel]
def get_dir_meta(channel: int) -> str:
    return "Direction" + "XYZ"[channel]

def get_pos_index(s) -> int:
    if (type(s) is int):
        return s
    index = 0
    if (s is "x" or s is "X"):
        index = 0
    elif (s is "y" or s is "Y"):
        index = 1
    else:
        index = 2
    return index

def get_eye_index(i) -> int:
    if (type(i) is int):
        return i
    if (i is Keywords.rightEye):
        return 0
    else:
        return 1