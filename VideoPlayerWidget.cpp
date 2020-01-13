#include "VideoPlayerWidget.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>

VideoPlayerWidget::VideoPlayerWidget()
{
	this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
	this->setAttribute(Qt::WA_TranslucentBackground);
	this->resize(800, 600);

	this->setStyleSheet(".QWidget{background: rgb(80, 80, 80);}");
	QWidget* widget = new QWidget;
	QVBoxLayout* l = new QVBoxLayout(this);
	l->addWidget(widget);

#ifdef USEDOPENFLRENDER
	m_pRender = new OpenGLRender;
#else
	m_pRender = new QtRenderWidget;
#endif

    QVBoxLayout* mainLayout = new QVBoxLayout(widget);
	QWidget* w = dynamic_cast<QWidget*>(m_pRender);
    mainLayout->addWidget(w);

	m_pSlider = new QSlider(Qt::Horizontal, this);
	m_pSlider->setMinimum(0);
	m_pSlider->setMaximum(100);
	mainLayout->addWidget(m_pSlider);
	QObject::connect(m_pSlider, &QSlider::valueChanged, this, &VideoPlayerWidget::onSliderValueChanged);

    QWidget* pW = new QWidget;
    mainLayout->addWidget(pW);
    QPushButton* loadVideoButton = new QPushButton("Load");
    QObject::connect(loadVideoButton, &QPushButton::clicked, \
        this, &VideoPlayerWidget::onClickedLoadButton);
    QHBoxLayout* buttonLayout = new QHBoxLayout(pW);
    buttonLayout->addWidget(loadVideoButton);
	// set button tag
	m_pPlayButton = new QPushButton("Play/Pause");
	buttonLayout->addWidget(m_pPlayButton);
	QObject::connect(m_pPlayButton, &QPushButton::clicked, this, &VideoPlayerWidget::onPlayButtonClicked);
	// display time label
	m_pTimeTag = new QLabel;
	buttonLayout->addWidget(m_pTimeTag);
    buttonLayout->addStretch();

    m_pDecodecVideoThread = new DecodecVideo(this);
    
    // create player
    m_pVideoPlayOper = new VideoPlayOper(this);
    m_pVideoPlayOper->setVideoRender(m_pRender);
    m_pVideoPlayOper->setVideoDecodec(m_pDecodecVideoThread);
	QObject::connect(m_pVideoPlayOper, &VideoPlayOper::updateDisplayInfos, \
		this, &VideoPlayerWidget::onUpdateDisplayInfos);
}

VideoPlayerWidget::~VideoPlayerWidget()
{
    
}

void VideoPlayerWidget::onClickedLoadButton(void)
{
    QString videoFileName = QFileDialog::getOpenFileName(this, "OpenVideoFile", "./");
    m_pVideoPlayOper->play(videoFileName);
}

void VideoPlayerWidget::onSliderValueChanged(int value)
{
	if (m_pDecodecVideoThread == nullptr)
		return;

	qreal totalTime = m_pDecodecVideoThread->getCurrentInfo().totalTime;
	if (fabs(totalTime - 0) < 0.0000001)
		return;

	qreal time = value * 1.0 / 100 * totalTime;
	m_pDecodecVideoThread->seekVideo(time);
}

void VideoPlayerWidget::onPlayButtonClicked(void)
{
	DecodecVideo::PlayStatus status = m_pDecodecVideoThread->getCurrentPlayerStatus();
	if (status == DecodecVideo::Player_Playing)
		m_pDecodecVideoThread->setCurrentPlayerStatus(DecodecVideo::Player_Pause);
	else
		m_pDecodecVideoThread->setCurrentPlayerStatus(DecodecVideo::Player_Playing);
}

void VideoPlayerWidget::setCurrentDisplayTime(qreal time)
{
	// current time String
	int hour = (int)time / 60 / 60;
	int min = (int)time / 60 % 60;
	int sec = (int)time % 60;
	QString timeStr = QString("%1:%2:%3").arg(hour, 2, 10, QChar('0')) \
		.arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0'));

	// total time String
	qreal totalTime = m_pDecodecVideoThread->getCurrentInfo().totalTime;
	hour = (int)totalTime / 60 / 60;
	min = (int)totalTime / 60 % 60;
	sec = (int)totalTime % 60;
	QString totalTimeStr = QString("%1:%2:%3").arg(hour, 2, 10, QChar('0')) \
		.arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0'));

	m_pTimeTag->setText(timeStr + " / " + totalTimeStr);
}

void VideoPlayerWidget::onUpdateDisplayInfos(qreal time)
{
	setCurrentDisplayTime(time);

	m_pSlider->blockSignals(true);
	qreal totalTime = m_pDecodecVideoThread->getCurrentInfo().totalTime;
	m_pSlider->setValue(time / totalTime * 100);
	m_pSlider->blockSignals(false);
}

void VideoPlayerWidget::closeEvent(QCloseEvent * event)
{
	m_pDecodecVideoThread->requestInterruption();
	this->close();
}
