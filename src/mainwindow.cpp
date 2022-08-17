#include "ui_mainwindow.h"
#include "mainwindow.h"

#include <QTimer>
#include <QDebug>
#include <QShortcut>


#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
#define COUNTRY(l) l.nativeTerritoryName()
#else
#define COUNTRY(l) l.nativeCountryName()
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
#define QTEXTTOSPEECH_ERROR QTextToSpeech::Error
#define WORD QTextToSpeech::BoundaryHint::Word
#define IMMEDIATE QTextToSpeech::BoundaryHint::Immediate
#define DEFAULD QTextToSpeech::BoundaryHint::Default
#else
#define QTEXTTOSPEECH_ERROR QTextToSpeech::BackendError
#define WORD
#define IMMEDIATE
#define DEFAULD
#endif


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      speech(nullptr)
{
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

    ui->setupUi(this);

    connect(ui->speakButton, &QPushButton::clicked, this, &MainWindow::speak);
    connect(ui->pitch, &QSlider::valueChanged, this, &MainWindow::setPitch);
    connect(ui->rate, &QSlider::valueChanged, this, &MainWindow::setRate);
    connect(ui->volume, &QSlider::valueChanged, this, &MainWindow::setVolume);

    connect(ui->stopButton, &QPushButton::pressed, this, &MainWindow::stop);
    connect(ui->pauseButton, &QPushButton::pressed, this,
            &MainWindow::pause);
    connect(ui->resumeButton, &QPushButton::pressed, this,
            &MainWindow::resume);

    QShortcut *shortCut = new QShortcut(this);
    shortCut->setKey(QKeySequence(QKeySequence::Quit));
    connect(shortCut, &QShortcut::activated, qApp, &QApplication::closeAllWindows);

    QMetaObject::invokeMethod(this, [this]{
        // Populate engine selection list
        ui->engine->addItem(tr("Default"), QStringLiteral("default"));
        const QStringList engines = QTextToSpeech::availableEngines();
        for (const QString &engine : engines)
            ui->engine->addItem(engine, engine);
        ui->engine->setCurrentIndex(0);

        engineSelected(0);
        statusBar()->hide();
        QGuiApplication::restoreOverrideCursor();
    }, Qt::QueuedConnection);
}

void MainWindow::speak()
{
    Q_ASSERT(speech);

    if (speech->state() == QTextToSpeech::Speaking)
        speech->stop(DEFAULD);

    const QString text = ui->plainTextEdit->toPlainText();
    qInfo() << __func__ << text;

    if (!text.isEmpty())
        speech->say(text);
}

void MainWindow::stop()
{
    qInfo() << __func__;
    Q_ASSERT(speech);
    speech->stop(DEFAULD);
}

void MainWindow::pause()
{
    qInfo() << __func__;
    Q_ASSERT(speech);
    speech->pause(DEFAULD);
}

void MainWindow::resume()
{
    qInfo() << __func__;
    Q_ASSERT(speech);
    speech->resume();
}

void MainWindow::setRate(int rate)
{
    qInfo() << __func__ << rate;
    speech->setRate(rate / 10.0);
}

void MainWindow::setPitch(int pitch)
{
    qInfo() << __func__ << pitch;
    speech->setPitch(pitch / 10.0);
}

void MainWindow::setVolume(int volume)
{
    qInfo() << __func__ << volume;
    speech->setVolume(volume / 100.0);
}

void MainWindow::stateChanged(QTextToSpeech::State state)
{
    qDebug() << __func__ << state;
    statusBar()->show();

    switch (state) {
    case QTextToSpeech::Speaking:
        ui->statusbar->showMessage(tr("Speech started..."), 0);
        break;
    case QTextToSpeech::Ready:
        ui->statusbar->showMessage(tr("Speech stopped..."), 0);
        break;
    case QTextToSpeech::Paused:
        ui->statusbar->showMessage(tr("Speech paused..."), 0);
        break;
    case QTEXTTOSPEECH_ERROR:
        ui->statusbar->showMessage(tr("Speech error!"), 0);
        break;
    default:
        statusBar()->hide();
        break;
    }
    QTimer::singleShot(5000, statusBar(), &QStatusBar::hide);

    ui->pauseButton->setEnabled(state == QTextToSpeech::Speaking);
    ui->resumeButton->setEnabled(state == QTextToSpeech::Paused);
    ui->stopButton->setEnabled(state == QTextToSpeech::Speaking || state == QTextToSpeech::Paused);
}

