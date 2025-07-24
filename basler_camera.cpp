#include "basler_camera.h"
#include <QDir>
#include <QDateTime>

BaslerCamera::BaslerCamera(QObject *parent)
    : QObject(parent)
    , m_camera(nullptr)
    , m_grabThread(nullptr)
    , m_grabFlag(false)
    , m_connected(false)
    , m_width(0)
    , m_height(0)
    , m_fps(0.0)
    , m_scalingFactor(1.0)
    , m_exposureTime(10000.0)
    , m_exposureAuto(false)
    , m_frameRateEnabled(false)
    , m_frameRate(30.0)
    , m_triggerEnabled(false)
    , m_triggerMode("Off")
    , m_triggerSource("Software")
    , m_triggerDelay(0.0)
    , m_recordingEnabled(false)
    , m_recordingPath("./recorded_images")
    , m_recordedImageCount(0)
    , m_maxRecordedImages(100)
    , m_frameCount(0)
    , m_realTimeFrameRate(0.0)
    , m_lastFrameTime(0.0)
    , m_currentFrameId(0)
    , m_errorsCount(0)
{
    qDebug() << "[BaslerCamera] Constructor called";
    
    try {
        // Initialize Pylon
        PylonInitialize();
        qDebug() << "[BaslerCamera] Pylon initialized successfully";
        updateStatus("Pylon initialized");
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error initializing Pylon:" << e.GetDescription();
        updateStatus("Pylon initialization failed");
    }
}

BaslerCamera::~BaslerCamera()
{
    qDebug() << "[BaslerCamera] Destructor called";
    
    disconnect();
    
    try {
        PylonTerminate();
        qDebug() << "[BaslerCamera] Pylon terminated successfully";
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error terminating Pylon:" << e.GetDescription();
    }
}

bool BaslerCamera::connect()
{
    qDebug() << "[BaslerCamera] Connecting to camera...";
    updateStatus("Connecting to camera...");
    
    try {
        // Get the transport layer factory
        CTlFactory& tlFactory = CTlFactory::GetInstance();
        
        // Get all attached devices
        DeviceInfoList_t devices;
        if (tlFactory.EnumerateDevices(devices) == 0) {
            qDebug() << "[BaslerCamera] No camera found!";
            updateStatus("No camera found");
            return false;
        }
        
        qDebug() << "[BaslerCamera] Found" << devices.size() << "camera(s)";
        updateStatus(QString("Found %1 camera(s)").arg(devices.size()));
        
        // Create camera object
        m_camera = new CInstantCamera(tlFactory.CreateFirstDevice());
        
        if (m_camera == nullptr) {
            qDebug() << "[BaslerCamera] Failed to create camera object";
            updateStatus("Failed to create camera object");
            return false;
        }
        
        qDebug() << "[BaslerCamera] Camera created successfully";
        
        // Get camera information
        try {
            CDeviceInfo deviceInfo = m_camera->GetDeviceInfo();
            m_cameraName = QString::fromUtf8(deviceInfo.GetFriendlyName().c_str());
            m_cameraModel = QString::fromUtf8(deviceInfo.GetModelName().c_str());
            m_cameraSerial = QString::fromUtf8(deviceInfo.GetSerialNumber().c_str());
            
            qDebug() << "[BaslerCamera] Camera Name:" << m_cameraName;
            qDebug() << "[BaslerCamera] Camera Model:" << m_cameraModel;
            qDebug() << "[BaslerCamera] Camera Serial:" << m_cameraSerial;
        }
        catch (const GenericException& e) {
            qDebug() << "[BaslerCamera] Error getting camera info:" << e.GetDescription();
        }
        
        // Open camera
        m_camera->Open();
        qDebug() << "[BaslerCamera] Camera opened successfully";
        
        // Update camera settings
        updateCameraSettings();
        
        m_connected = true;
        updateStatus("Camera connected successfully");
        
        return true;
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error connecting to camera:" << e.GetDescription();
        updateStatus("Connection failed");
        m_connected = false;
        if (m_camera) {
            delete m_camera;
            m_camera = nullptr;
        }
        return false;
    }
}

void BaslerCamera::disconnect()
{
    qDebug() << "[BaslerCamera] Disconnecting camera...";
    
    stopGrabbing();
    
    if (m_camera) {
        try {
            if (m_camera->IsOpen()) {
                m_camera->Close();
            }
            delete m_camera;
            m_camera = nullptr;
        }
        catch (const GenericException& e) {
            qDebug() << "[BaslerCamera] Error closing camera:" << e.GetDescription();
        }
    }
    
    m_connected = false;
    updateStatus("Camera disconnected");
}

void BaslerCamera::startGrabbing()
{
    if (!m_camera || !m_camera->IsOpen()) {
        qDebug() << "[BaslerCamera] Camera not open, cannot start grabbing";
        return;
    }
    
    if (m_grabFlag) {
        qDebug() << "[BaslerCamera] Already grabbing";
        return;
    }
    
    try {
        m_grabFlag = true;
        m_grabThread = new std::thread(&BaslerCamera::grabLoop, this);
        
        // Reset frame rate measurement
        resetFrameRateMeasurement();
        
        qDebug() << "[BaslerCamera] Grabbing started";
        updateStatus("Grabbing started");
    }
    catch (const std::exception& e) {
        qDebug() << "[BaslerCamera] Error starting grabbing:" << e.what();
        m_grabFlag = false;
        updateStatus("Failed to start grabbing");
    }
}

