class XSTPropertiesName:
    """gst const property name"""
    MASS_MODEL_TASK = 'task'
    MASS_MODEL_ID = 'id'
    INPUT = 'input'
    OUTPUT = 'output'
    MODULE = 'module'
    CLASS = 'class'


class XSTPluginName:
    """gst subplugin name"""
    MASS_MODEL = 'mass_model'
    MASS_MODEL_POST = 'mass_model_post'

class XSTPostTask:
    """mass model post process class"""
    REID_PERSON_POST = 'reid_person'
    SMOKE_DET_POST = 'smoke_detect'
    BREAK_IN_DET_POST = 'break_in_det'
    MOT_COUNTING_POST = 'mot_counting'
