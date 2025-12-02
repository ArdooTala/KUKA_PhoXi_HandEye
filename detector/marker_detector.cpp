#include "marker_detector.h"
#include <iostream>

namespace markerDetection {
PhoXiCam::PhoXiCam (std::string id) : 
hwId(id)
{
    if (!factory.isPhoXiControlRunning()) {
        std::cerr << "PhoXiControl is not running" <<std::endl;
        return;
    }
    std::cout << "PhoXi Control Version: " << factory.GetPhoXiControlVersion() << std::endl;
    std::cout << "PhoXi API Version: " << factory.GetAPIVersion() << std::endl << std::endl;

}


PhoXiCam::~PhoXiCam () {
    if (device) {
        std::cout << "Wow wow wow...What's the rush?!" << std::endl;
    }
}

bool PhoXiCam::connect() {
    //Try to connect device opened in PhoXi Control, if any
    device = factory.CreateAndConnectFirstAttached();
    if (device)
    {
        std::cout << "You have already PhoXi device opened in PhoXi Control: "
            << (std::string) device->HardwareIdentification << std::endl;
    }
    else
        device = factory.CreateAndConnect(hwId);

    //Check if device was created
    if (!device)
    {
        std::cout << "Your device was not created!" << std::endl;
        return 0;
    }

    //Check if device is connected
    if (!device->isConnected())
    {
        std::cout << "Your device is not connected" << std::endl;
        return 0;
    }

    std::cout << "Connected to the device: "
        << (std::string) device->HardwareIdentification << std::endl;

    return 1;
}

void PhoXiCam::initDevice () {
    if (!connect()) {
        std::cerr << "Could not connect to the device!" <<std::endl;
        return;
    }

    device->MotionCam->OperationMode = pho::api::PhoXiOperationMode::Scanner;
    device->MotionCamScannerMode->TextureSource = pho::api::PhoXiTextureSource::LED;
    device->CoordinatesSettings->CameraSpace = pho::api::PhoXiCameraSpace::PrimaryCamera;
    device->CoordinatesSettings->CoordinateSpace = pho::api::PhoXiCoordinateSpace::MarkerSpace;
    device->CoordinatesSettings->RecognizeMarkers = true;

    std::cout << "Device config is set for calibration." << std::endl;
}

} // namespace MarkerDetection
