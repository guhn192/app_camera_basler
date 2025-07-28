#include "mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QScrollArea>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , baslerCamera(new BaslerCamera(this))
    , updateTimer(new QTimer(this))
    , imageLabel(nullptr)
    , statusLabel(nullptr)
    , cameraInfoLabel(nullptr)
    , cameraSettingsLabel(nullptr)
    , connectButton(nullptr)
    , disconnectButton(nullptr)
    , grabButton(nullptr)
    , widthSpinBox(nullptr)
    , heightSpinBox(nullptr)
    , setResolutionButton(nullptr)
    , resolutionComboBox(nullptr)
    , scalingFactorSpinBox(nullptr)
    , scalingFactorSlider(nullptr)
    , setScalingFactorButton(nullptr)
    , scalingFactorLabel(nullptr)
    , exposureTimeSpinBox(nullptr)
    , exposureTimeSlider(nullptr)
    , setExposureTimeButton(nullptr)
    , exposureTimeLabel(nullptr)
    , exposureAutoCheckBox(nullptr)
    , frameRateSpinBox(nullptr)
    , frameRateSlider(nullptr)
    , setFrameRateButton(nullptr)
    , frameRateLabel(nullptr)
    , frameRateEnabledCheckBox(nullptr)
    , triggerEnabledCheckBox(nullptr)
    , triggerModeComboBox(nullptr)
    , triggerSourceComboBox(nullptr)
    , triggerDelaySpinBox(nullptr)
    , triggerDelaySlider(nullptr)
    , setTriggerDelayButton(nullptr)
    , triggerDelayLabel(nullptr)
    , softwareTriggerButton(nullptr)
    , recordingToggleButton(nullptr)
    , recordingStatusLabel(nullptr)
    , recordedImageCountLabel(nullptr)
    , resetRecordingCountButton(nullptr)
    , recordingPathEdit(nullptr)
    , setRecordingPathButton(nullptr)
    , maxRecordedImagesSpinBox(nullptr)
    , setMaxRecordedImagesButton(nullptr)
    , realTimeFrameRateLabel(nullptr)
    , frameCountLabel(nullptr)
    , frameIdLabel(nullptr)
    , errorsCountLabel(nullptr)
{
    setupUI();
    
    // Connect signals
    connect(baslerCamera, &BaslerCamera::imageUpdated, this, &MainWindow::updateImage);
    connect(baslerCamera, &BaslerCamera::statusChanged, this, &MainWindow::updateStatus);
    connect(baslerCamera, &BaslerCamera::settingsChanged, this, &MainWindow::updateCameraSettings);
    connect(baslerCamera, &BaslerCamera::frameRateUpdated, this, &MainWindow::onFrameRateUpdated);
    connect(baslerCamera, &BaslerCamera::frameIdUpdated, this, &MainWindow::onFrameIdUpdated);
    connect(baslerCamera, &BaslerCamera::errorsCountUpdated, this, &MainWindow::onErrorsCountUpdated);
    
    // Setup timer for periodic image updates
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateImage);
    updateTimer->start(10); // ~30 FPS
    
    updateStatus("Application started");
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // Create central widget and layout
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // Create scroll area for left panel
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setMaximumWidth(400); // Limit width of scroll area
    
    // Create widget for scroll area content
    QWidget *scrollContent = new QWidget();
    QVBoxLayout *leftPanel = new QVBoxLayout(scrollContent);
    
    // Create top layout for camera info and controls
    QHBoxLayout *topLayout = new QHBoxLayout();
    
    // Create camera info display
    cameraInfoLabel = new QTextEdit();
    cameraInfoLabel->setMaximumHeight(80);
    cameraInfoLabel->setReadOnly(true);
    cameraInfoLabel->setText("Camera not connected");
    cameraInfoLabel->setStyleSheet("QTextEdit { background-color: #f8f8f8; border: 1px solid #ccc; }");
    leftPanel->addWidget(new QLabel("Camera Information:"));
    leftPanel->addWidget(cameraInfoLabel);
    
    // Create camera settings display
    cameraSettingsLabel = new QTextEdit();
    cameraSettingsLabel->setMaximumHeight(60);
    cameraSettingsLabel->setReadOnly(true);
    cameraSettingsLabel->setText("Settings: Not available");
    cameraSettingsLabel->setStyleSheet("QTextEdit { background-color: #f0f8ff; border: 1px solid #ccc; }");
    leftPanel->addWidget(new QLabel("Camera Settings:"));
    leftPanel->addWidget(cameraSettingsLabel);
    
    // Create real-time frame rate display section
    QGroupBox *realTimeGroup = new QGroupBox("Real-time Frame Rate");
    QVBoxLayout *realTimeLayout = new QVBoxLayout(realTimeGroup);
    
    // Real-time frame rate label
    realTimeFrameRateLabel = new QLabel("Current FPS: 0.0");
    realTimeFrameRateLabel->setAlignment(Qt::AlignCenter);
    realTimeFrameRateLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; color: red; padding: 5px; background-color: #ffe6e6; border: 1px solid #ff9999; }");
    
    // Create horizontal layout for frame info
    QHBoxLayout *frameInfoLayout = new QHBoxLayout();
    
    // Frame count label
    frameCountLabel = new QLabel("Frame Count: 0");
    frameCountLabel->setAlignment(Qt::AlignCenter);
    frameCountLabel->setStyleSheet("QLabel");
    
    // Frame ID label
    frameIdLabel = new QLabel("Frame ID: 0");
    frameIdLabel->setAlignment(Qt::AlignCenter);
    frameIdLabel->setStyleSheet("QLabel");
    
    // Errors count label
    errorsCountLabel = new QLabel("Errors: 0");
    errorsCountLabel->setAlignment(Qt::AlignCenter);
    errorsCountLabel->setStyleSheet("QLabel");
    
    // Add frame info labels to horizontal layout
    frameInfoLayout->addWidget(frameCountLabel);
    frameInfoLayout->addWidget(frameIdLabel);
    frameInfoLayout->addWidget(errorsCountLabel);
    
    realTimeLayout->addWidget(realTimeFrameRateLabel);
    realTimeLayout->addLayout(frameInfoLayout);
    
    leftPanel->addWidget(realTimeGroup);
    
    // Create resolution control section
    QGroupBox *resolutionGroup = new QGroupBox("Resolution Control");
    QVBoxLayout *resolutionLayout = new QVBoxLayout(resolutionGroup);
    
    // Resolution combo box
    resolutionComboBox = new QComboBox();
    resolutionComboBox->addItem("Select resolution...");
    resolutionComboBox->setEnabled(false);
    resolutionLayout->addWidget(new QLabel("Preset Resolutions:"));
    resolutionLayout->addWidget(resolutionComboBox);
    
    // Custom resolution controls
    QHBoxLayout *customResLayout = new QHBoxLayout();
    
    widthSpinBox = new QSpinBox();
    widthSpinBox->setRange(1, 10000);
    widthSpinBox->setValue(640);
    widthSpinBox->setSuffix(" px");
    widthSpinBox->setEnabled(false);
    
    heightSpinBox = new QSpinBox();
    heightSpinBox->setRange(1, 10000);
    heightSpinBox->setValue(480);
    heightSpinBox->setSuffix(" px");
    heightSpinBox->setEnabled(false);
    
    customResLayout->addWidget(new QLabel("Width:"));
    customResLayout->addWidget(widthSpinBox);
    customResLayout->addWidget(new QLabel("Height:"));
    customResLayout->addWidget(heightSpinBox);
    
    resolutionLayout->addLayout(customResLayout);
    
    // Set resolution button
    setResolutionButton = new QPushButton("Set Resolution");
    setResolutionButton->setEnabled(false);
    resolutionLayout->addWidget(setResolutionButton);
    
    leftPanel->addWidget(resolutionGroup);
    
    // Create scaling control section
    QGroupBox *scalingGroup = new QGroupBox("Scaling Factor Control");
    QVBoxLayout *scalingLayout = new QVBoxLayout(scalingGroup);
    
    // Scaling factor spin box
    scalingFactorSpinBox = new QDoubleSpinBox();
    scalingFactorSpinBox->setRange(0.1, 10.0);
    scalingFactorSpinBox->setValue(1.0);
    scalingFactorSpinBox->setSingleStep(0.1);
    scalingFactorSpinBox->setDecimals(2);
    scalingFactorSpinBox->setSuffix("x");
    scalingFactorSpinBox->setEnabled(false);
    
    // Scaling factor slider
    scalingFactorSlider = new QSlider(Qt::Horizontal);
    scalingFactorSlider->setRange(10, 1000); // 0.1 to 10.0 * 100
    scalingFactorSlider->setValue(100); // 1.0
    scalingFactorSlider->setEnabled(false);
    
    // Scaling factor label
    scalingFactorLabel = new QLabel("Current: 1.00x");
    scalingFactorLabel->setAlignment(Qt::AlignCenter);
    scalingFactorLabel->setStyleSheet("QLabel { font-weight: bold; color: blue; }");
    
    // Scaling controls layout
    QHBoxLayout *scalingControlsLayout = new QHBoxLayout();
    scalingControlsLayout->addWidget(new QLabel("Scaling Factor:"));
    scalingControlsLayout->addWidget(scalingFactorSpinBox);
    
    scalingLayout->addLayout(scalingControlsLayout);
    scalingLayout->addWidget(scalingFactorSlider);
    scalingLayout->addWidget(scalingFactorLabel);
    
    // Set scaling factor button
    setScalingFactorButton = new QPushButton("Set Scaling Factor");
    setScalingFactorButton->setEnabled(false);
    scalingLayout->addWidget(setScalingFactorButton);
    
    leftPanel->addWidget(scalingGroup);
    
    // Create exposure control section
    QGroupBox *exposureGroup = new QGroupBox("Exposure Control");
    QVBoxLayout *exposureLayout = new QVBoxLayout(exposureGroup);
    
    // Exposure auto checkbox
    exposureAutoCheckBox = new QCheckBox("Auto Exposure");
    exposureAutoCheckBox->setEnabled(false);
    exposureLayout->addWidget(exposureAutoCheckBox);
    
    // Exposure time spin box
    exposureTimeSpinBox = new QDoubleSpinBox();
    exposureTimeSpinBox->setRange(1000, 1000000);
    exposureTimeSpinBox->setValue(10000);
    exposureTimeSpinBox->setSingleStep(100);
    exposureTimeSpinBox->setDecimals(0);
    exposureTimeSpinBox->setSuffix(" μs");
    exposureTimeSpinBox->setEnabled(false);
    
    // Exposure time slider
    exposureTimeSlider = new QSlider(Qt::Horizontal);
    exposureTimeSlider->setRange(1000, 1000000);
    exposureTimeSlider->setValue(10000);
    exposureTimeSlider->setEnabled(false);
    
    // Exposure time label
    exposureTimeLabel = new QLabel("Current: 10000 μs");
    exposureTimeLabel->setAlignment(Qt::AlignCenter);
    exposureTimeLabel->setStyleSheet("QLabel { font-weight: bold; color: green; }");
    
    // Exposure controls layout
    QHBoxLayout *exposureControlsLayout = new QHBoxLayout();
    exposureControlsLayout->addWidget(new QLabel("Exposure Time:"));
    exposureControlsLayout->addWidget(exposureTimeSpinBox);
    
    exposureLayout->addLayout(exposureControlsLayout);
    exposureLayout->addWidget(exposureTimeSlider);
    exposureLayout->addWidget(exposureTimeLabel);
    
    // Set exposure time button
    setExposureTimeButton = new QPushButton("Set Exposure Time");
    setExposureTimeButton->setEnabled(false);
    exposureLayout->addWidget(setExposureTimeButton);
    
    leftPanel->addWidget(exposureGroup);
    
    // Create frame rate control section
    QGroupBox *frameRateGroup = new QGroupBox("Frame Rate Control");
    QVBoxLayout *frameRateLayout = new QVBoxLayout(frameRateGroup);
    
    // Frame rate enable checkbox
    frameRateEnabledCheckBox = new QCheckBox("Enable Fixed Frame Rate");
    frameRateEnabledCheckBox->setEnabled(false);
    frameRateLayout->addWidget(frameRateEnabledCheckBox);
    
    // Frame rate spin box
    frameRateSpinBox = new QDoubleSpinBox();
    frameRateSpinBox->setRange(1.0, 100.0);
    frameRateSpinBox->setValue(30.0);
    frameRateSpinBox->setSingleStep(0.1);
    frameRateSpinBox->setDecimals(1);
    frameRateSpinBox->setSuffix(" fps");
    frameRateSpinBox->setEnabled(false);
    
    // Frame rate slider
    frameRateSlider = new QSlider(Qt::Horizontal);
    frameRateSlider->setRange(10, 1000); // 1.0 to 100.0 * 10
    frameRateSlider->setValue(300); // 30.0
    frameRateSlider->setEnabled(false);
    
    // Frame rate label
    frameRateLabel = new QLabel("Current: 30.0 fps");
    frameRateLabel->setAlignment(Qt::AlignCenter);
    frameRateLabel->setStyleSheet("QLabel { font-weight: bold; color: purple; }");
    
    // Frame rate controls layout
    QHBoxLayout *frameRateControlsLayout = new QHBoxLayout();
    frameRateControlsLayout->addWidget(new QLabel("Frame Rate:"));
    frameRateControlsLayout->addWidget(frameRateSpinBox);
    
    frameRateLayout->addLayout(frameRateControlsLayout);
    frameRateLayout->addWidget(frameRateSlider);
    frameRateLayout->addWidget(frameRateLabel);
    
    // Set frame rate button
    setFrameRateButton = new QPushButton("Set Frame Rate");
    setFrameRateButton->setEnabled(false);
    frameRateLayout->addWidget(setFrameRateButton);
    
    leftPanel->addWidget(frameRateGroup);
    
    // Create trigger control section
    QGroupBox *triggerGroup = new QGroupBox("Trigger Control");
    QVBoxLayout *triggerLayout = new QVBoxLayout(triggerGroup);
    
    // Trigger enable checkbox
    triggerEnabledCheckBox = new QCheckBox("Enable Trigger");
    triggerEnabledCheckBox->setEnabled(false);
    triggerLayout->addWidget(triggerEnabledCheckBox);
    
    // Trigger mode combo box
    QHBoxLayout *triggerModeLayout = new QHBoxLayout();
    triggerModeLayout->addWidget(new QLabel("Trigger Mode:"));
    triggerModeComboBox = new QComboBox();
    triggerModeComboBox->addItems(QStringList() << "Off" << "On");
    triggerModeComboBox->setEnabled(false);
    triggerModeLayout->addWidget(triggerModeComboBox);
    triggerLayout->addLayout(triggerModeLayout);
    
    // Trigger source combo box
    QHBoxLayout *triggerSourceLayout = new QHBoxLayout();
    triggerSourceLayout->addWidget(new QLabel("Trigger Source:"));
    triggerSourceComboBox = new QComboBox();
    triggerSourceComboBox->addItems(QStringList() << "Software" << "Line1" << "Line2" << "Line3" << "Line4");
    triggerSourceComboBox->setEnabled(false);
    triggerSourceLayout->addWidget(triggerSourceComboBox);
    triggerLayout->addLayout(triggerSourceLayout);
    
    // Trigger delay spin box
    triggerDelaySpinBox = new QDoubleSpinBox();
    triggerDelaySpinBox->setRange(0.0, 1000000.0);
    triggerDelaySpinBox->setValue(0.0);
    triggerDelaySpinBox->setSingleStep(1.0);
    triggerDelaySpinBox->setDecimals(0);
    triggerDelaySpinBox->setSuffix(" μs");
    triggerDelaySpinBox->setEnabled(false);
    
    // Trigger delay slider
    triggerDelaySlider = new QSlider(Qt::Horizontal);
    triggerDelaySlider->setRange(0, 1000000);
    triggerDelaySlider->setValue(0);
    triggerDelaySlider->setEnabled(false);
    
    // Trigger delay label
    triggerDelayLabel = new QLabel("Current: 0 μs");
    triggerDelayLabel->setAlignment(Qt::AlignCenter);
    triggerDelayLabel->setStyleSheet("QLabel { font-weight: bold; color: orange; }");
    
    // Trigger delay controls layout
    QHBoxLayout *triggerDelayControlsLayout = new QHBoxLayout();
    triggerDelayControlsLayout->addWidget(new QLabel("Trigger Delay:"));
    triggerDelayControlsLayout->addWidget(triggerDelaySpinBox);
    
    triggerLayout->addLayout(triggerDelayControlsLayout);
    triggerLayout->addWidget(triggerDelaySlider);
    triggerLayout->addWidget(triggerDelayLabel);
    
    // Set trigger delay button
    setTriggerDelayButton = new QPushButton("Set Trigger Delay");
    setTriggerDelayButton->setEnabled(false);
    triggerLayout->addWidget(setTriggerDelayButton);
    
    // Software trigger button
    softwareTriggerButton = new QPushButton("Software Trigger");
    softwareTriggerButton->setEnabled(false);
    softwareTriggerButton->setStyleSheet("QPushButton { background-color: #ff6b6b; color: white; font-weight: bold; }");
    triggerLayout->addWidget(softwareTriggerButton);
    
    leftPanel->addWidget(triggerGroup);
    
    // Create image recording control section
    QGroupBox *recordingGroup = new QGroupBox("Image Recording");
    QVBoxLayout *recordingLayout = new QVBoxLayout(recordingGroup);
    
    // Recording toggle button
    recordingToggleButton = new QPushButton("Start Recording");
    recordingToggleButton->setEnabled(false);
    recordingToggleButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    recordingLayout->addWidget(recordingToggleButton);
    
    // Recording status label
    recordingStatusLabel = new QLabel("Status: Not Recording");
    recordingStatusLabel->setAlignment(Qt::AlignCenter);
    recordingStatusLabel->setStyleSheet("QLabel { font-weight: bold; color: gray; padding: 5px; background-color: #f0f0f0; border: 1px solid #ccc; }");
    recordingLayout->addWidget(recordingStatusLabel);
    
    // Recorded image count label
    recordedImageCountLabel = new QLabel("Saved Images: 0");
    recordedImageCountLabel->setAlignment(Qt::AlignCenter);
    recordedImageCountLabel->setStyleSheet("QLabel { font-weight: bold; color: blue; padding: 5px; background-color: #e6f3ff; border: 1px solid #99ccff; }");
    recordingLayout->addWidget(recordedImageCountLabel);
    
    // Reset recording count button
    resetRecordingCountButton = new QPushButton("Reset Count");
    resetRecordingCountButton->setEnabled(false);
    resetRecordingCountButton->setStyleSheet("QPushButton { background-color: #ff9800; color: white; }");
    recordingLayout->addWidget(resetRecordingCountButton);
    
    // Recording path control
    QHBoxLayout *recordingPathLayout = new QHBoxLayout();
    recordingPathLayout->addWidget(new QLabel("Save Path:"));
    recordingPathEdit = new QLineEdit("./recorded_images");
    recordingPathEdit->setEnabled(false);
    recordingPathLayout->addWidget(recordingPathEdit);
    setRecordingPathButton = new QPushButton("Set Path");
    setRecordingPathButton->setEnabled(false);
    recordingPathLayout->addWidget(setRecordingPathButton);
    recordingLayout->addLayout(recordingPathLayout);
    
    // Max recorded images control
    QHBoxLayout *maxRecordedImagesLayout = new QHBoxLayout();
    maxRecordedImagesLayout->addWidget(new QLabel("Max Images:"));
    maxRecordedImagesSpinBox = new QSpinBox();
    maxRecordedImagesSpinBox->setRange(1, 9999);
    maxRecordedImagesSpinBox->setValue(100);
    maxRecordedImagesSpinBox->setEnabled(false);
    maxRecordedImagesLayout->addWidget(maxRecordedImagesSpinBox);
    setMaxRecordedImagesButton = new QPushButton("Set Max");
    setMaxRecordedImagesButton->setEnabled(false);
    maxRecordedImagesLayout->addWidget(setMaxRecordedImagesButton);
    recordingLayout->addLayout(maxRecordedImagesLayout);
    
    leftPanel->addWidget(recordingGroup);
    
    // Create status label
    statusLabel = new QLabel("Status: Ready");
    statusLabel->setStyleSheet("QLabel { color: blue; font-weight: bold; padding: 5px; }");
    leftPanel->addWidget(statusLabel);
    
    // Create camera IP address section
    QGroupBox *ipGroup = new QGroupBox("Camera IP Address");
    QHBoxLayout *ipLayout = new QHBoxLayout(ipGroup);
    
    ipAddressEdit = new QLineEdit("192.168.0.2");
    ipAddressEdit->setPlaceholderText("Enter camera IP address");
    ipAddressEdit->setStyleSheet("QLineEdit { padding: 5px; }");
    
    setIPButton = new QPushButton("Set IP");
    setIPButton->setStyleSheet("QPushButton { padding: 5px; }");
    
    ipLayout->addWidget(new QLabel("IP:"));
    ipLayout->addWidget(ipAddressEdit);
    ipLayout->addWidget(setIPButton);
    
    leftPanel->addWidget(ipGroup);
    
    // Create button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    // Create buttons
    connectButton = new QPushButton("Connect");
    disconnectButton = new QPushButton("Disconnect");
    grabButton = new QPushButton("Start Grabbing");
    
    // Set button styles
    connectButton->setStyleSheet("QPushButton { padding: 8px; font-weight: bold; }");
    disconnectButton->setStyleSheet("QPushButton { padding: 8px; font-weight: bold; }");
    grabButton->setStyleSheet("QPushButton { padding: 8px; font-weight: bold; }");
    setResolutionButton->setStyleSheet("QPushButton { padding: 8px; font-weight: bold; }");
    setScalingFactorButton->setStyleSheet("QPushButton { padding: 8px; font-weight: bold; }");
    setExposureTimeButton->setStyleSheet("QPushButton { padding: 8px; font-weight: bold; }");
    setFrameRateButton->setStyleSheet("QPushButton { padding: 8px; font-weight: bold; }");
    
    // Initially disable buttons
    disconnectButton->setEnabled(false);
    grabButton->setEnabled(false);
    
    buttonLayout->addWidget(connectButton);
    buttonLayout->addWidget(disconnectButton);
    buttonLayout->addWidget(grabButton);
    
    leftPanel->addLayout(buttonLayout);
    
    // Set scroll area widget
    scrollArea->setWidget(scrollContent);
    
    // Add scroll area to top layout
    topLayout->addWidget(scrollArea);
    
    // Create image display label
    imageLabel = new QLabel("No Image");
    imageLabel->setMinimumSize(640, 480);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("QLabel { border: 2px solid gray; background-color: #f0f0f0; }");
    topLayout->addWidget(imageLabel);
    
    // Add top layout to main layout
    mainLayout->addLayout(topLayout);
    
    // Connect button signals
    connect(connectButton, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(disconnectButton, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);
    connect(setIPButton, &QPushButton::clicked, this, &MainWindow::onSetIPClicked);
    connect(grabButton, &QPushButton::clicked, this, &MainWindow::onGrabClicked);
    connect(setResolutionButton, &QPushButton::clicked, this, &MainWindow::onSetResolutionClicked);
    connect(setScalingFactorButton, &QPushButton::clicked, this, &MainWindow::onSetScalingFactorClicked);
    connect(setExposureTimeButton, &QPushButton::clicked, this, &MainWindow::onSetExposureTimeClicked);
    connect(setFrameRateButton, &QPushButton::clicked, this, &MainWindow::onSetFrameRateClicked);
    connect(resolutionComboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &MainWindow::onResolutionComboChanged);
    connect(scalingFactorSlider, &QSlider::valueChanged, this, &MainWindow::onScalingFactorSliderChanged);
    connect(exposureTimeSlider, &QSlider::valueChanged, this, &MainWindow::onExposureTimeSliderChanged);
    connect(exposureAutoCheckBox, &QCheckBox::toggled, this, &MainWindow::onExposureAutoChanged);
    connect(frameRateSlider, &QSlider::valueChanged, this, &MainWindow::onFrameRateSliderChanged);
    connect(frameRateEnabledCheckBox, &QCheckBox::toggled, this, &MainWindow::onFrameRateEnabledChanged);
    connect(setTriggerDelayButton, &QPushButton::clicked, this, &MainWindow::onSetTriggerDelayClicked);
    connect(triggerDelaySlider, &QSlider::valueChanged, this, &MainWindow::onTriggerDelaySliderChanged);
    connect(triggerEnabledCheckBox, &QCheckBox::toggled, this, &MainWindow::onTriggerEnabledChanged);
    connect(triggerModeComboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &MainWindow::onTriggerModeChanged);
    connect(triggerSourceComboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &MainWindow::onTriggerSourceChanged);
    connect(softwareTriggerButton, &QPushButton::clicked, this, &MainWindow::onSoftwareTriggerClicked);
    connect(recordingToggleButton, &QPushButton::clicked, this, &MainWindow::onRecordingToggleClicked);
    connect(resetRecordingCountButton, &QPushButton::clicked, this, &MainWindow::onResetRecordingCountClicked);
    connect(setRecordingPathButton, &QPushButton::clicked, this, &MainWindow::onSetRecordingPathClicked);
    connect(setMaxRecordedImagesButton, &QPushButton::clicked, this, &MainWindow::onSetMaxRecordedImagesClicked);
    
    // Set window properties
    setWindowTitle("Basler Camera Grabber");
    resize(1200, 800);
    setMinimumSize(1000, 600);
}