void BaslerCamera::stopGrabbing()
{
    if (!m_grabFlag) {
        qDebug() << "[BaslerCamera] Not grabbing";
        return;
    }
    
    m_grabFlag = false;
    
    if (m_grabThread && m_grabThread->joinable()) {
        m_grabThread->join();
        delete m_grabThread;
        m_grabThread = nullptr;
    }
    
    qDebug() << "[BaslerCamera] Grabbing stopped";
    updateStatus("Grabbing stopped");
}

void BaslerCamera::grabLoop()
{
    qDebug() << "[BaslerCamera] Grab loop started";
    
    // Start continuous grabbing
    try {
        m_camera->StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByUser);
        qDebug() << "[BaslerCamera] Continuous grabbing started";
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error starting continuous grabbing:" << e.GetDescription();
        return;
    }
    
    while (m_grabFlag) {
        try {
            // Retrieve result with shorter timeout for continuous grabbing
            if (m_camera->RetrieveResult(100, m_grabResult, TimeoutHandling_Return)) {
                if (m_grabResult->GrabSucceeded()) {
                    // Update current frame ID
                    m_currentFrameId = m_grabResult->GetID();
                    
                    // Increment frame count immediately after successful grab
                    {
                        std::lock_guard<std::mutex> lock(m_frameRateMutex);
                        m_frameCount++;
                        
                        // Start timer on first frame
                        if (m_frameCount == 1) {
                            m_frameRateTimer.start();
                        }
                    }
                    
                    qDebug() << "[BaslerCamera Grab] Frame ID:" << m_grabResult->GetID() << "Count:" << m_frameCount;

                    // Convert image to OpenCV format
                    cv::Mat image;
                    convertBaslerImageToOpenCV(m_grabResult, image);

                    // Update current image
                    {
                        std::lock_guard<std::mutex> lock(m_imageMutex);
                        m_currentImage = image.clone();
                    }
                    
                    // Save image if recording is enabled
                    if (m_recordingEnabled && !image.empty()) {
                        // Create directory if it doesn't exist
                        QDir dir(m_recordingPath);
                        if (!dir.exists()) {
                            dir.mkpath(".");
                        }
                        
                        // Generate filename with pattern_XX.bmp format
                        QString filename = QString("%1/pattern_%2.bmp")
                                         .arg(m_recordingPath)
                                         .arg(m_recordedImageCount, 2, 10, QChar('0')); // 2 digits, zero-padded
                        
                        // Save image
                        if (cv::imwrite(filename.toStdString(), image)) {
                            m_recordedImageCount++;
                            qDebug() << "[BaslerCamera] Image saved:" << filename << "Total saved:" << m_recordedImageCount;
                            
                            // Check if max count reached and reset if needed
                            if (m_recordedImageCount >= m_maxRecordedImages) {
                                qDebug() << "[BaslerCamera] Max count reached (" << m_maxRecordedImages << "), resetting count to 0";
                                m_recordedImageCount = 0;
                            }
                        } else {
                            qDebug() << "[BaslerCamera] Failed to save image:" << filename;
                        }
                    }
                    
                    // Emit image updated signal
                    emit imageUpdated();
                    
                    // Emit frame ID updated signal
                    emit frameIdUpdated(m_currentFrameId);
                    
                    // Update real-time frame rate after image is processed and emitted
                    updateRealTimeFrameRate();
                } else {
                    // Increment error count
                    m_errorsCount++;
                    emit errorsCountUpdated(m_errorsCount);
                    
                    qDebug() << "[BaslerCamera] Grab failed:" << m_grabResult->GetErrorDescription();
                }
            }
            // Note: No else clause here as RetrieveResult returns false on timeout, which is normal
        }
        catch (const GenericException& e) {
            qDebug() << "[BaslerCamera] Error in grab loop:" << e.GetDescription();
            break;
        }
    }
    
    // Stop continuous grabbing
    try {
        m_camera->StopGrabbing();
        qDebug() << "[BaslerCamera] Continuous grabbing stopped";
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error stopping continuous grabbing:" << e.GetDescription();
    }
    
    qDebug() << "[BaslerCamera] Grab loop ended";
}

cv::Mat BaslerCamera::getImage()
{
    std::lock_guard<std::mutex> lock(m_imageMutex);
    return m_currentImage.clone();
}

QString BaslerCamera::getCameraInfo() const
{
    if (!m_connected) {
        return "Camera not connected";
    }
    
    QString info = QString("Name: %1\nModel: %2\nSerial: %3")
                   .arg(m_cameraName.isEmpty() ? "Unknown" : m_cameraName)
                   .arg(m_cameraModel.isEmpty() ? "Unknown" : m_cameraModel)
                   .arg(m_cameraSerial.isEmpty() ? "Unknown" : m_cameraSerial);
    
    return info;
}

void BaslerCamera::updateStatus(const QString &status)
{
    emit statusChanged(status);
} 

