#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QToolBar>
#include <QStatusBar>
#include "DrawingModel.h"
#include "CanvasView.h"
#include "SyncManager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onSaveClicked();
    void onThicknessChanged(int t);
    void onSaveSuccess();
    void onSaveError(const QString &msg);

private:
    DrawingModel *m_model;
    CanvasView   *m_canvas;
    SyncManager  *m_sync;

    QLabel       *m_colorSwatch;
    QLabel       *m_thicknessLabel;
    QLabel       *m_statusLabel;
    QPushButton  *m_saveButton;

    int   m_colorIndex = 0;

    void setupToolbar();
    void applyMetroStyle();
    void updateColorSwatch();
    QColor colorForIndex(int idx) const;
};