void MainWindow::onConnectClicked()
{
    if (baslerCamera->connect()) {
        connectButton->setEnabled(false);
        disconnectButton->setEnabled(true);
        grabButton->setEnabled(true);
        setResolutionButton->setEnabled(true);
        setScalingFactorButton->setEnabled(true);
        setExposureTimeButton->setEnabled(true);
        setFrameRateButton->setEnabled(true);
        widthSpinBox->setEnabled(true);
        heightSpinBox->setEnabled(true);
        resolutionComboBox->setEnabled(true);
        scalingFactorSpinBox->setEnabled(true);
        scalingFactorSlider->setEnabled(true);
        exposureTimeSpinBox->setEnabled(true);
        exposureTimeSlider->setEnabled(true);
        exposureAutoCheckBox->setEnabled(true);
        frameRateSpinBox->setEnabled(true);
        frameRateSlider->setEnabled(true);
        frameRateEnabledCheckBox->setEnabled(true);
        triggerEnabledCheckBox->setEnabled(true);
        triggerModeComboBox->setEnabled(true);
        triggerSourceComboBox->setEnabled(true);
        triggerDelaySpinBox->setEnabled(true);
        triggerDelaySlider->setEnabled(true);
        setTriggerDelayButton->setEnabled(true);
        softwareTriggerButton->setEnabled(true);
        recordingToggleButton->setEnabled(true);
        resetRecordingCountButton->setEnabled(true);
        recordingPathEdit->setEnabled(true);
        setRecordingPathButton->setEnabled(true);
        maxRecordedImagesSpinBox->setEnabled(true);
        setMaxRecordedImagesButton->setEnabled(true);
        
        updateCameraInfo();
        updateCameraSettings();
        updateResolutionControls();
        updateScalingControls();
        updateExposureControls();
        updateFrameRateControls();
        updateTriggerControls();
        updateRecordingControls();
    } else {
        QMessageBox::warning(this, "Connection Error", "Failed to connect to camera!");
    }
}