void BaslerCamera::updateCameraSettings()
{
    if (!m_camera || !m_camera->IsOpen()) {
        qDebug() << "[BaslerCamera] Camera not open, cannot get settings";
        return;
    }
    
    try {
        // Get width and height
        CIntegerParameter widthParam(m_camera->GetNodeMap(), "Width");
        CIntegerParameter heightParam(m_camera->GetNodeMap(), "Height");
        
        m_width = widthParam.GetValue();
        m_height = heightParam.GetValue();
        
        qDebug() << "[BaslerCamera] Resolution:" << m_width << "x" << m_height;
        
        // Get FPS (AcquisitionFrameRate)
        try {
            CFloatParameter fpsParam(m_camera->GetNodeMap(), "AcquisitionFrameRate");
            m_fps = fpsParam.GetValue();
            qDebug() << "[BaslerCamera] FPS:" << m_fps;
        }
        catch (const GenericException& e) {
            qDebug() << "[BaslerCamera] Could not get FPS:" << e.GetDescription();
            m_fps = 0.0;
        }
        
        // Get scaling factor
        try {
            CFloatParameter scalingParam(m_camera->GetNodeMap(), "ScalingFactor");
            m_scalingFactor = scalingParam.GetValue();
            qDebug() << "[BaslerCamera] Scaling Factor:" << m_scalingFactor;
        }
        catch (const GenericException& e) {
            qDebug() << "[BaslerCamera] Could not get Scaling Factor:" << e.GetDescription();
            m_scalingFactor = 1.0;
        }
        
        // Get exposure time
        try {
            CFloatParameter exposureParam(m_camera->GetNodeMap(), "ExposureTime");
            m_exposureTime = exposureParam.GetValue();
            qDebug() << "[BaslerCamera] Exposure Time:" << m_exposureTime << "μs";
        }
        catch (const GenericException& e) {
            qDebug() << "[BaslerCamera] Could not get Exposure Time:" << e.GetDescription();
            m_exposureTime = 10000.0;
        }
        
        // Get exposure auto
        try {
            CEnumParameter exposureAutoParam(m_camera->GetNodeMap(), "ExposureAuto");
            m_exposureAuto = (exposureAutoParam.GetValue() == "Continuous");
            qDebug() << "[BaslerCamera] Exposure Auto:" << (m_exposureAuto ? "On" : "Off");
        }
        catch (const GenericException& e) {
            qDebug() << "[BaslerCamera] Could not get Exposure Auto:" << e.GetDescription();
            m_exposureAuto = false;
        }
        
        // Get frame rate enable
        try {
            CBooleanParameter frameRateEnableParam(m_camera->GetNodeMap(), "AcquisitionFrameRateEnable");
            m_frameRateEnabled = frameRateEnableParam.GetValue();
            qDebug() << "[BaslerCamera] Frame Rate Enable:" << (m_frameRateEnabled ? "On" : "Off");
        }
        catch (const GenericException& e) {
            qDebug() << "[BaslerCamera] Could not get Frame Rate Enable:" << e.GetDescription();
            m_frameRateEnabled = false;
        }
        
        // Get frame rate
        try {
            CFloatParameter frameRateParam(m_camera->GetNodeMap(), "AcquisitionFrameRate");
            m_frameRate = frameRateParam.GetValue();
            qDebug() << "[BaslerCamera] Frame Rate:" << m_frameRate << "fps";
        }
        catch (const GenericException& e) {
            qDebug() << "[BaslerCamera] Could not get Frame Rate:" << e.GetDescription();
            m_frameRate = 30.0;
        }
        
        // Get trigger mode
        try {
            CEnumParameter triggerModeParam(m_camera->GetNodeMap(), "TriggerMode");
            m_triggerMode = QString::fromUtf8(triggerModeParam.GetValue().c_str());
            m_triggerEnabled = (m_triggerMode != "Off");
            qDebug() << "[BaslerCamera] Trigger Mode:" << m_triggerMode;
        }
        catch (const GenericException& e) {
            qDebug() << "[BaslerCamera] Could not get Trigger Mode:" << e.GetDescription();
            m_triggerMode = "Off";
            m_triggerEnabled = false;
        }
        
        // Get trigger source
        try {
            CEnumParameter triggerSourceParam(m_camera->GetNodeMap(), "TriggerSource");
            m_triggerSource = QString::fromUtf8(triggerSourceParam.GetValue().c_str());
            qDebug() << "[BaslerCamera] Trigger Source:" << m_triggerSource;
        }
        catch (const GenericException& e) {
            qDebug() << "[BaslerCamera] Could not get Trigger Source:" << e.GetDescription();
            m_triggerSource = "Software";
        }
        
        // Get trigger delay
        try {
            CFloatParameter triggerDelayParam(m_camera->GetNodeMap(), "TriggerDelay");
            m_triggerDelay = triggerDelayParam.GetValue();
            qDebug() << "[BaslerCamera] Trigger Delay:" << m_triggerDelay << "μs";
        }
        catch (const GenericException& e) {
            qDebug() << "[BaslerCamera] Could not get Trigger Delay:" << e.GetDescription();
            m_triggerDelay = 0.0;
        }
        
        updateStatus(QString("Settings: %1x%2 @ %3 FPS, Scale: %4, Exp: %5 μs, FR: %6, Trig: %7").arg(m_width).arg(m_height).arg(m_fps, 0, 'f', 1).arg(m_scalingFactor, 0, 'f', 2).arg(m_exposureTime, 0, 'f', 0).arg(m_frameRateEnabled ? "Fixed" : "Auto").arg(m_triggerEnabled ? m_triggerMode : "Off"));
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error getting camera settings:" << e.GetDescription();
        m_width = 0;
        m_height = 0;
        m_fps = 0.0;
        m_scalingFactor = 1.0;
        m_exposureTime = 10000.0;
        m_exposureAuto = false;
        m_frameRateEnabled = false;
        m_frameRate = 30.0;
    }
}

