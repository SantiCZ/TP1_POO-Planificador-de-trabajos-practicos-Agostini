#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("CollaborativeCanvas");
    app.setOrganizationName("CollabCanvas");
    app.setStyle("Fusion");

    MainWindow w;
    w.show();

    return app.exec();
}