void MainWindow::engineSelected(int index)
{
    QString engineName = ui->engine->itemData(index).toString();

    qDebug() << __func__ << engineName << speech;

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    // If not initilized
    if (!speech || engineName == QLatin1String("default")) {
        if (engineName == QLatin1String("default"))
            speech.reset(new QTextToSpeech(this));
        else
            speech.reset(new QTextToSpeech(engineName, this));

        connect(speech.get(), &QTextToSpeech::stateChanged, this, &MainWindow::stateChanged);
    }
    else if (speech->engine() != engineName)
        speech->setEngine(engineName);
#else
    if (engineName == QLatin1String("default"))
        speech.reset(new QTextToSpeech(this));
    else
        speech.reset(new QTextToSpeech(engineName, this));

    connect(speech.get(), &QTextToSpeech::stateChanged, this, &MainWindow::stateChanged);
#endif

    disconnect(ui->language, &QComboBox::currentIndexChanged, this, &MainWindow::languageSelected);
    ui->language->clear();

    // Populate the languages combobox before connecting its signal.
    const auto locales = speech->availableLocales();

    QLocale current = speech->locale();

    for (const QLocale &locale : locales) {
        ui->language->addItem(locale.nativeLanguageName() + QLatin1String(" (") +
                              COUNTRY(locale) + QChar(')'),
                              locale);
        if (locale.name() == current.name())
            current = locale;
    }
    setRate(ui->rate->value());
    setPitch(ui->pitch->value());
    setVolume(ui->volume->value());

    connect(speech.get(), &QTextToSpeech::localeChanged, this, &MainWindow::localeChanged);

    connect(ui->language, &QComboBox::currentIndexChanged, this, &MainWindow::languageSelected);
    localeChanged(current);

    connect(ui->engine, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &MainWindow::engineSelected, Qt::UniqueConnection);
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    connect(speech.get(), &QTextToSpeech::errorOccurred,
            this, &MainWindow::onError, Qt::UniqueConnection);
#endif
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
void MainWindow::onError(QTextToSpeech::ErrorReason reason, const QString &errorString)
{
    qDebug() << __func__ << errorString;
    statusBar()->show();
    statusBar()->showMessage(errorString);
    QTimer::singleShot(10000, statusBar(), &QStatusBar::hide);
}
#endif

void MainWindow::languageSelected()
{
    speech->setLocale(ui->language->currentData().toLocale());
}

void MainWindow::voiceSelected(int index)
{
    speech->setVoice(voices.at(index));
}

void MainWindow::localeChanged(const QLocale &locale)
{
    ui->language->setCurrentIndex(ui->language->findData(locale));

    disconnect(ui->voice, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::voiceSelected);
    ui->voice->clear();

    const QString langName = QLocale::languageToString(locale.language());

    voices = speech->availableVoices();
    QVoice currentVoice = speech->voice();
    for (const QVoice &voice : qAsConst(voices)) {
        QString voiceName = voice.name();
        QVoice::Gender voiceGender = voice.gender();
        QVoice::Age voiceAge = voice.age();
        QString genderString;

        QString name;

        if (voiceName.startsWith(langName)) {
            voiceName.remove(0, langName.length());

            if (voiceName.startsWith(QChar('+')))
                voiceName.remove(0, 1);
        }

        if (voiceGender == QVoice::Unknown) {
            if (voiceName.contains(QLatin1String("female")) || voiceName.contains(QLatin1String("grandma")))
                voiceGender = QVoice::Female;
            else if (voiceName.contains(QLatin1String("male")) || voiceName.contains(QLatin1String("grandpa")))
                voiceGender = QVoice::Male;
        }

        if (voiceGender == QVoice::Male)
            genderString = tr("Male");
        else if (voiceGender == QVoice::Female)
            genderString = tr("Female");

        if (!voiceName.isEmpty())
            name += voiceName;
        if (!genderString.isEmpty())
            name += QLatin1String(" - ") + genderString;
        if (voiceAge != QVoice::Other)
            name += QLatin1String(" - ") + QVoice::ageName(voiceAge);

        if (!name.isEmpty()) {
            ui->voice->addItem(name);
            if (voice.name() == currentVoice.name())
                ui->voice->setCurrentIndex(ui->voice->count() - 1);
        }
    }
    connect(ui->voice, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::voiceSelected);
}

MainWindow::~MainWindow()
{
    delete ui;
}