int BaslerCamera::getWidth() const
{
    return m_width;
}

int BaslerCamera::getHeight() const
{
    return m_height;
}

double BaslerCamera::getFPS() const
{
    return m_fps;
}

QString BaslerCamera::getCurrentSettings() const
{
    if (!m_connected) {
        return "Camera not connected";
    }
    
    QString settings = QString("Resolution: %1 x %2\nFPS: %3\nScaling Factor: %4\nExposure: %5 μs (%6)\nFrame Rate: %7 (%8)")
                       .arg(m_width)
                       .arg(m_height)
                       .arg(m_fps, 0, 'f', 1)
                       .arg(m_scalingFactor, 0, 'f', 2)
                       .arg(m_exposureTime, 0, 'f', 0)
                       .arg(m_exposureAuto ? "Auto" : "Manual")
                       .arg(m_frameRate, 0, 'f', 1)
                       .arg(m_frameRateEnabled ? "Fixed" : "Auto");
    
    return settings;
} 

bool BaslerCamera::setResolution(int width, int height)
{
    if (!m_camera || !m_camera->IsOpen()) {
        qDebug() << "[BaslerCamera] Camera not open, cannot set resolution";
        return false;
    }
    
    try {
        // Stop grabbing if active
        bool wasGrabbing = false;
        if (m_grabFlag) {
            stopGrabbing();
            wasGrabbing = true;
        }
        
        // Set width
        CIntegerParameter widthParam(m_camera->GetNodeMap(), "Width");
        widthParam.SetValue(width);
        
        // Set height
        CIntegerParameter heightParam(m_camera->GetNodeMap(), "Height");
        heightParam.SetValue(height);
        
        // Update stored values
        m_width = width;
        m_height = height;
        
        qDebug() << "[BaslerCamera] Resolution set to:" << m_width << "x" << m_height;
        
        // Restart grabbing if it was active
        if (wasGrabbing) {
            startGrabbing();
        }
        
        // Emit settings changed signal
        emit settingsChanged();
        
        updateStatus(QString("Resolution changed to: %1x%2").arg(width).arg(height));
        return true;
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error setting resolution:" << e.GetDescription();
        updateStatus("Failed to set resolution");
        return false;
    }
}

QStringList BaslerCamera::getAvailableResolutions() const
{
    QStringList resolutions;
    
    if (!m_camera || !m_camera->IsOpen()) {
        return resolutions;
    }
    
    try {
        // Get width range
        CIntegerParameter widthParam(m_camera->GetNodeMap(), "Width");
        int64_t widthMin = widthParam.GetMin();
        int64_t widthMax = widthParam.GetMax();
        int64_t widthInc = widthParam.GetInc();
        
        // Get height range
        CIntegerParameter heightParam(m_camera->GetNodeMap(), "Height");
        int64_t heightMin = heightParam.GetMin();
        int64_t heightMax = heightParam.GetMax();
        int64_t heightInc = heightParam.GetInc();
        
        // Add some common resolutions within the range
        QList<QPair<int, int>> commonResolutions = {
            {1920, 1200}
        };
        
        for (const auto& res : commonResolutions) {
            int w = res.first;
            int h = res.second;
            
            // Check if resolution is within camera's supported range
            if (w >= widthMin && w <= widthMax && h >= heightMin && h <= heightMax) {
                // Check if resolution is aligned with increment
                if ((w - widthMin) % widthInc == 0 && (h - heightMin) % heightInc == 0) {
                    resolutions.append(QString("%1 x %2").arg(w).arg(h));
                }
            }
        }
        
        // Add current resolution if not in list
        QString currentRes = QString("%1 x %2").arg(m_width).arg(m_height);
        if (!resolutions.contains(currentRes)) {
            resolutions.prepend(currentRes + " (Current)");
        }
        
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error getting available resolutions:" << e.GetDescription();
    }
    
    return resolutions;
} 

double BaslerCamera::getScalingFactor() const
{
    return m_scalingFactor;
}

bool BaslerCamera::setScalingFactor(double factor)
{
    if (!m_camera || !m_camera->IsOpen()) {
        qDebug() << "[BaslerCamera] Camera not open, cannot set scaling factor";
        return false;
    }
    
    try {
        // Stop grabbing if active
        bool wasGrabbing = false;
        if (m_grabFlag) {
            stopGrabbing();
            wasGrabbing = true;
        }
        
        // Set scaling factor
        CFloatParameter scalingParam(m_camera->GetNodeMap(), "ScalingFactor");
        scalingParam.SetValue(factor);
        
        // Update stored value
        m_scalingFactor = factor;
        
        qDebug() << "[BaslerCamera] Scaling factor set to:" << m_scalingFactor;
        
        // Restart grabbing if it was active
        if (wasGrabbing) {
            startGrabbing();
        }
        
        // Emit settings changed signal
        emit settingsChanged();
        
        updateStatus(QString("Scaling factor changed to: %1").arg(factor, 0, 'f', 2));
        return true;
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error setting scaling factor:" << e.GetDescription();
        updateStatus("Failed to set scaling factor");
        return false;
    }
}

