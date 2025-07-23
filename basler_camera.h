#ifndef BASLER_CAMERA_H
#define BASLER_CAMERA_H

#include <QObject>
#include <QDebug>
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

signals:
    void imageUpdated();
    void statusChanged(const QString &status);
    void settingsChanged();

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
    
    void grabLoop();
    void updateStatus(const QString &status);
    void updateCameraSettings();
};

#endif // BASLER_CAMERA_H 