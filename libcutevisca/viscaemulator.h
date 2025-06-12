#ifndef VISCAEMULATOR_H
#define VISCAEMULATOR_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QByteArray>
#include <QDebug>

struct PTZState {
    int zoomLevel = 0;
    int panPos = 0;
    int tiltPos = 0;
    int panSpeed = 0;
    int tiltSpeed = 0;
};

struct ViscaClient {
    QHostAddress sender;
    quint16 port;
};

class ViscaEmulator : public QObject {
    Q_OBJECT

public:
    explicit ViscaEmulator(QObject *parent = nullptr);

protected:
    void sendViscaResponse(const QByteArray &header, const QByteArray &responsePayload, const ViscaClient &c);
private slots:
    void onReadyRead();

private:
    QUdpSocket *socket;
    quint32 seqNum;
    quint16 listenPort = 52388; // 52381;
    quint32 expectedSequenceNumber;
    PTZState ptzState;

    void sendAckAndCompletion(const QByteArray &originalPayload, const ViscaClient &c);
    void sendErrorResponse(const ViscaClient &c);
    void sendViscaResponse(const QByteArray &responsePayload, const ViscaClient &c, uchar type=0x01, uchar detail=0x11);

    void handleViscaCommand(const QByteArray &payload, const ViscaClient &c);
    void handleZoomCommand(const QByteArray &payload);

    void handlePanTiltAutomaticCommand(const QByteArray &payload);
    void handlePanTiltAbsoluteCommand(const QByteArray &payload);
    void handlePanTiltRelativeCommand(const QByteArray &payload);
};

#endif // VISCAEMULATOR_H