double BaslerCamera::getMinScalingFactor() const
{
    if (!m_camera || !m_camera->IsOpen()) {
        return 1.0;
    }
    
    try {
        CFloatParameter scalingParam(m_camera->GetNodeMap(), "ScalingFactor");
        return scalingParam.GetMin();
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error getting min scaling factor:" << e.GetDescription();
        return 1.0;
    }
}

double BaslerCamera::getMaxScalingFactor() const
{
    if (!m_camera || !m_camera->IsOpen()) {
        return 1.0;
    }
    
    try {
        CFloatParameter scalingParam(m_camera->GetNodeMap(), "ScalingFactor");
        return scalingParam.GetMax();
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error getting max scaling factor:" << e.GetDescription();
        return 1.0;
    }
}

double BaslerCamera::getScalingFactorIncrement() const
{
    if (!m_camera || !m_camera->IsOpen()) {
        return 0.1;
    }
    
    try {
        CFloatParameter scalingParam(m_camera->GetNodeMap(), "ScalingFactor");
        return scalingParam.GetInc();
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error getting scaling factor increment:" << e.GetDescription();
        return 0.1;
    }
} 

double BaslerCamera::getExposureTime() const
{
    return m_exposureTime;
}

bool BaslerCamera::setExposureTime(double exposureTime)
{
    if (!m_camera || !m_camera->IsOpen()) {
        qDebug() << "[BaslerCamera] Camera not open, cannot set exposure time";
        return false;
    }
    
    try {
        // Stop grabbing if active
        bool wasGrabbing = false;
        if (m_grabFlag) {
            stopGrabbing();
            wasGrabbing = true;
        }
        
        // Set exposure time
        CFloatParameter exposureParam(m_camera->GetNodeMap(), "ExposureTime");
        exposureParam.SetValue(exposureTime);
        
        // Update stored value
        m_exposureTime = exposureTime;
        
        qDebug() << "[BaslerCamera] Exposure time set to:" << m_exposureTime << "μs";
        
        // Restart grabbing if it was active
        if (wasGrabbing) {
            startGrabbing();
        }
        
        // Emit settings changed signal
        emit settingsChanged();
        
        updateStatus(QString("Exposure time changed to: %1 μs").arg(exposureTime, 0, 'f', 0));
        return true;
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error setting exposure time:" << e.GetDescription();
        updateStatus("Failed to set exposure time");
        return false;
    }
}

double BaslerCamera::getMinExposureTime() const
{
    if (!m_camera || !m_camera->IsOpen()) {
        return 1000.0;
    }
    
    try {
        CFloatParameter exposureParam(m_camera->GetNodeMap(), "ExposureTime");
        return exposureParam.GetMin();
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error getting min exposure time:" << e.GetDescription();
        return 1000.0;
    }
}

double BaslerCamera::getMaxExposureTime() const
{
    if (!m_camera || !m_camera->IsOpen()) {
        return 1000000.0;
    }
    
    try {
        CFloatParameter exposureParam(m_camera->GetNodeMap(), "ExposureTime");
        return exposureParam.GetMax();
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error getting max exposure time:" << e.GetDescription();
        return 1000000.0;
    }
}

double BaslerCamera::getExposureTimeIncrement() const
{
    if (!m_camera || !m_camera->IsOpen()) {
        return 100.0;
    }
    
    try {
        CFloatParameter exposureParam(m_camera->GetNodeMap(), "ExposureTime");
        return exposureParam.GetInc();
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error getting exposure time increment:" << e.GetDescription();
        return 100.0;
    }
}

bool BaslerCamera::isExposureAuto() const
{
    return m_exposureAuto;
}

bool BaslerCamera::setExposureAuto(bool enable)
{
    if (!m_camera || !m_camera->IsOpen()) {
        qDebug() << "[BaslerCamera] Camera not open, cannot set exposure auto";
        return false;
    }
    
    try {
        // Stop grabbing if active
        bool wasGrabbing = false;
        if (m_grabFlag) {
            stopGrabbing();
            wasGrabbing = true;
        }
        
        // Set exposure auto
        CEnumParameter exposureAutoParam(m_camera->GetNodeMap(), "ExposureAuto");
        exposureAutoParam.SetValue(enable ? "Continuous" : "Off");
        
        // Update stored value
        m_exposureAuto = enable;
        
        qDebug() << "[BaslerCamera] Exposure auto set to:" << (enable ? "On" : "Off");
        
        // Restart grabbing if it was active
        if (wasGrabbing) {
            startGrabbing();
        }
        
        // Emit settings changed signal
        emit settingsChanged();
        
        updateStatus(QString("Exposure auto changed to: %1").arg(enable ? "On" : "Off"));
        return true;
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error setting exposure auto:" << e.GetDescription();
        updateStatus("Failed to set exposure auto");
        return false;
    }
} 

bool BaslerCamera::isFrameRateEnabled() const
{
    return m_frameRateEnabled;
}

