#include "marker_detector.h"
#include "phoxi/PhoXiFrameDataType.h"

namespace markerDetection {
PhoXiCam::PhoXiCam(std::string id) : hwId(id) {
  if (!factory.isPhoXiControlRunning()) {
    std::cerr << "PhoXiControl is not running" << std::endl;
    return;
  }
  std::cout << "PhoXi Control Version: " << factory.GetPhoXiControlVersion()
            << std::endl;
  std::cout << "PhoXi API Version: " << factory.GetAPIVersion() << std::endl
            << std::endl;
}

PhoXiCam::~PhoXiCam() {
  if (device) {
    std::cout << "Wow wow wow...What's the rush?!" << std::endl;
    if (device->isAcquiring())
      device->StopAcquisition();

    // Wait for recorder to finish recording all wanted frames before stopping
    // recording. This function will return frame index even for frames which
    // were not actually recorded due to `every` (every n-th) skipping.
    while (lastFrameIndex != device->LastRecordedFrameIndex()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (!device->StopRecording()) {
      std::cout << "Failed to stop recording!" << std::endl;
    }

    device->Disconnect();
  }
}

bool PhoXiCam::Connect() {
  // Try to connect device opened in PhoXi Control, if any
  device = factory.CreateAndConnectFirstAttached();
  if (device)
    std::cout << "You have already PhoXi device opened in PhoXi Control: "
              << std::endl;
  else
    device = factory.CreateAndConnect(hwId);

  if (!device) {
    std::cout << "Your device was not created!" << std::endl;
    return 0;
  }
  if (!device->isConnected()) {
    std::cout << "Your device is not connected" << std::endl;
    return 0;
  }

  std::cout << "Connected device: "
            << (std::string)device->HardwareIdentification << std::endl;

  return 1;
}

void PhoXiCam::InitDevice(bool calibration) {
  if (!Connect()) {
    std::cerr << "Could not connect to the device!" << std::endl;
    return;
  }

  device->TriggerMode = pho::api::PhoXiTriggerMode::Software;
  if (!device->TriggerMode.isLastOperationSuccessful()) {
    std::cout << "Failed to set trigger mode to Software!";
    return;
  }

  // Get the current Output configuration
  pho::api::FrameOutputSettings NewOutputSettings;
  NewOutputSettings.SendPointCloud = true;
  NewOutputSettings.SendNormalMap = true;
  NewOutputSettings.SendDepthMap = true;
  NewOutputSettings.SendConfidenceMap = false;
  NewOutputSettings.SendTexture = true;
  NewOutputSettings.SendColorCameraImage = true;
  NewOutputSettings.SendEventMap = false;
  device->OutputSettings = NewOutputSettings;

  device->CoordinatesSettings->CameraSpace =
      pho::api::PhoXiCameraSpace::PrimaryCamera;
  device->CoordinatesSettings->CoordinateSpace =
      calibration ? pho::api::PhoXiCoordinateSpace::MarkerSpace
                  : pho::api::PhoXiCoordinateSpace::CameraSpace;
  device->CoordinatesSettings->RecognizeMarkers = calibration;

  if (device->IsRecording()) {
    device->StopRecording();
  }

  // Note: relative path is relative to PhoXiControl working directory:
  // ~/.PhotoneoPhoXiControl
  auto plyRecordingOptions = R"json({
        "folder": "/home/ardeshir/Desktop/RecordingExampleOutput",
        "every": 1,
        "capacity": -1,
        "pattern": "scan_####",
        "overwrite_existing": true,
        "containers": {
            "ply": {
                "enabled": false,
                "point_cloud": true,
                "depth_map": true,
                "texture": true
            },
            "tif": {
                "enabled": true,
                "point_cloud": false,
                "color_camera_image": true,
                "normal_map": true,
                "depth_map": true,
                "texture": true,
                "split_rgb_channels": false
            }
        }
    })json";

  // Start recording with setup json options for PLY container, do not store
  // options persistently
  pho::api::PhoXi::StartRecordingResult ret =
      device->StartRecording(plyRecordingOptions, false);
  if (ret != pho::api::PhoXi::StartRecordingResult::Success) {
    std::cout << "Failed to start recording! Error: " << static_cast<int>(ret)
              << std::endl;
    return;
  }

  std::cout << "Device config is set for calibration." << std::endl;
}

pho::api::PFrame PhoXiCam::Trigger() {
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
  std::cout << "Dropped " << clearedFrames << " frames in acquisition buffer"
            << std::endl;

  // While we checked the state of the StartAcquisition call,
  // this check is not necessary, but it is a good practice
  if (!device->isAcquiring())
    throw std::runtime_error("Scanner is not acquiring");

  std::cout << "Triggering a scan..." << std::endl;
  lastFrameIndex = device->TriggerFrame(true, true);

  if (lastFrameIndex < 0) {
    throw std::runtime_error("Trigger was unsuccessful!");
  }

  std::cout << "Scan was triggered, Frame Id: " << lastFrameIndex << std::endl;

  // Wait for a frame with specific FrameID. There is a possibility, that
  // frame triggered before the trigger will arrive after the trigger
  // call, and will be retrieved before requested frame
  // Because of this, the TriggerFrame call returns the requested frame
  // ID, so it can than be retrieved from the Frame structure. This call
  // is doing that internally in background
  // You can specify Timeout here - default is the Timeout stored in Timeout
  // Feature -> Infinity by default
  frame = device->GetSpecificFrame(lastFrameIndex);

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

  return frame;
}

pho::api::PhoXiCoordinateTransformation PhoXiCam::GetCameraTransform() {
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

void PhoXiCam::SaveFrame() {}

Eigen::Isometry3d
Phoxi2Eigen(pho::api::PhoXiCoordinateTransformation &phoxi_tf) {
  Eigen::Isometry3d eigen_tf;

  eigen_tf.linear()(0, 0) = phoxi_tf.Rotation[0][0];
  eigen_tf.linear()(0, 1) = phoxi_tf.Rotation[0][1];
  eigen_tf.linear()(0, 2) = phoxi_tf.Rotation[0][2];

  eigen_tf.linear()(1, 0) = phoxi_tf.Rotation[1][0];
  eigen_tf.linear()(1, 1) = phoxi_tf.Rotation[1][1];
  eigen_tf.linear()(1, 2) = phoxi_tf.Rotation[1][2];

  eigen_tf.linear()(2, 0) = phoxi_tf.Rotation[2][0];
  eigen_tf.linear()(2, 1) = phoxi_tf.Rotation[2][1];
  eigen_tf.linear()(2, 2) = phoxi_tf.Rotation[2][2];

  eigen_tf.translation() << phoxi_tf.Translation.x, phoxi_tf.Translation.y,
      phoxi_tf.Translation.z;

  return eigen_tf;
}
} // namespace markerDetection