void MainWindow::onDisconnectClicked()
{
    baslerCamera->disconnect();
    connectButton->setEnabled(true);
    disconnectButton->setEnabled(false);
    grabButton->setEnabled(false);
    setResolutionButton->setEnabled(false);
    setScalingFactorButton->setEnabled(false);
    setExposureTimeButton->setEnabled(false);
    setFrameRateButton->setEnabled(false);
    widthSpinBox->setEnabled(false);
    heightSpinBox->setEnabled(false);
    resolutionComboBox->setEnabled(false);
    scalingFactorSpinBox->setEnabled(false);
    scalingFactorSlider->setEnabled(false);
    exposureTimeSpinBox->setEnabled(false);
    exposureTimeSlider->setEnabled(false);
    exposureAutoCheckBox->setEnabled(false);
    frameRateSpinBox->setEnabled(false);
    frameRateSlider->setEnabled(false);
    frameRateEnabledCheckBox->setEnabled(false);
    triggerEnabledCheckBox->setEnabled(false);
    triggerModeComboBox->setEnabled(false);
    triggerSourceComboBox->setEnabled(false);
    triggerDelaySpinBox->setEnabled(false);
    triggerDelaySlider->setEnabled(false);
    setTriggerDelayButton->setEnabled(false);
    softwareTriggerButton->setEnabled(false);
    recordingToggleButton->setEnabled(false);
    resetRecordingCountButton->setEnabled(false);
    recordingPathEdit->setEnabled(false);
    setRecordingPathButton->setEnabled(false);
    maxRecordedImagesSpinBox->setEnabled(false);
    setMaxRecordedImagesButton->setEnabled(false);
    grabButton->setText("Start Grabbing");
    
    // Clear image and camera info
    imageLabel->setText("No Image");
    cameraInfoLabel->setText("Camera not connected");
    cameraSettingsLabel->setText("Settings: Not available");
    resolutionComboBox->clear();
    resolutionComboBox->addItem("Select resolution...");
    scalingFactorLabel->setText("Current: 1.00x");
    exposureTimeLabel->setText("Current: 10000 μs");
    frameRateLabel->setText("Current: 30.0 fps");
    recordingStatusLabel->setText("Status: Not Recording");
    recordedImageCountLabel->setText("Saved Images: 0");
    recordingPathEdit->setText("./recorded_images");
    maxRecordedImagesSpinBox->setValue(100);
}