bool BaslerCamera::setFrameRateEnabled(bool enable)
{
    if (!m_camera || !m_camera->IsOpen()) {
        qDebug() << "[BaslerCamera] Camera not open, cannot set frame rate enable";
        return false;
    }
    
    try {
        // Stop grabbing if active
        bool wasGrabbing = false;
        if (m_grabFlag) {
            stopGrabbing();
            wasGrabbing = true;
        }
        
        // Set frame rate enable
        CBooleanParameter frameRateEnableParam(m_camera->GetNodeMap(), "AcquisitionFrameRateEnable");
        frameRateEnableParam.SetValue(enable);
        
        // Update stored value
        m_frameRateEnabled = enable;
        
        qDebug() << "[BaslerCamera] Frame rate enable set to:" << (enable ? "On" : "Off");
        
        // Restart grabbing if it was active
        if (wasGrabbing) {
            startGrabbing();
        }
        
        // Emit settings changed signal
        emit settingsChanged();
        
        updateStatus(QString("Frame rate enable changed to: %1").arg(enable ? "On" : "Off"));
        return true;
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error setting frame rate enable:" << e.GetDescription();
        updateStatus("Failed to set frame rate enable");
        return false;
    }
}

double BaslerCamera::getFrameRate() const
{
    return m_frameRate;
}

bool BaslerCamera::setFrameRate(double frameRate)
{
    if (!m_camera || !m_camera->IsOpen()) {
        qDebug() << "[BaslerCamera] Camera not open, cannot set frame rate";
        return false;
    }
    
    try {
        // Stop grabbing if active
        bool wasGrabbing = false;
        if (m_grabFlag) {
            stopGrabbing();
            wasGrabbing = true;
        }
        
        // Set frame rate
        CFloatParameter frameRateParam(m_camera->GetNodeMap(), "AcquisitionFrameRate");
        frameRateParam.SetValue(frameRate);
        
        // Update stored value
        m_frameRate = frameRate;
        
        qDebug() << "[BaslerCamera] Frame rate set to:" << m_frameRate << "fps";
        
        // Restart grabbing if it was active
        if (wasGrabbing) {
            startGrabbing();
        }
        
        // Emit settings changed signal
        emit settingsChanged();
        
        updateStatus(QString("Frame rate changed to: %1 fps").arg(frameRate, 0, 'f', 1));
        return true;
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error setting frame rate:" << e.GetDescription();
        updateStatus("Failed to set frame rate");
        return false;
    }
}

double BaslerCamera::getMinFrameRate() const
{
    if (!m_camera || !m_camera->IsOpen()) {
        return 1.0;
    }
    
    try {
        CFloatParameter frameRateParam(m_camera->GetNodeMap(), "AcquisitionFrameRate");
        return frameRateParam.GetMin();
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error getting min frame rate:" << e.GetDescription();
        return 1.0;
    }
}

double BaslerCamera::getMaxFrameRate() const
{
    if (!m_camera || !m_camera->IsOpen()) {
        return 100.0;
    }
    
    try {
        CFloatParameter frameRateParam(m_camera->GetNodeMap(), "AcquisitionFrameRate");
        return frameRateParam.GetMax();
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error getting max frame rate:" << e.GetDescription();
        return 100.0;
    }
}

double BaslerCamera::getFrameRateIncrement() const
{
    if (!m_camera || !m_camera->IsOpen()) {
        return 0.1;
    }
    
    try {
        CFloatParameter frameRateParam(m_camera->GetNodeMap(), "AcquisitionFrameRate");
        return frameRateParam.GetInc();
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error getting frame rate increment:" << e.GetDescription();
        return 0.1;
    }
} 

void BaslerCamera::updateRealTimeFrameRate()
{
    std::lock_guard<std::mutex> lock(m_frameRateMutex);
    
    // Get current time
    qint64 currentTime = m_lastFrameTimer.nsecsElapsed();
    double currentTimeMs = currentTime / 1000000.0; // Convert to milliseconds
    
    // Start timer on first frame
    if (m_frameCount == 1) {
        m_lastFrameTimer.start();
        m_lastFrameTime = currentTimeMs;
        return;
    }
    
    // Calculate frame interval
    double frameInterval = currentTimeMs - m_lastFrameTime;
    m_lastFrameTime = currentTimeMs;
    
    // Add interval to our moving average buffer
    m_frameIntervals.append(frameInterval);
    
    // Keep only the last MAX_INTERVALS intervals
    if (m_frameIntervals.size() > MAX_INTERVALS) {
        m_frameIntervals.removeFirst();
    }
    
    // Calculate average frame interval
    double avgInterval = 0.0;
    if (!m_frameIntervals.isEmpty()) {
        for (double interval : m_frameIntervals) {
            avgInterval += interval;
        }
        avgInterval /= m_frameIntervals.size();
    }
    
    // Calculate frame rate from average interval
    if (avgInterval > 0) {
        m_realTimeFrameRate = 1000.0 / avgInterval; // Convert to fps
    }
    
    // Get camera's configured frame rate for comparison
    double configuredFrameRate = 0.0;
    try {
        if (m_camera && m_camera->IsOpen()) {
            CFloatParameter fpsParam(m_camera->GetNodeMap(), "AcquisitionFrameRate");
            configuredFrameRate = fpsParam.GetValue();
        }
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error getting configured frame rate:" << e.GetDescription();
    }
    
    // Emit frame rate updated signal every few frames
    if (m_frameCount % 3 == 0) { // Update every 3 frames for more responsive UI
        emit frameRateUpdated(m_realTimeFrameRate);
        
        qDebug() << "[BaslerCamera] Real-time frame rate:" << m_realTimeFrameRate 
                 << "fps (configured:" << configuredFrameRate 
                 << "fps, avg interval:" << avgInterval << "ms, current interval:" << frameInterval << "ms)";
    }
}

