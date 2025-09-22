
import math
import numpy as np

# in mm
COXA_LENGTH=33
FEMUR_LENGTH=90
TIBIA_LENGTH=110


# x, y, z offsets of the coxa joints 
# from the center of the robot
# in mm
LEG_COXA_OFFSETS = np.mat([
                            [55, 76, 0],
                            [85, 0, 0],
                            [55, -76, 0],
                            [-55, -76, 0],
                            [-85, 0, 0],
                            [-55, 76, 0]
                                        ]).T


def coordinate_to_angle(x, y, z):

    # --calculate coxa angle--
    # get the angle from the y axis to the y, z point
    # we subtract pi/2 because we use servo angle 0 
    # as home postition
    coxa_angle = math.pi/2 - np.arctan2(y,z)

    # calculate femur and tibia angles using cosine rule


