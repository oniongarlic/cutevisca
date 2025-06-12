#include "viscaemulator.h"

ViscaEmulator::ViscaEmulator(QObject *parent)
    : QObject(parent), expectedSequenceNumber(0),seqNum(0) {
    socket = new QUdpSocket(this);
    if (!socket->bind(QHostAddress::LocalHost, listenPort)) {
        qCritical() << "Failed to bind UDP socket on port" << listenPort;
        return;
    }

    connect(socket, &QUdpSocket::readyRead, this, &ViscaEmulator::onReadyRead);
    qDebug() << "VISCA Emulator listening on UDP port" << listenPort;
}

void ViscaEmulator::onReadyRead() {
    ViscaClient c;
    while (socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        socket->readDatagram(datagram.data(), datagram.size(), &c.sender, &c.port);

        qDebug() << datagram.size() << datagram.toHex(':');

        // Header 8 bytes + Payload 1-16 bytes
        if (datagram.size() < 8) {
            qWarning("Invalid packet");
            continue;
        }

        QByteArray header = datagram.left(8);
        QByteArray payload = datagram.mid(8);

        if (payload.size() < 1) {
            qWarning("Invalid payload");
            continue;
        }

        quint16 psize = static_cast<quint8>(header[2]) << 8 | static_cast<quint8>(header[3]);

        seqNum = static_cast<quint8>(header[4]) << 24 |
                         static_cast<quint8>(header[5]) << 16 |
                         static_cast<quint8>(header[6]) << 8 |
                         static_cast<quint8>(header[7]);

        qDebug() << "Payload size" << psize << "Sequence" << seqNum;

        handleViscaCommand(payload, c);
    }
}

void ViscaEmulator::handleViscaCommand(const QByteArray &payload, const ViscaClient &c)
{
    // Sequence reset
    if (payload[0] == 0x01 && payload.size()==1) {
        expectedSequenceNumber = 0;
        seqNum = 0;
        sendViscaResponse(QByteArray(), c, 0x02, 0x01);
        expectedSequenceNumber++;
        return;
    }

    if (seqNum != expectedSequenceNumber) {
        qWarning() << "Wrong sequence number, expected" << expectedSequenceNumber << " got " << seqNum;
        sendErrorResponse(c);
        return;
    }

    quint8 cnt=payload[0] >> 4;
    quint8 dev=payload[0] & 0x0F;
    quint8 ci=payload[1];
    quint8 cc=payload[2];

    QByteArray cmd = payload.mid(2);

    qDebug() << cnt << dev << ci << cc << cmd.toHex();

    switch (ci) {
    case 0x01: // Command
        if (cmd.startsWith(QByteArray::fromHex("0407"))) {
            handleZoomCommand(payload);
        } else if (cmd.startsWith(QByteArray::fromHex("0601"))) {
            handlePanTiltAutomaticCommand(payload);
        } else if (cmd.startsWith(QByteArray::fromHex("0602"))) {
            handlePanTiltAbsoluteCommand(payload);
        } else if (cmd.startsWith(QByteArray::fromHex("0603"))) {
            handlePanTiltRelativeCommand(payload);
        } else if (cmd.startsWith(QByteArray::fromHex("0604"))) {
            // Home
            ptzState.tiltPos=0;
            ptzState.panPos=0;
        } else if (cmd.startsWith(QByteArray::fromHex("0605"))) {
            // Reset
            ptzState.tiltPos=0;
            ptzState.panPos=0;
            ptzState.zoomLevel=0;
        } else {
            qWarning() << "Unhandled command payload" << payload.toHex(':');
        }
        sendAckAndCompletion(payload, c);
        break;
    case 0x09: // Inquiry
        if (cmd.startsWith(QByteArray::fromHex("0400"))) {
            sendViscaResponse(QByteArray::fromHex("905002FF"), c);
        } else if (cmd.startsWith(QByteArray::fromHex("0002"))) {
            sendViscaResponse(QByteArray::fromHex("905096961212000000FF"), c);
        } else {
            qWarning() << "Unhandled inquiry payload" << payload.toHex(':');
            sendErrorResponse(c);
        }
        break;
    default: // Invalid
        qWarning("Invalid, not a command or inquiry?");
        sendErrorResponse(c);
        break;
    }
}