double BaslerCamera::getRealTimeFrameRate() const
{
    std::lock_guard<std::mutex> lock(m_frameRateMutex);
    return m_realTimeFrameRate;
}

int BaslerCamera::getFrameCount() const
{
    std::lock_guard<std::mutex> lock(m_frameRateMutex);
    return m_frameCount;
}

void BaslerCamera::resetFrameRateMeasurement()
{
    std::lock_guard<std::mutex> lock(m_frameRateMutex);
    m_frameCount = 0;
    m_realTimeFrameRate = 0.0;
    m_lastFrameTime = 0.0;
    m_frameIntervals.clear();
    m_frameRateTimer.invalidate();
    m_lastFrameTimer.invalidate();
}

int BaslerCamera::getCurrentFrameId() const
{
    return m_currentFrameId;
}

int BaslerCamera::getErrorsCount() const
{
    return m_errorsCount;
}

void BaslerCamera::convertBaslerImageToOpenCV(const CGrabResultPtr& grabResult, cv::Mat& image)
{
    if (!grabResult->GrabSucceeded()) {
        image = cv::Mat();
        return;
    }
    
    // Get image buffer and properties
    const uint8_t* pImageBuffer = (uint8_t*)grabResult->GetBuffer();
    int width = grabResult->GetWidth();
    int height = grabResult->GetHeight();

    if (width <= 0 || height <= 0 || pImageBuffer == nullptr) {
        image = cv::Mat();
        return;
    }
    
    // Handle different pixel formats
    EPixelType pixelType = grabResult->GetPixelType();
    
    switch (pixelType) {
        case PixelType_Mono8:
            image = cv::Mat(height, width, CV_8UC1, (void*)pImageBuffer);
            cv::cvtColor(image, image, cv::COLOR_GRAY2BGR);
            break;
            
        case PixelType_RGB8packed:
            image = cv::Mat(height, width, CV_8UC3, (void*)pImageBuffer);
            cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
            break;
            
        case PixelType_BGR8packed:
            image = cv::Mat(height, width, CV_8UC3, (void*)pImageBuffer);
            break;
            
        case PixelType_Mono12:
        case PixelType_Mono16:
            // Convert to 8-bit for display
            image = cv::Mat(height, width, CV_16UC1, (void*)pImageBuffer);
            image.convertTo(image, CV_8UC1, 255.0 / 65535.0);
            cv::cvtColor(image, image, cv::COLOR_GRAY2BGR);
            break;
            
        default:
            qDebug() << "[BaslerCamera] Unsupported pixel format:" << pixelType;
            // Try to handle as RGB8
            image = cv::Mat(height, width, CV_8UC3, (void*)pImageBuffer);
            cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
            break;
    }
} 

// Trigger control methods
bool BaslerCamera::isTriggerEnabled() const
{
    return m_triggerEnabled;
}

bool BaslerCamera::setTriggerEnabled(bool enable)
{
    if (!m_camera || !m_camera->IsOpen()) {
        qDebug() << "[BaslerCamera] Camera not open, cannot set trigger enabled";
        return false;
    }
    
    try {
        // Stop grabbing before changing trigger settings
        bool wasGrabbing = m_grabFlag.load();
        if (wasGrabbing) {
            stopGrabbing();
        }
        
        CEnumParameter triggerModeParam(m_camera->GetNodeMap(), "TriggerMode");
        triggerModeParam.SetValue(enable ? "On" : "Off");
        
        m_triggerEnabled = enable;
        m_triggerMode = enable ? "On" : "Off";
        
        qDebug() << "[BaslerCamera] Trigger enabled set to:" << enable;
        
        // Restart grabbing if it was running
        if (wasGrabbing) {
            startGrabbing();
        }
        
        emit settingsChanged();
        return true;
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error setting trigger enabled:" << e.GetDescription();
        return false;
    }
}

QString BaslerCamera::getTriggerMode() const
{
    return m_triggerMode;
}

bool BaslerCamera::setTriggerMode(const QString &mode)
{
    if (!m_camera || !m_camera->IsOpen()) {
        qDebug() << "[BaslerCamera] Camera not open, cannot set trigger mode";
        return false;
    }
    
    try {
        // Stop grabbing before changing trigger settings
        bool wasGrabbing = m_grabFlag.load();
        if (wasGrabbing) {
            stopGrabbing();
        }
        
        CEnumParameter triggerModeParam(m_camera->GetNodeMap(), "TriggerMode");
        triggerModeParam.SetValue(mode.toUtf8().constData());
        
        m_triggerMode = mode;
        m_triggerEnabled = (mode != "Off");
        
        qDebug() << "[BaslerCamera] Trigger mode set to:" << mode;
        
        // Restart grabbing if it was running
        if (wasGrabbing) {
            startGrabbing();
        }
        
        emit settingsChanged();
        return true;
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error setting trigger mode:" << e.GetDescription();
        return false;
    }
}

QStringList BaslerCamera::getAvailableTriggerModes() const
{
    QStringList modes;
    modes << "Off" << "On";
    return modes;
}

