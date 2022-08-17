#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QCommandLineParser>
#include <QSplashScreen>


#if !(QT_VERSION >= QT_VERSION_CHECK(5, 15, 0) && QT_VERSION <= QT_VERSION_CHECK(6, 0, 0) || QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
#error Only Qt versions 5.15 and greater equal to 6.4 are supported
#endif


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationVersion(QStringLiteral(APP_VERSION));

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
    w.show();

    return a.exec();
}
