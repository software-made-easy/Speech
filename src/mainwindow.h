#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextToSpeech>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QTextToSpeech;
class QVoice;
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void speak();
    void stop();
    void pause();
    void resume();

    void setRate(int);
    void setPitch(int);
    void setVolume(int volume);

    void stateChanged(QTextToSpeech::State state);
    void engineSelected(int index);
    void languageSelected();
    void voiceSelected(int index);

    void localeChanged(const QLocale &locale);

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    void onError(QTextToSpeech::ErrorReason reason, const QString &errorString);
#endif

private:
    Ui::MainWindow *ui;
    QSharedPointer<QTextToSpeech> speech;
    // QTextToSpeech *speech;
    QVector<QVoice> voices;
};

#endif
