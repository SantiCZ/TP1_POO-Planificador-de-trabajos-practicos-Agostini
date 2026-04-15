#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void solicitarDatos();              // Pide el JSON al servidor
    void procesarRespuesta(QNetworkReply *reply); // Dibuja el tablero

private:
    // --- ESTA ES LA LÍNEA QUE TE FALTABA ---
    void enviarOrden(QString urlStr);
    // ---------------------------------------

    QNetworkAccessManager *manager;
    QTimer *timer;
    QWidget *centralWidget;
    QHBoxLayout *layoutTablero;
};

#endif // MAINWINDOW_H