QString BaslerCamera::getTriggerSource() const
{
    return m_triggerSource;
}

bool BaslerCamera::setTriggerSource(const QString &source)
{
    if (!m_camera || !m_camera->IsOpen()) {
        qDebug() << "[BaslerCamera] Camera not open, cannot set trigger source";
        return false;
    }
    
    try {
        // Stop grabbing before changing trigger settings
        bool wasGrabbing = m_grabFlag.load();
        if (wasGrabbing) {
            stopGrabbing();
        }
        
        CEnumParameter triggerSourceParam(m_camera->GetNodeMap(), "TriggerSource");
        triggerSourceParam.SetValue(source.toUtf8().constData());
        
        m_triggerSource = source;
        
        qDebug() << "[BaslerCamera] Trigger source set to:" << source;
        
        // Restart grabbing if it was running
        if (wasGrabbing) {
            startGrabbing();
        }
        
        emit settingsChanged();
        return true;
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error setting trigger source:" << e.GetDescription();
        return false;
    }
}

QStringList BaslerCamera::getAvailableTriggerSources() const
{
    QStringList sources;
    sources << "Software" << "Line1" << "Line2" << "Line3" << "Line4";
    return sources;
}

double BaslerCamera::getTriggerDelay() const
{
    return m_triggerDelay;
}

bool BaslerCamera::setTriggerDelay(double delay)
{
    if (!m_camera || !m_camera->IsOpen()) {
        qDebug() << "[BaslerCamera] Camera not open, cannot set trigger delay";
        return false;
    }
    
    try {
        // Stop grabbing before changing trigger settings
        bool wasGrabbing = m_grabFlag.load();
        if (wasGrabbing) {
            stopGrabbing();
        }
        
        CFloatParameter triggerDelayParam(m_camera->GetNodeMap(), "TriggerDelay");
        triggerDelayParam.SetValue(delay);
        
        m_triggerDelay = delay;
        
        qDebug() << "[BaslerCamera] Trigger delay set to:" << delay << "μs";
        
        // Restart grabbing if it was running
        if (wasGrabbing) {
            startGrabbing();
        }
        
        emit settingsChanged();
        return true;
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error setting trigger delay:" << e.GetDescription();
        return false;
    }
}

double BaslerCamera::getMinTriggerDelay() const
{
    if (!m_camera || !m_camera->IsOpen()) {
        return 0.0;
    }
    
    try {
        CFloatParameter triggerDelayParam(m_camera->GetNodeMap(), "TriggerDelay");
        return triggerDelayParam.GetMin();
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error getting min trigger delay:" << e.GetDescription();
        return 0.0;
    }
}

double BaslerCamera::getMaxTriggerDelay() const
{
    if (!m_camera || !m_camera->IsOpen()) {
        return 1000000.0; // 1 second default
    }
    
    try {
        CFloatParameter triggerDelayParam(m_camera->GetNodeMap(), "TriggerDelay");
        return triggerDelayParam.GetMax();
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error getting max trigger delay:" << e.GetDescription();
        return 1000000.0;
    }
}

double BaslerCamera::getTriggerDelayIncrement() const
{
    if (!m_camera || !m_camera->IsOpen()) {
        return 1.0;
    }
    
    try {
        CFloatParameter triggerDelayParam(m_camera->GetNodeMap(), "TriggerDelay");
        return triggerDelayParam.GetInc();
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error getting trigger delay increment:" << e.GetDescription();
        return 1.0;
    }
} 

bool BaslerCamera::executeSoftwareTrigger()
{
    if (!m_camera || !m_camera->IsOpen()) {
        qDebug() << "[BaslerCamera] Camera not open, cannot execute software trigger";
        return false;
    }
    
    try {
        CCommandParameter triggerCommand(m_camera->GetNodeMap(), "TriggerSoftware");
        triggerCommand.Execute();
        qDebug() << "[BaslerCamera] Software trigger executed successfully";
        return true;
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error executing software trigger:" << e.GetDescription();
        return false;
    }
} 

// Image recording control methods
bool BaslerCamera::isRecordingEnabled() const
{
    return m_recordingEnabled;
}

void BaslerCamera::setRecordingEnabled(bool enable)
{
    m_recordingEnabled = enable;
    qDebug() << "[BaslerCamera] Recording enabled:" << enable;
}

void BaslerCamera::setRecordingPath(const QString &path)
{
    m_recordingPath = path;
    qDebug() << "[BaslerCamera] Recording path set to:" << path;
}

QString BaslerCamera::getRecordingPath() const
{
    return m_recordingPath;
}

int BaslerCamera::getRecordedImageCount() const
{
    return m_recordedImageCount;
}

void BaslerCamera::resetRecordingCount()
{
    m_recordedImageCount = 0;
    qDebug() << "[BaslerCamera] Recording count reset to 0";
}

void BaslerCamera::setMaxRecordedImages(int maxCount)
{
    if (maxCount > 0) {
        m_maxRecordedImages = maxCount;
        qDebug() << "[BaslerCamera] Max recorded images set to:" << maxCount;
    } else {
        qDebug() << "[BaslerCamera] Invalid max count:" << maxCount << "must be > 0";
    }
}

int BaslerCamera::getMaxRecordedImages() const
{
    return m_maxRecordedImages;
} 