void MainWindow::onGrabClicked()
{
    if (baslerCamera->isConnected()) {
        if (grabButton->text() == "Start Grabbing") {
            baslerCamera->startGrabbing();
            grabButton->setText("Stop Grabbing");
        } else {
            baslerCamera->stopGrabbing();
            grabButton->setText("Start Grabbing");
        }
    }
}

void MainWindow::onSetResolutionClicked()
{
    int width = widthSpinBox->value();
    int height = heightSpinBox->value();
    
    if (baslerCamera->setResolution(width, height)) {
        updateCameraSettings();
        updateResolutionControls();
    } else {
        QMessageBox::warning(this, "Resolution Error", "Failed to set resolution!");
    }
}

void MainWindow::onResolutionComboChanged(const QString &text)
{
    if (text == "Select resolution..." || text.isEmpty()) {
        return;
    }
    
    // Parse resolution from text (e.g., "1920 x 1080")
    QStringList parts = text.split(" x ");
    if (parts.size() == 2) {
        bool ok1, ok2;
        int width = parts[0].toInt(&ok1);
        int height = parts[1].toInt(&ok2);
        
        if (ok1 && ok2) {
            widthSpinBox->setValue(width);
            heightSpinBox->setValue(height);
        }
    }
}

void MainWindow::updateImage()
{
    if (baslerCamera->isConnected()) {
        cv::Mat image = baslerCamera->getImage();
        
        if (!image.empty()) {
            // Convert OpenCV Mat to QImage
            QImage qimg(image.data, image.cols, image.rows, image.step, QImage::Format_RGB888);
            qimg = qimg.rgbSwapped(); // Convert BGR to RGB
            
            // Scale image to fit label while maintaining aspect ratio
            QPixmap pixmap = QPixmap::fromImage(qimg);
            pixmap = pixmap.scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            
            imageLabel->setPixmap(pixmap);
        }
    }
}

