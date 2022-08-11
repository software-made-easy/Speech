#include "mainwindow.h"


#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
#define COUNTRY(l) l.nativeTerritoryName()
#else
#define COUNTRY(l) l.nativeCountryName()
#endif


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      speech(nullptr)
{
    ui->setupUi(this);

    // Populate engine selection list
    ui->engine->addItem(QStringLiteral("Default"), QStringLiteral("default"));
    const auto engines = QTextToSpeech::availableEngines();
    for (const QString &engine : engines)
        ui->engine->addItem(engine, engine);
    ui->engine->setCurrentIndex(0);
    engineSelected(0);

    connect(ui->speakButton, &QPushButton::clicked, this, &MainWindow::speak);
    connect(ui->pitch, &QSlider::valueChanged, this, &MainWindow::setPitch);
    connect(ui->rate, &QSlider::valueChanged, this, &MainWindow::setRate);
    connect(ui->volume, &QSlider::valueChanged, this, &MainWindow::setVolume);
    connect(ui->engine, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::engineSelected);
}

void MainWindow::speak()
{
    speech->say(ui->plainTextEdit->toPlainText());
}

void MainWindow::stop()
{
    speech->stop();
}

void MainWindow::setRate(int rate)
{
    speech->setRate(rate / 10.0);
}

void MainWindow::setPitch(int pitch)
{
    speech->setPitch(pitch / 10.0);
}

void MainWindow::setVolume(int volume)
{
    speech->setVolume(volume / 100.0);
}

void MainWindow::stateChanged(QTextToSpeech::State state)
{
    switch (state) {
    case QTextToSpeech::Speaking:
        ui->statusbar->showMessage(tr("Speech started..."));
        break;
    case QTextToSpeech::Ready:
        ui->statusbar->showMessage(tr("Speech stopped..."), 2000);
        break;
    case QTextToSpeech::Paused:
        ui->statusbar->showMessage(tr("Speech paused..."));
        break;
    case QTextToSpeech::BackendError:
        ui->statusbar->showMessage(tr("Speech error!"));
        break;
    default:
        break;
    }

    ui->pauseButton->setEnabled(state == QTextToSpeech::Speaking);
    ui->resumeButton->setEnabled(state == QTextToSpeech::Paused);
    ui->stopButton->setEnabled(state == QTextToSpeech::Speaking || state == QTextToSpeech::Paused);
}

void MainWindow::engineSelected(int index)
{
    QString engineName = ui->engine->currentData().toString();
    delete speech;
    if (engineName == "default")
        speech = new QTextToSpeech(this);
    else
        speech = new QTextToSpeech(engineName, this);
    disconnect(ui->language, &QComboBox::currentIndexChanged, this, &MainWindow::languageSelected);
    ui->language->clear();

    // Populate the languages combobox before connecting its signal.
    const QVector<QLocale> locales = speech->availableLocales();

    static QLocale current = QLocale::system();
    static const QString lang = current.nativeLanguageName();
    static const QString country = current.nativeCountryName();
    static const QString name = lang + QLatin1String(" (") + country + QChar(')');

    for (const QLocale &locale : locales) {
        QVariant localeVariant(locale);
        ui->language->addItem(name, localeVariant);
        if (locale.name() == current.name())
            current = locale;
    }
    setRate(ui->rate->value());
    setPitch(ui->pitch->value());
    setVolume(ui->volume->value());
    connect(ui->stopButton, &QPushButton::clicked, speech, &QTextToSpeech::stop);
    connect(ui->pauseButton, &QPushButton::clicked, speech, &QTextToSpeech::pause);
    connect(ui->resumeButton, &QPushButton::clicked, speech, &QTextToSpeech::resume);

    connect(speech, &QTextToSpeech::stateChanged, this, &MainWindow::stateChanged);
    connect(speech, &QTextToSpeech::localeChanged, this, &MainWindow::localeChanged);

    connect(ui->language, &QComboBox::currentIndexChanged, this, &MainWindow::languageSelected);
    localeChanged(current);
}

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
    QVariant localeVariant(locale);
    ui->language->setCurrentIndex(ui->language->findData(localeVariant));

    disconnect(ui->voice, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::voiceSelected);
    ui->voice->clear();

    voices = speech->availableVoices();
    QVoice currentVoice = speech->voice();
    for (const QVoice &voice : qAsConst(voices)) {
        ui->voice->addItem(QStringLiteral("%1 - %2 - %3").arg(voice.name(),
                                                       QVoice::genderName(voice.gender()),
                                                       QVoice::ageName(voice.age())));
        if (voice.name() == currentVoice.name())
            ui->voice->setCurrentIndex(ui->voice->count() - 1);
    }
    connect(ui->voice, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::voiceSelected);
}
