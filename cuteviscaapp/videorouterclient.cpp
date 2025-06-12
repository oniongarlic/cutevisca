#include "videorouterclient.h"

#include <QDebug>

void VideoRouterClient::connectToHost(const QString &host, quint16 port)
{
    inputLabels.clear();
    outputLabels.clear();
    currentRouting.clear();
    socket->connectToHost(host, port);
}

void VideoRouterClient::onConnected()
{
    qDebug() << "Connected to video router";
    m_isConnected=true;
    emit isConnectedChanged();
}

void VideoRouterClient::onDisconnected()
{
    qDebug() << "Disconnected to video router";
    inputLabels.clear();
    outputLabels.clear();
    currentRouting.clear();
    m_isConnected=false;
    emit isConnectedChanged();
}

void VideoRouterClient::onReadyRead()
{
    buffer.append(socket->readAll());
    QList<QByteArray> lines = buffer.split('\n');

    if (!buffer.endsWith('\n'))
        buffer = lines.takeLast(); // Keep incomplete line in buffer
    else
        buffer.clear();

    for (const QByteArray &lineRaw : lines) {
        QString line = QString::fromUtf8(lineRaw).trimmed();
        if (line.isEmpty()) continue;

        if (!initialized) {
            if (line == "INPUT LABELS:") {
                state = InputLabels;
            } else if (line == "OUTPUT LABELS:") {
                state = OutputLabels;
            } else if (line == "VIDEO OUTPUT ROUTING:") {
                state = Routing;
            } else if (line == "END PRELUDE:") {
                initialized = true;
                qDebug() << "Initial configuration complete.";
                qDebug() << inputLabels;
                qDebug() << outputLabels;
                qDebug() << currentRouting;
                emit tablesInitialized();
            } else {
                parseInitialMessage(line);
            }
        } else {
            handleRoutingChange(line);
        }
    }
}

void VideoRouterClient::parseInitialMessage(const QString &line)
{
    QStringList parts = line.split(' ', Qt::SkipEmptyParts);
    if (parts.size() < 2) return;

    bool ok;
    int index = parts[0].toInt(&ok);
    if (!ok) return;

    QString label = parts.mid(1).join(" ");

    switch (state) {
    case InputLabels:
        inputLabels[index] = label;
        break;
    case OutputLabels:
        outputLabels[index] = label;
        break;
    case Routing:
        if (parts.size() == 2) {
            int input = parts[1].toInt(&ok);
            if (ok) {
                currentRouting[index] = input;
            }
        }
        break;
    default:
        break;
    }
}

void VideoRouterClient::handleRoutingChange(const QString &line)
{
    QStringList parts = line.split(' ', Qt::SkipEmptyParts);
    if (parts.size() != 2) return;

    bool ok1, ok2;
    int output = parts[0].toInt(&ok1);
    int input = parts[1].toInt(&ok2);

    if (ok1 && ok2) {
        if (currentRouting.value(output) != input) {
            currentRouting[output] = input;
            emit routingChanged(output, input);
        }
    }
}

void VideoRouterClient::setRouting(int output, int input)
{
    if (!socket->isOpen()) return;

    QString command = QString("%1 %2\n").arg(output).arg(input);
    socket->write(command.toUtf8());
    socket->flush();
}

VideoRouterClient::VideoRouterClient(QObject *parent)
    : QObject(parent),
    socket(new QTcpSocket(this)),
    initialized(false),
    state(None)
{
    connect(socket, &QTcpSocket::readyRead, this, &VideoRouterClient::onReadyRead);
    connect(socket, &QTcpSocket::connected, this, &VideoRouterClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &VideoRouterClient::onDisconnected);
}

bool VideoRouterClient::isConnected() const
{
    return m_isConnected;
}

void VideoRouterClient::setIsConnected(bool newIsConnected)
{
    if (m_isConnected == newIsConnected)
        return;
    m_isConnected = newIsConnected;
    emit isConnectedChanged();
}