void MainWindow::updateStatus(const QString &status)
{
    statusLabel->setText("Status: " + status);
    qDebug() << "[MainWindow]" << status;
}

void MainWindow::updateCameraInfo()
{
    QString info = baslerCamera->getCameraInfo();
    cameraInfoLabel->setText(info);
}

void MainWindow::updateCameraSettings()
{
    QString settings = baslerCamera->getCurrentSettings();
    cameraSettingsLabel->setText(settings);
}

void MainWindow::updateResolutionControls()
{
    if (!baslerCamera->isConnected()) {
        return;
    }
    
    // Update spin boxes with current values
    widthSpinBox->setValue(baslerCamera->getWidth());
    heightSpinBox->setValue(baslerCamera->getHeight());
    
    // Update combo box with available resolutions
    resolutionComboBox->clear();
    resolutionComboBox->addItem("Select resolution...");
    
    QStringList resolutions = baslerCamera->getAvailableResolutions();
    for (const QString& res : resolutions) {
        resolutionComboBox->addItem(res);
    }
}

void MainWindow::onSetScalingFactorClicked()
{
    double factor = scalingFactorSpinBox->value();
    
    if (baslerCamera->setScalingFactor(factor)) {
        updateCameraSettings();
        updateScalingControls();
    } else {
        QMessageBox::warning(this, "Scaling Factor Error", "Failed to set scaling factor!");
    }
}

