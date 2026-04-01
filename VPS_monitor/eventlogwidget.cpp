#include "eventlogwidget.h"

#include <QListWidgetItem>
#include <QFont>

EventLogWidget::EventLogWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_list = new QListWidget(this);
    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setAlternatingRowColors(false);
    m_list->setSelectionMode(QAbstractItemView::NoSelection);
    m_list->setFocusPolicy(Qt::NoFocus);
    m_list->setUniformItemSizes(false);
    m_list->setWordWrap(true);

    // Estilo oscuro coherente con el panel
    m_list->setStyleSheet(QStringLiteral(R"(
        QListWidget {
            background: #12121A;
            border: none;
            color: #CCCCDD;
        }
        QListWidget::item {
            padding: 4px 8px;
            border-bottom: 1px solid #1E1E2A;
        }
        QListWidget::item:hover {
            background: #1A1A26;
        }
        QScrollBar:vertical {
            background: #12121A;
            width: 6px;
        }
        QScrollBar::handle:vertical {
            background: #333345;
            border-radius: 3px;
        }
    )"));

    layout->addWidget(m_list);
}

void EventLogWidget::appendEvent(const EventEntry &entry)
{
    const QString timeStr = entry.when.toString(QStringLiteral("HH:mm:ss"));
    const QString icon    = iconForSeverity(entry.severity);
    const QString text    = QStringLiteral("%1  %2  %3").arg(timeStr, icon, entry.message);

    auto *item = new QListWidgetItem(text, m_list);
    item->setForeground(colorForSeverity(entry.severity));

    QFont f = item->font();
    f.setPointSize(9);
    f.setFamily(QStringLiteral("Monospace"));
    item->setFont(f);

    // Insertar al inicio (más reciente primero)
    m_list->insertItem(0, item);

    // Limitar a 200 entradas visibles
    while (m_list->count() > 200)
        delete m_list->takeItem(m_list->count() - 1);
}

void EventLogWidget::loadAll(const QList<EventEntry> &entries)
{
    m_list->clear();
    // Los eventos ya vienen con el más reciente primero
    for (const EventEntry &e : entries)
        appendEvent(e);
}

void EventLogWidget::clearLog()
{
    m_list->clear();
}

QColor EventLogWidget::colorForSeverity(const QString &sev) const
{
    if (sev == QLatin1String("critical")) return QColor(0xFF, 0x6E, 0x6E);
    if (sev == QLatin1String("warning"))  return QColor(0xF5, 0xC2, 0x42);
    return QColor(0x88, 0xBB, 0xFF);   // info
}

QString EventLogWidget::iconForSeverity(const QString &sev) const
{
    if (sev == QLatin1String("critical")) return QStringLiteral("[!!]");
    if (sev == QLatin1String("warning"))  return QStringLiteral("[!]");
    return QStringLiteral("[i]");
}
