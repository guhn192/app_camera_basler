#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QTextEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QCheckBox>
#include "basler_camera.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onGrabClicked();
    void onSetResolutionClicked();
    void onResolutionComboChanged(const QString &text);
    void onSetScalingFactorClicked();
    void onScalingFactorSliderChanged(int value);
    void onSetExposureTimeClicked();
    void onExposureTimeSliderChanged(int value);
    void onExposureAutoChanged(bool checked);
    void onSetFrameRateClicked();
    void onFrameRateSliderChanged(int value);
    void onFrameRateEnabledChanged(bool checked);
    void onFrameRateUpdated(double frameRate);
    void onFrameIdUpdated(int frameId);
    void onErrorsCountUpdated(int errorsCount);
    void updateImage();

private:
    BaslerCamera *baslerCamera;
    QTimer *updateTimer;
    
    // UI elements
    QLabel *imageLabel;
    QLabel *statusLabel;
    QTextEdit *cameraInfoLabel;
    QTextEdit *cameraSettingsLabel;
    QPushButton *connectButton;
    QPushButton *disconnectButton;
    QPushButton *grabButton;
    
    // Resolution control
    QSpinBox *widthSpinBox;
    QSpinBox *heightSpinBox;
    QPushButton *setResolutionButton;
    QComboBox *resolutionComboBox;
    
    // Scaling control
    QDoubleSpinBox *scalingFactorSpinBox;
    QSlider *scalingFactorSlider;
    QPushButton *setScalingFactorButton;
    QLabel *scalingFactorLabel;
    
    // Exposure control
    QDoubleSpinBox *exposureTimeSpinBox;
    QSlider *exposureTimeSlider;
    QPushButton *setExposureTimeButton;
    QLabel *exposureTimeLabel;
    QCheckBox *exposureAutoCheckBox;
    
    // Frame rate control
    QDoubleSpinBox *frameRateSpinBox;
    QSlider *frameRateSlider;
    QPushButton *setFrameRateButton;
    QLabel *frameRateLabel;
    QCheckBox *frameRateEnabledCheckBox;
    
    // Real-time frame rate display
    QLabel *realTimeFrameRateLabel;
    QLabel *frameCountLabel;
    QLabel *frameIdLabel;
    QLabel *errorsCountLabel;
    
    void setupUI();
    void updateStatus(const QString &status);
    void updateCameraInfo();
    void updateCameraSettings();
    void updateResolutionControls();
    void updateScalingControls();
    void updateExposureControls();
    void updateFrameRateControls();
    void updateRealTimeFrameRateDisplay();
};

#endif // MAINWINDOW_H