void MainWindow::onScalingFactorSliderChanged(int value)
{
    double factor = value / 100.0;
    scalingFactorSpinBox->setValue(factor);
    scalingFactorLabel->setText(QString("Current: %1x").arg(factor, 0, 'f', 2));
}

void MainWindow::updateScalingControls()
{
    if (!baslerCamera->isConnected()) {
        return;
    }
    
    // Update spin box with current value
    scalingFactorSpinBox->setValue(baslerCamera->getScalingFactor());
    
    // Update slider with current value
    int sliderValue = static_cast<int>(baslerCamera->getScalingFactor() * 100);
    scalingFactorSlider->setValue(sliderValue);
    
    // Update label
    scalingFactorLabel->setText(QString("Current: %1x").arg(baslerCamera->getScalingFactor(), 0, 'f', 2));
    
    // Update range if needed
    double minFactor = baslerCamera->getMinScalingFactor();
    double maxFactor = baslerCamera->getMaxScalingFactor();
    double increment = baslerCamera->getScalingFactorIncrement();
    
    scalingFactorSpinBox->setRange(minFactor, maxFactor);
    scalingFactorSpinBox->setSingleStep(increment);
    scalingFactorSlider->setRange(static_cast<int>(minFactor * 100), static_cast<int>(maxFactor * 100));
}

void MainWindow::onSetExposureTimeClicked()
{
    double exposureTime = exposureTimeSpinBox->value();
    
    if (baslerCamera->setExposureTime(exposureTime)) {
        updateCameraSettings();
        updateExposureControls();
    } else {
        QMessageBox::warning(this, "Exposure Time Error", "Failed to set exposure time!");
    }
}

void MainWindow::onExposureTimeSliderChanged(int value)
{
    exposureTimeSpinBox->setValue(value);
    exposureTimeLabel->setText(QString("Current: %1 μs").arg(value));
}

void MainWindow::onExposureAutoChanged(bool checked)
{
    if (baslerCamera->setExposureAuto(checked)) {
        updateCameraSettings();
        updateExposureControls();
    } else {
        QMessageBox::warning(this, "Exposure Auto Error", "Failed to set exposure auto!");
        // Revert checkbox state
        exposureAutoCheckBox->setChecked(!checked);
    }
}

