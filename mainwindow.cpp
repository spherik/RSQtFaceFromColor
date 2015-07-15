#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onNewImage()));


    // Create a PXCSenseManager instance
    sm = PXCSenseManager::CreateInstance();
    pxcStatus sts;
    // Select the color stream
    sm->EnableStream(PXCCapture::STREAM_TYPE_COLOR,640,480);

    /* Set Module */
    sts = sm->EnableFace();
    if(sts < PXC_STATUS_NO_ERROR)
    {
        std::cerr << "EnableFace error" << std::endl;
        return;
    }

    faceModule = sm->QueryFace();
    if (faceModule == NULL)
    {
        std::cout << "FaceModule error" << std::endl;
        return;
    }

    config = NULL;
    faceData = NULL;

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNewImage()
{
    if (sm->AcquireFrame(true)<PXC_STATUS_NO_ERROR)
        return;

    // In every new frame, update the face data
    faceData->Update();

    // retrieve the sample
    PXCCapture::Sample *sample=sm->QuerySample();

    //image is a PXCImage instance
    PXCImage::ImageData colorData;
    sample->color->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &colorData);
    int height = sample->color->QueryInfo().height;
    int width= sample->color->QueryInfo().width;

    // image planes are in data.planes[0-3] with pitch data.pitches[0-3]
    QImage colorImage(colorData.planes[0],width, height,QImage::Format_RGB32);

    // Release access to camera's memory
    sample->color->ReleaseAccess(&colorData);



    // go fetching the next sample
    sm->ReleaseFrame();

    const int numFaces = faceData->QueryNumberOfDetectedFaces();
    std::cout << numFaces << " faces detected" << std::endl;

    for(int i = 0; i < numFaces && i < MAX_FACES; i++)
    {
        PXCRectI32 rectangle;
        PXCFaceData::Face* trackedFace = faceData->QueryFaceByIndex(i);
        const PXCFaceData::DetectionData* detectionData = trackedFace->QueryDetection();
        pxcBool hasRect = detectionData->QueryBoundingRect(&rectangle);
        if(hasRect)
        {
            QPainter qPainter(&colorImage);
            qPainter.setBrush(Qt::NoBrush);
            qPainter.setPen(Qt::red);
            qPainter.drawRect(rectangle.x,rectangle.y,rectangle.w,rectangle.h);
            bool bEnd = qPainter.end();
        }
    }

    // Set image in label
    ui->colorImage->setPixmap( QPixmap::fromImage(colorImage).scaled(width,height,Qt::KeepAspectRatio));

}

void MainWindow::onStart()
{

    // Initialize the PXCSenseManager
    sm->Init();

    //Retrieve PXCFaceData instance from the face module
    faceData = faceModule->CreateOutput();

    //Retrieve an active configuration from the face data
    config = faceModule->CreateActiveConfiguration();
    if (config == NULL)
    {
        std::cout << "Config error" << std::endl;
        return;
    }

    // Set the tracking mode
    config->SetTrackingMode(PXCFaceConfiguration::FACE_MODE_COLOR);

    // Enable the detection algorithm
    config->detection.isEnabled = true;

    // Set the maximum number of faces to track
    config->detection.maxTrackedFaces = MAX_FACES;

    // Apply the changes
    config->ApplyChanges();

    // Set framerate (30 fps)
    timer->start(1000/30);

    // Set UI
    this->ui->startButton->setEnabled(false);
    this->ui->stopButton->setEnabled(true);
}

void MainWindow::onStop()
{
    // Stop timer
    timer->stop();

    // Release any instance of PXCFaceConfiguration
    config->Release();

    // Close the last opened stream and release any session and processing module instances
    sm->Release();

    // Set UI
    this->ui->startButton->setEnabled(true);
    this->ui->stopButton->setEnabled(false);

}
