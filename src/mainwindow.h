#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/qmainwindow.h>

#include "ui_mainwindow.h"

#include <QTextToSpeech>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);

public slots:
    void speak();
    void stop();

    void setRate(int);
    void setPitch(int);
    void setVolume(int volume);

    void stateChanged(QTextToSpeech::State state);
    void engineSelected(int index);
    void languageSelected();
    void voiceSelected(int index);

    void localeChanged(const QLocale &locale);

private:
    Ui::MainWindow *ui;
    QTextToSpeech *speech;
    QVector<QVoice> voices;
};

#endif
