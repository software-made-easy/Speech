#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QCommandLineParser>
#include <QSplashScreen>


#if defined(Q_OS_WASM) && QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#error You must use Qt 5.14 or newer // Becouse of the file dialogs
#elif QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
#error You must use Qt 5.10 or newer // Because of QGuiApplication::screenAt
#endif


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationVersion(QStringLiteral(APP_VERSION));

    QSplashScreen splash;
    splash.setWindowFlag(Qt::WindowStaysOnTopHint);
    splash.show();
    a.processEvents();

    static constexpr QChar underscore[1] = {
        QLatin1Char('_')
    };

    QTranslator translator, qtTranslator;

    // load translation for Qt
    if (qtTranslator.load(QLocale::system(), QStringLiteral("qtbase"),
                          QString::fromRawData(underscore, 1),
                          QStringLiteral(
                              ":/qtTranslations/")))
        a.installTranslator(&qtTranslator);

    // try to load translation for current locale from resource file
    if (translator.load(QLocale::system(), QStringLiteral("Speech"),
                        QString::fromRawData(underscore, 1),
                        QStringLiteral(":/translations")))
        a.installTranslator(&translator);

    MainWindow w;

    splash.finish(&w);

    w.show();

    return a.exec();
}
