#include "basler_camera.h"

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
    if (!m_connected || !m_camera) {
        qDebug() << "[BaslerCamera] Cannot start grabbing - camera not connected";
        return;
    }
    
    if (m_grabThread) {
        qDebug() << "[BaslerCamera] Grabbing already started";
        return;
    }
    
    qDebug() << "[BaslerCamera] Starting grabbing...";
    updateStatus("Starting grabbing...");
    
    m_grabFlag = true;
    m_grabThread = new std::thread(&BaslerCamera::grabLoop, this);
    
    qDebug() << "[BaslerCamera] Grabbing started successfully";
    updateStatus("Grabbing started");
}

void BaslerCamera::stopGrabbing()
{
    qDebug() << "[BaslerCamera] Stopping grabbing...";
    
    if (m_grabThread) {
        m_grabFlag = false;
        if (m_grabThread->joinable()) {
            m_grabThread->join();
        }
        delete m_grabThread;
        m_grabThread = nullptr;
    }
    
    updateStatus("Grabbing stopped");
}

void BaslerCamera::grabLoop()
{
    qDebug() << "[BaslerCamera] Grab loop started";
    
    try {
        if (m_camera == nullptr) {
            qDebug() << "[BaslerCamera] Camera is null in grab loop";
            return;
        }
        
        // Start grabbing
        m_camera->StartGrabbing(GrabStrategy_LatestImageOnly);
        
        while (m_grabFlag) {
            try {
                if (m_camera->RetrieveResult(5000, m_grabResult, TimeoutHandling_ThrowException)) {
                    if (m_grabResult->GrabSucceeded()) {
                        // Get image buffer and properties
                        const uint8_t* pImageBuffer = (uint8_t*)m_grabResult->GetBuffer();
                        int width = m_grabResult->GetWidth();
                        int height = m_grabResult->GetHeight();
                        
                        if (width > 0 && height > 0 && pImageBuffer != nullptr) {
                            cv::Mat img;
                            
                            // Handle different pixel formats
                            EPixelType pixelType = m_grabResult->GetPixelType();
                            
                            switch (pixelType) {
                                case PixelType_Mono8:
                                    img = cv::Mat(height, width, CV_8UC1, (void*)pImageBuffer);
                                    cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
                                    break;
                                    
                                case PixelType_RGB8packed:
                                    img = cv::Mat(height, width, CV_8UC3, (void*)pImageBuffer);
                                    cv::cvtColor(img, img, cv::COLOR_RGB2BGR);
                                    break;
                                    
                                case PixelType_BGR8packed:
                                    img = cv::Mat(height, width, CV_8UC3, (void*)pImageBuffer);
                                    break;
                                    
                                case PixelType_Mono12:
                                case PixelType_Mono16:
                                    // Convert to 8-bit for display
                                    img = cv::Mat(height, width, CV_16UC1, (void*)pImageBuffer);
                                    img.convertTo(img, CV_8UC1, 255.0 / 65535.0);
                                    cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
                                    break;
                                    
                                default:
                                    qDebug() << "[BaslerCamera] Unsupported pixel format:" << pixelType;
                                    // Try to handle as RGB8
                                    img = cv::Mat(height, width, CV_8UC3, (void*)pImageBuffer);
                                    cv::cvtColor(img, img, cv::COLOR_RGB2BGR);
                                    break;
                            }
                            
                            if (!img.empty()) {
                                // Update image with mutex protection
                                {
                                    std::lock_guard<std::mutex> lock(m_imageMutex);
                                    m_currentImage = img.clone();
                                }
                                
                                // Emit signal for UI update
                                emit imageUpdated();
                            }
                        }
                    }
                }
            }
            catch (const GenericException& e) {
                qDebug() << "[BaslerCamera] Error in grab loop:" << e.GetDescription();
                // Continue loop even if there's an error
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        // Stop grabbing
        m_camera->StopGrabbing();
        qDebug() << "[BaslerCamera] Grab loop stopped";
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error in grab loop:" << e.GetDescription();
    }
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
        
        updateStatus(QString("Settings: %1x%2 @ %3 FPS, Scale: %4").arg(m_width).arg(m_height).arg(m_fps, 0, 'f', 1).arg(m_scalingFactor, 0, 'f', 2));
    }
    catch (const GenericException& e) {
        qDebug() << "[BaslerCamera] Error getting camera settings:" << e.GetDescription();
        m_width = 0;
        m_height = 0;
        m_fps = 0.0;
        m_scalingFactor = 1.0;
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
    
    QString settings = QString("Resolution: %1 x %2\nFPS: %3\nScaling Factor: %4")
                       .arg(m_width)
                       .arg(m_height)
                       .arg(m_fps, 0, 'f', 1)
                       .arg(m_scalingFactor, 0, 'f', 2);
    
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