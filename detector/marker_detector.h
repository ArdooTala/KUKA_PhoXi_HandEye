#pragma once

#include <PhoXi.h>
#include <string>

namespace markerDetection {

class PhoXiCam {
public:
    PhoXiCam (std::string id);
    ~PhoXiCam ();
    void initDevice();
private:
    bool connect();
private:
    std::string hwId;
    pho::api::PhoXiFactory factory;
    pho::api::PPhoXi device;
    pho::api::PFrame frame;
};

} // namespace markerDetection