void ViscaEmulator::handleZoomCommand(const QByteArray &payload) {
    quint8 zoomSpeed, direction;
    if (payload.size() < 6) return;

    quint8 zoomType = payload[4] >> 4;

    if (zoomType==0) {
        zoomSpeed = 3; // "normal speed"
        direction = payload[4];
    } else {
        zoomSpeed = payload[4] & 0x0F;
        direction = payload[4] >> 4;
    }

    qDebug() << "Zoom" << zoomType << direction << zoomSpeed;

    switch (direction) {
    case 0x02: // Tele
        ptzState.zoomLevel += zoomSpeed * 100;
        break;
    case 0x03: // Wide
        ptzState.zoomLevel -= zoomSpeed * 100;
        break;
    case 0x00: // Stop
    default:
        break;
    }

    ptzState.zoomLevel = std::clamp(ptzState.zoomLevel, 0, 0x4000);
    qDebug() << "Zoom set to:" << ptzState.zoomLevel;
}

void ViscaEmulator::handlePanTiltRelativeCommand(const QByteArray &payload)
{

}

void ViscaEmulator::handlePanTiltAutomaticCommand(const QByteArray &payload)
{

}

void ViscaEmulator::handlePanTiltAbsoluteCommand(const QByteArray &payload)
{
    if (payload.size() < 11) return;

    ptzState.panSpeed = payload[4];
    ptzState.tiltSpeed = payload[5];

    int panPos = (payload[6] << 12) | (payload[7] << 8) | (payload[8] << 4) | payload[9];
    int tiltPos = (payload[10] << 12) | (payload[11] << 8) | (payload[12] << 4) | payload[13];

    ptzState.panPos = qBound(-0x3000, panPos, 0x3000);
    ptzState.tiltPos = qBound(-0x1000, tiltPos, 0x1000);

    qDebug() << "Pan set to:" << ptzState.panPos << "Speed:" << ptzState.panSpeed;
    qDebug() << "Tilt set to:" << ptzState.tiltPos << "Speed:" << ptzState.tiltSpeed;
}

void ViscaEmulator::sendAckAndCompletion(const QByteArray &originalPayload, const ViscaClient &c) {
    sendViscaResponse(QByteArray::fromHex("9041FF"), c);
    sendViscaResponse(QByteArray::fromHex("9051FF"), c);

    expectedSequenceNumber++;
}

void ViscaEmulator::sendErrorResponse(const ViscaClient &c) {
    sendViscaResponse(QByteArray::fromHex("906002FF"), c);

    expectedSequenceNumber++;
}

void ViscaEmulator::sendViscaResponse(const QByteArray &header, const QByteArray &responsePayload, const ViscaClient &c) {
    QByteArray packet = header + responsePayload;
    socket->writeDatagram(packet, c.sender, c.port);
}

void ViscaEmulator::sendViscaResponse(const QByteArray &responsePayload, const ViscaClient &c, uchar type, uchar detail) {
    QByteArray header(16, 0);
    header[0] = type;
    header[1] = detail;
    header[2] = (responsePayload.size() >> 8) & 0xFF;
    header[3] = responsePayload.size() & 0xFF;
    header[4] = (expectedSequenceNumber >> 24) & 0xFF;
    header[5] = (expectedSequenceNumber >> 16) & 0xFF;
    header[6] = (expectedSequenceNumber >> 8) & 0xFF;
    header[7] = expectedSequenceNumber & 0xFF;

    qDebug() << expectedSequenceNumber;

    sendViscaResponse(header, responsePayload, c);
}