void MainWindow::updateExposureControls()
{
    if (!baslerCamera->isConnected()) {
        return;
    }
    
    // Update spin box with current value
    exposureTimeSpinBox->setValue(baslerCamera->getExposureTime());
    
    // Update slider with current value
    exposureTimeSlider->setValue(static_cast<int>(baslerCamera->getExposureTime()));
    
    // Update label
    exposureTimeLabel->setText(QString("Current: %1 μs").arg(baslerCamera->getExposureTime(), 0, 'f', 0));
    
    // Update checkbox
    exposureAutoCheckBox->setChecked(baslerCamera->isExposureAuto());
    
    // Update range if needed
    double minExposure = baslerCamera->getMinExposureTime();
    double maxExposure = baslerCamera->getMaxExposureTime();
    double increment = baslerCamera->getExposureTimeIncrement();
    
    exposureTimeSpinBox->setRange(minExposure, maxExposure);
    exposureTimeSpinBox->setSingleStep(increment);
    exposureTimeSlider->setRange(static_cast<int>(minExposure), static_cast<int>(maxExposure));
    
    // Enable/disable manual controls based on auto exposure
    bool manualEnabled = !baslerCamera->isExposureAuto();
    exposureTimeSpinBox->setEnabled(manualEnabled);
    exposureTimeSlider->setEnabled(manualEnabled);
    setExposureTimeButton->setEnabled(manualEnabled);
}

void MainWindow::onSetFrameRateClicked()
{
    double frameRate = frameRateSpinBox->value();
    
    if (baslerCamera->setFrameRate(frameRate)) {
        updateCameraSettings();
        updateFrameRateControls();
    } else {
        QMessageBox::warning(this, "Frame Rate Error", "Failed to set frame rate!");
    }
}

void MainWindow::onFrameRateSliderChanged(int value)
{
    double frameRate = value / 10.0;
    frameRateSpinBox->setValue(frameRate);
    frameRateLabel->setText(QString("Current: %1 fps").arg(frameRate, 0, 'f', 1));
}

void MainWindow::onFrameRateEnabledChanged(bool checked)
{
    if (baslerCamera->setFrameRateEnabled(checked)) {
        updateCameraSettings();
        updateFrameRateControls();
    } else {
        QMessageBox::warning(this, "Frame Rate Enable Error", "Failed to set frame rate enable!");
        // Revert checkbox state
        frameRateEnabledCheckBox->setChecked(!checked);
    }
}

void MainWindow::updateFrameRateControls()
{
    if (!baslerCamera->isConnected()) {
        return;
    }
    
    // Update spin box with current value
    frameRateSpinBox->setValue(baslerCamera->getFrameRate());
    
    // Update slider with current value
    int sliderValue = static_cast<int>(baslerCamera->getFrameRate() * 10);
    frameRateSlider->setValue(sliderValue);
    
    // Update label
    frameRateLabel->setText(QString("Current: %1 fps").arg(baslerCamera->getFrameRate(), 0, 'f', 1));
    
    // Update checkbox
    frameRateEnabledCheckBox->setChecked(baslerCamera->isFrameRateEnabled());
    
    // Update range if needed
    double minFrameRate = baslerCamera->getMinFrameRate();
    double maxFrameRate = baslerCamera->getMaxFrameRate();
    double increment = baslerCamera->getFrameRateIncrement();
    
    frameRateSpinBox->setRange(minFrameRate, maxFrameRate);
    frameRateSpinBox->setSingleStep(increment);
    frameRateSlider->setRange(static_cast<int>(minFrameRate * 10), static_cast<int>(maxFrameRate * 10));
    
    // Enable/disable manual controls based on frame rate enable
    bool manualEnabled = baslerCamera->isFrameRateEnabled();
    frameRateSpinBox->setEnabled(manualEnabled);
    frameRateSlider->setEnabled(manualEnabled);
    setFrameRateButton->setEnabled(manualEnabled);
}

void MainWindow::onFrameRateUpdated(double frameRate)
{
    // Update real-time frame rate display
    realTimeFrameRateLabel->setText(QString("Current FPS: %1").arg(frameRate, 0, 'f', 1));
    frameCountLabel->setText(QString("Frame Count: %1").arg(baslerCamera->getFrameCount()));
}

void MainWindow::onFrameIdUpdated(int frameId)
{
    frameIdLabel->setText(QString("Frame ID: %1").arg(frameId));
}

void MainWindow::onErrorsCountUpdated(int errorsCount)
{
    errorsCountLabel->setText(QString("Errors: %1").arg(errorsCount));
}

void MainWindow::updateRealTimeFrameRateDisplay()
{
    if (!baslerCamera->isConnected()) {
        realTimeFrameRateLabel->setText("Current FPS: 0.0");
        frameCountLabel->setText("Frame Count: 0");
        return;
    }
    
    // Update frame rate and count display
    realTimeFrameRateLabel->setText(QString("Current FPS: %1").arg(baslerCamera->getRealTimeFrameRate(), 0, 'f', 1));
    frameCountLabel->setText(QString("Frame Count: %1").arg(baslerCamera->getFrameCount()));
}

// Trigger control slots
void MainWindow::onTriggerEnabledChanged(bool checked)
{
    if (baslerCamera->setTriggerEnabled(checked)) {
        updateCameraSettings();
        updateTriggerControls();
    } else {
        QMessageBox::warning(this, "Trigger Enable Error", "Failed to set trigger enable!");
        // Revert checkbox state
        triggerEnabledCheckBox->setChecked(!checked);
    }
}

void MainWindow::onTriggerModeChanged(const QString &text)
{
    if (baslerCamera->setTriggerMode(text)) {
        updateCameraSettings();
        updateTriggerControls();
    } else {
        QMessageBox::warning(this, "Trigger Mode Error", "Failed to set trigger mode!");
        // Revert combo box selection
        triggerModeComboBox->setCurrentText(baslerCamera->getTriggerMode());
    }
}

void MainWindow::onTriggerSourceChanged(const QString &text)
{
    if (baslerCamera->setTriggerSource(text)) {
        updateCameraSettings();
        updateTriggerControls();
    } else {
        QMessageBox::warning(this, "Trigger Source Error", "Failed to set trigger source!");
        // Revert combo box selection
        triggerSourceComboBox->setCurrentText(baslerCamera->getTriggerSource());
    }
}

