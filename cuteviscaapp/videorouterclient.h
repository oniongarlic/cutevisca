#ifndef VIDEOROUTERCLIENT_H
#define VIDEOROUTERCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QQmlEngine>

class VideoRouterClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isConnected READ isConnected WRITE setIsConnected NOTIFY isConnectedChanged FINAL)
    QML_ELEMENT
public:
    explicit VideoRouterClient(QObject *parent = nullptr);

    Q_INVOKABLE void setRouting(int output, int input);
    Q_INVOKABLE void connectToHost(const QString &host, quint16 port);

    Q_INVOKABLE int getInputsCount() { return inputLabels.count(); }
    Q_INVOKABLE int getOutputsCount() { return outputLabels.count(); }
    Q_INVOKABLE int getRoutingFor(int output) { return currentRouting.value(output); }

    bool isConnected() const;
    void setIsConnected(bool newIsConnected);

signals:
    void routingChanged(int output, int input);
    void tablesInitialized();

    void isConnectedChanged();

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();

private:
    QTcpSocket *socket;
    QByteArray buffer;
    bool initialized;

    enum ParseState {
        None,
        InputLabels,
        OutputLabels,
        Routing
    };

    ParseState state;

    QMap<int, QString> inputLabels;
    QMap<int, QString> outputLabels;
    QMap<int, int> currentRouting;

    void handleRoutingChange(const QString &line);
    void parseInitialMessage(const QString &line);
    bool m_isConnected;
};

#endif // VIDEOROUTERCLIENT_H
