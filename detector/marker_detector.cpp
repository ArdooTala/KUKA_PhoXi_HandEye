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
        if (device->isAcquiring()) device->StopAcquisition();
    }
}

bool PhoXiCam::connect() {
    //Try to connect device opened in PhoXi Control, if any
    device = factory.CreateAndConnectFirstAttached();
    if (device)
        std::cout << "You have already PhoXi device opened in PhoXi Control: " << std::endl;
    else
        device = factory.CreateAndConnect(hwId);

    if (!device)
    {
        std::cout << "Your device was not created!" << std::endl;
        return 0;
    }
    if (!device->isConnected())
    {
        std::cout << "Your device is not connected" << std::endl;
        return 0;
    }

    std::cout << "Connected device: " << (std::string) device->HardwareIdentification << std::endl;

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

void PhoXiCam::trigger() {
    if (!device || !device->isConnected())
        throw std::runtime_error("Device not connected");

    // Start the device acquisition, if necessary
    if (!device->isAcquiring()) {
        if (!device->StartAcquisition()) {
            throw std::runtime_error("Error in StartAcquisition");
        }
    }

    // We can clear the current Acquisition buffer -- This will not clear
    // Frames that arrive to the PC after the Clear command is performed
    auto clearedFrames = device->ClearBuffer();
    std::cout << "Dropped " << clearedFrames
            << " frames in acquisition buffer" << std::endl;

    // While we checked the state of the StartAcquisition call,
    // this check is not necessary, but it is a good practice
    if (!device->isAcquiring())
        throw std::runtime_error("Scanner is not acquiring");

    std::cout << "Triggering a scan..." << std::endl;
    const auto frameID = device->TriggerFrame(
            /*WaitForAccept*/true,
            /*WaitForGrabbingEnd*/true);

    if (frameID < 0) {
        // If negative number is returned trigger was unsuccessful
        throw std::runtime_error("Trigger was unsuccessful!");
    }

    std::cout << "Scan was triggered, Frame Id: " << frameID << std::endl;

    // Wait for a frame with specific FrameID. There is a possibility, that
    // frame triggered before the trigger will arrive after the trigger
    // call, and will be retrieved before requested frame
    // Because of this, the TriggerFrame call returns the requested frame
    // ID, so it can than be retrieved from the Frame structure. This call
    // is doing that internally in background
    // You can specify Timeout here - default is the Timeout stored in Timeout
    // Feature -> Infinity by default
    frame = device->GetSpecificFrame(frameID);

    if (!frame || !frame->Successful) {
        throw std::runtime_error("Failed to retrieve frame");
    }

    std::cout << "Frame successfully retrieved." << std::endl;

    if (frame->Empty()) {
        throw std::runtime_error("Frame is empty");
    }

    if (frame->Texture.Empty()) {
        throw std::runtime_error("Frame Texture is empty");
    }
}

pho::api::PhoXiCoordinateTransformation PhoXiCam::getCameraTransform() {
    cameraPosition.Rotation[0][0] = (double)(frame->Info.CurrentCameraXAxis.x);
    cameraPosition.Rotation[1][0] = (double)(frame->Info.CurrentCameraXAxis.y);
    cameraPosition.Rotation[2][0] = (double)(frame->Info.CurrentCameraXAxis.z);

    cameraPosition.Rotation[0][1] = (double)(frame->Info.CurrentCameraYAxis.x);
    cameraPosition.Rotation[1][1] = (double)(frame->Info.CurrentCameraYAxis.y);
    cameraPosition.Rotation[2][1] = (double)(frame->Info.CurrentCameraYAxis.z);

    cameraPosition.Rotation[0][2] = (double)(frame->Info.CurrentCameraZAxis.x);
    cameraPosition.Rotation[1][2] = (double)(frame->Info.CurrentCameraZAxis.y);
    cameraPosition.Rotation[2][2] = (double)(frame->Info.CurrentCameraZAxis.z);

    cameraPosition.Translation = frame->Info.CurrentCameraPosition;

    return cameraPosition;
}

void PhoXiCam::saveFrame() {

}

} // namespace MarkerDetection
