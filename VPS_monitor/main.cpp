#include <QApplication>
#include <QIcon>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName(QStringLiteral("VPS Monitor"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));
    app.setOrganizationName(QStringLiteral("Ejercicio 02"));

    // High-DPI support
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    MainWindow w;
    w.show();

    return app.exec();
}
