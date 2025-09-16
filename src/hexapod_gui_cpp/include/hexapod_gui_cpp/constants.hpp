#pragma once

#include <string>
#include <vector>
#include <map>
#include <numeric>

namespace Constants {

const std::map<std::string, std::map<std::string, int>> SERVO_CHANNELS = {
    {"camera", {{"pan", 0}, {"tilt", 1}}},
    {"leg1", {{"coxa", 15}, {"femur", 14}, {"tibia", 13}}},
    {"leg2", {{"coxa", 12}, {"femur", 11}, {"tibia", 10}}},
    {"leg3", {{"coxa", 9}, {"femur", 8}, {"tibia", 31}}},
    {"leg4", {{"coxa", 22}, {"femur", 23}, {"tibia", 27}}},
    {"leg5", {{"coxa", 19}, {"femur", 20}, {"tibia", 21}}},
    {"leg6", {{"coxa", 16}, {"femur", 17}, {"tibia", 18}}}
};

const std::map<std::string, std::map<std::string, std::string>> RVIZ_JOINT_NAMES = {
    {"leg1", {{"coxa", "leg1_coxa_joint"}, {"femur", "leg1_femur_joint"}, {"tibia", "leg1_tibia_joint"}}},
    {"leg2", {{"coxa", "leg2_coxa_joint"}, {"femur", "leg2_femur_joint"}, {"tibia", "leg2_tibia_joint"}}},
    {"leg3", {{"coxa", "leg3_coxa_joint"}, {"femur", "leg3_femur_joint"}, {"tibia", "leg3_tibia_joint"}}},
    {"leg4", {{"coxa", "leg4_coxa_joint"}, {"femur", "leg4_femur_joint"}, {"tibia", "leg4_tibia_joint"}}},
    {"leg5", {{"coxa", "leg5_coxa_joint"}, {"femur", "leg5_femur_joint"}, {"tibia", "leg5_tibia_joint"}}},
    {"leg6", {{"coxa", "leg6_coxa_joint"}, {"femur", "leg6_femur_joint"}, {"tibia", "leg6_tibia_joint"}}},
    {"camera", {{"pan", "camera_pan_joint"}, {"tilt", "camera_tilt_joint"}}}
};

// A canonical, ordered list of all joint names
inline std::vector<std::string> getAllRvizJointNames() {
    std::vector<std::string> names;
    std::vector<std::string> components = {"leg1", "leg2", "leg3", "leg4", "leg5", "leg6", "camera"};
    for (const auto& comp : components) {
        // Assuming fixed order of coxa, femur, tibia, then pan, tilt
        if (RVIZ_JOINT_NAMES.count(comp)) {
            const auto& joints = RVIZ_JOINT_NAMES.at(comp);
            if(joints.count("coxa")) names.push_back(joints.at("coxa"));
            if(joints.count("femur")) names.push_back(joints.at("femur"));
            if(joints.count("tibia")) names.push_back(joints.at("tibia"));
            if(joints.count("pan")) names.push_back(joints.at("pan"));
            if(joints.count("tilt")) names.push_back(joints.at("tilt"));
        }
    }
    return names;
}

const std::vector<std::string> ALL_RVIZ_JOINT_NAMES = getAllRvizJointNames();

inline std::map<std::string, double> getInitialJointStates() {
    std::map<std::string, double> states;
    for(const auto& name : ALL_RVIZ_JOINT_NAMES) {
        states[name] = 0.0;
    }
    return states;
}

const std::map<std::string, double> INITIAL_JOINT_STATES = getInitialJointStates();

// Create reverse maps for easy lookup
inline std::map<int, std::pair<std::string, std::string>> createReverseServoMap() {
    std::map<int, std::pair<std::string, std::string>> map;
    for (const auto& comp_pair : SERVO_CHANNELS) {
        for (const auto& joint_pair : comp_pair.second) {
            map[joint_pair.second] = {comp_pair.first, joint_pair.first};
        }
    }
    return map;
}
const std::map<int, std::pair<std::string, std::string>> REVERSE_SERVO_MAP = createReverseServoMap();

inline std::map<std::string, int> createJointNameToChannelMap() {
     std::map<std::string, int> map;
     for (const auto& comp_pair : RVIZ_JOINT_NAMES) {
        for (const auto& joint_pair : comp_pair.second) {
            map[joint_pair.second] = SERVO_CHANNELS.at(comp_pair.first).at(joint_pair.first);
        }
    }
    return map;
}
const std::map<std::string, int> JOINT_NAME_TO_CHANNEL_MAP = createJointNameToChannelMap();


const std::string TOPIC_COLOR_IMG = "/camera/camera/color/image_raw";
const std::string TOPIC_DEPTH_IMG = "/camera/camera/depth/image_rect_raw";
const std::string TOPIC_GAZEBO_CMD = "/joint_trajectory_controller/joint_trajectory";
const std::string SERVICE_SET_SERVO = "/set_servo_angle";

} // namespace Constants