void MainWindow::onSetTriggerDelayClicked()
{
    double delay = triggerDelaySpinBox->value();
    
    if (baslerCamera->setTriggerDelay(delay)) {
        updateCameraSettings();
        updateTriggerControls();
    } else {
        QMessageBox::warning(this, "Trigger Delay Error", "Failed to set trigger delay!");
    }
}

void MainWindow::onTriggerDelaySliderChanged(int value)
{
    double delay = static_cast<double>(value);
    triggerDelaySpinBox->setValue(delay);
    triggerDelayLabel->setText(QString("Current: %1 μs").arg(delay, 0, 'f', 0));
}

void MainWindow::updateTriggerControls()
{
    if (!baslerCamera->isConnected()) {
        return;
    }
    
    // Update spin box with current value
    triggerDelaySpinBox->setValue(baslerCamera->getTriggerDelay());
    
    // Update slider with current value
    triggerDelaySlider->setValue(static_cast<int>(baslerCamera->getTriggerDelay()));
    
    // Update label
    triggerDelayLabel->setText(QString("Current: %1 μs").arg(baslerCamera->getTriggerDelay(), 0, 'f', 0));
    
    // Update checkbox
    triggerEnabledCheckBox->setChecked(baslerCamera->isTriggerEnabled());
    
    // Update combo boxes
    triggerModeComboBox->setCurrentText(baslerCamera->getTriggerMode());
    triggerSourceComboBox->setCurrentText(baslerCamera->getTriggerSource());
    
    // Update range if needed
    double minDelay = baslerCamera->getMinTriggerDelay();
    double maxDelay = baslerCamera->getMaxTriggerDelay();
    double increment = baslerCamera->getTriggerDelayIncrement();
    
    triggerDelaySpinBox->setRange(minDelay, maxDelay);
    triggerDelaySpinBox->setSingleStep(increment);
    triggerDelaySlider->setRange(static_cast<int>(minDelay), static_cast<int>(maxDelay));
    
    // Enable/disable manual controls based on trigger enable
    bool manualEnabled = baslerCamera->isTriggerEnabled();
    triggerModeComboBox->setEnabled(manualEnabled);
    triggerSourceComboBox->setEnabled(manualEnabled);
    triggerDelaySpinBox->setEnabled(manualEnabled);
    triggerDelaySlider->setEnabled(manualEnabled);
    setTriggerDelayButton->setEnabled(manualEnabled);
    
    // Enable software trigger button only when trigger is enabled and source is Software
    bool softwareTriggerEnabled = manualEnabled && (baslerCamera->getTriggerSource() == "Software");
    softwareTriggerButton->setEnabled(softwareTriggerEnabled);
}

void MainWindow::onSoftwareTriggerClicked()
{
    if (!baslerCamera->isConnected() || !baslerCamera->isTriggerEnabled()) {
        return;
    }
    
    if (baslerCamera->executeSoftwareTrigger()) {
        qDebug() << "[MainWindow] Software trigger executed successfully";
    } else {
        QMessageBox::warning(this, "Software Trigger Error", "Failed to execute software trigger!");
    }
}

// Image recording control slots
void MainWindow::onRecordingToggleClicked()
{
    bool isRecording = baslerCamera->isRecordingEnabled();
    baslerCamera->setRecordingEnabled(!isRecording);
    updateRecordingControls();
}

void MainWindow::onResetRecordingCountClicked()
{
    baslerCamera->resetRecordingCount();
    updateRecordingControls();
}

void MainWindow::updateRecordingControls()
{
    if (!baslerCamera->isConnected()) {
        return;
    }
    
    bool isRecording = baslerCamera->isRecordingEnabled();
    
    // Update toggle button
    if (isRecording) {
        recordingToggleButton->setText("Stop Recording");
        recordingToggleButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; }");
        recordingStatusLabel->setText("Status: Recording");
        recordingStatusLabel->setStyleSheet("QLabel { font-weight: bold; color: red; padding: 5px; background-color: #ffe6e6; border: 1px solid #ff9999; }");
    } else {
        recordingToggleButton->setText("Start Recording");
        recordingToggleButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
        recordingStatusLabel->setText("Status: Not Recording");
        recordingStatusLabel->setStyleSheet("QLabel { font-weight: bold; color: gray; padding: 5px; background-color: #f0f0f0; border: 1px solid #ccc; }");
    }
    
    // Update recorded image count
    int recordedCount = baslerCamera->getRecordedImageCount();
    recordedImageCountLabel->setText(QString("Saved Images: %1").arg(recordedCount));
    
    // Update recording path
    recordingPathEdit->setText(baslerCamera->getRecordingPath());
    
    // Update max recorded images
    maxRecordedImagesSpinBox->setValue(baslerCamera->getMaxRecordedImages());
    
    // Enable reset button only if there are recorded images
    resetRecordingCountButton->setEnabled(recordedCount > 0);
}

void MainWindow::onSetRecordingPathClicked()
{
    QString newPath = recordingPathEdit->text().trimmed();
    if (!newPath.isEmpty()) {
        baslerCamera->setRecordingPath(newPath);
        updateRecordingControls();
        qDebug() << "[MainWindow] Recording path set to:" << newPath;
    } else {
        QMessageBox::warning(this, "Path Error", "Please enter a valid path!");
    }
}

void MainWindow::onSetMaxRecordedImagesClicked()
{
    int maxCount = maxRecordedImagesSpinBox->value();
    if (maxCount > 0) {
        baslerCamera->setMaxRecordedImages(maxCount);
        updateRecordingControls();
        qDebug() << "[MainWindow] Max recorded images set to:" << maxCount;
    } else {
        QMessageBox::warning(this, "Max Count Error", "Please enter a valid number greater than 0!");
    }
}

void MainWindow::onSetIPClicked()
{
    QString ipAddress = ipAddressEdit->text().trimmed();
    if (!ipAddress.isEmpty()) {
        baslerCamera->setCameraIP(ipAddress);
        qDebug() << "[MainWindow] Camera IP set to:" << ipAddress;
        updateStatus(QString("Camera IP set to: %1").arg(ipAddress));
    } else {
        QMessageBox::warning(this, "IP Address Error", "Please enter a valid IP address!");
    }
}

