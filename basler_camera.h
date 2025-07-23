#ifndef BASLER_CAMERA_H
#define BASLER_CAMERA_H

#include <QObject>
#include <QDebug>
#include <QElapsedTimer>
#include <thread>
#include <atomic>
#include <mutex>
#include <opencv2/opencv.hpp>

// Basler Pylon includes
#include <pylon/PylonIncludes.h>

using namespace Pylon;

class BaslerCamera : public QObject
{
    Q_OBJECT

public:
    explicit BaslerCamera(QObject *parent = nullptr);
    ~BaslerCamera();

    bool connect();
    void disconnect();
    bool isConnected() const { return m_connected; }
    
    cv::Mat getImage();
    void startGrabbing();
    void stopGrabbing();
    
    // Camera information
    QString getCameraInfo() const;
    
    // Camera settings
    int getWidth() const;
    int getHeight() const;
    double getFPS() const;
    QString getCurrentSettings() const;
    
    // Camera control
    bool setResolution(int width, int height);
    QStringList getAvailableResolutions() const;
    
    // Scaling control
    double getScalingFactor() const;
    bool setScalingFactor(double factor);
    double getMinScalingFactor() const;
    double getMaxScalingFactor() const;
    double getScalingFactorIncrement() const;
    
    // Exposure control
    double getExposureTime() const;
    bool setExposureTime(double exposureTime);
    double getMinExposureTime() const;
    double getMaxExposureTime() const;
    double getExposureTimeIncrement() const;
    bool isExposureAuto() const;
    bool setExposureAuto(bool enable);
    
    // Frame rate control
    bool isFrameRateEnabled() const;
    bool setFrameRateEnabled(bool enable);
    double getFrameRate() const;
    bool setFrameRate(double frameRate);
    double getMinFrameRate() const;
    double getMaxFrameRate() const;
    double getFrameRateIncrement() const;
    
    // Real-time frame rate measurement
    double getRealTimeFrameRate() const;
    int getFrameCount() const;
    void resetFrameRateMeasurement();

signals:
    void imageUpdated();
    void statusChanged(const QString &status);
    void settingsChanged();
    void frameRateUpdated(double frameRate);

private:
    CInstantCamera* m_camera;
    CGrabResultPtr m_grabResult;
    
    std::thread* m_grabThread;
    std::atomic<bool> m_grabFlag;
    std::atomic<bool> m_connected;
    
    cv::Mat m_currentImage;
    std::mutex m_imageMutex;
    
    // Camera info
    QString m_cameraName;
    QString m_cameraModel;
    QString m_cameraSerial;
    
    // Camera settings
    int m_width;
    int m_height;
    double m_fps;
    double m_scalingFactor;
    double m_exposureTime;
    bool m_exposureAuto;
    bool m_frameRateEnabled;
    double m_frameRate;
    
    // Real-time frame rate measurement
    mutable std::mutex m_frameRateMutex;
    QElapsedTimer m_frameRateTimer;
    int m_frameCount;
    double m_realTimeFrameRate;
    static const int FRAME_RATE_WINDOW_SIZE = 30; // Number of frames to average
    
    void grabLoop();
    void updateStatus(const QString &status);
    void updateCameraSettings();
    void updateRealTimeFrameRate();
    void convertBaslerImageToOpenCV(const CGrabResultPtr& grabResult, cv::Mat& image);
};

#endif // BASLER_CAMERA_H 