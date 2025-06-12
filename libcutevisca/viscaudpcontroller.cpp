#include "viscaudpcontroller.h"

ViscaUdpController::ViscaUdpController(QObject *parent) : QObject(parent), sequenceNumber(1), timeoutid(0) {
    udpSocket = new QUdpSocket(this);

    // Bind to listen for responses from the camera
    if (!udpSocket->bind(QHostAddress::Any, localPort)) {
        qWarning("Failed to bind listening socket");
    }

    // Connect response handling
    connect(udpSocket, &QUdpSocket::readyRead, this, &ViscaUdpController::processIncomingData);
}

bool ViscaUdpController::connectCamera()
{
    resetSequenceNumber();

    return true;
}

void ViscaUdpController::disconnectCamera()
{
    udpSocket->close();

    m_isConnected=false;
    m_isPowered=false;

    emit isConnectedChanged();
    emit isPoweredChanged();
}

void ViscaUdpController::setCamera(const QString &host, uint port)
{
    cameraPort=port;
    cameraIP=host;
}

/**
 * @brief ViscaUdpController::sendViscaCommand
 * @param command
 * @param type
 * @return sequence number of sent command
 *
 * Send given VISCA raw command as UDP packet. Takes care of combining the message header, command and end marker (0xFF)
 *
 */
quint32 ViscaUdpController::sendViscaCommand(const QByteArray &command, uint type)
{
    QByteArray udpHeader(8, 0);

    // Set command type
    udpHeader[0] = (type >> 8) & 0xFF;
    udpHeader[1] = type & 0xFF;

    // Payload size (VISCA command length)
    int payloadSize = command.size()+(type==0x0200 ? 0 : 1); // Adjust for 0xFF
    udpHeader[2] = (payloadSize >> 8) & 0xFF; // High byte
    udpHeader[3] = payloadSize & 0xFF;        // Low byte

    // Sequence number (increments for each command)
    udpHeader[4] = (sequenceNumber >> 24) & 0xFF;
    udpHeader[5] = (sequenceNumber >> 16) & 0xFF;
    udpHeader[6] = (sequenceNumber >> 8) & 0xFF;
    udpHeader[7] = sequenceNumber & 0xFF;

    // Combine header and command
    QByteArray packet = udpHeader + command;
    if (type!=0x0200)
        packet.append(0xFF);

    qint64 bytesSent = udpSocket->writeDatagram(packet, QHostAddress(cameraIP), cameraPort);

    if (bytesSent == -1) {
        qDebug() << "Failed to send VISCA command!" << udpSocket->error() << udpSocket->errorString();
        emit networkError(udpSocket->error());
        return 0;
    }

    if (timeoutid>0)
        killTimer(timeoutid);
    timeoutid=startTimer(100);

    qDebug() << "Sent VISCA command (Seq:" << sequenceNumber << ", Size:" << bytesSent << " bytes)" << packet.toHex(':') << cameraIP << cameraPort;
    expectedSeq = sequenceNumber;
    sequenceNumber++;

    return expectedSeq;
}

void ViscaUdpController::timerEvent(QTimerEvent *event)
{
    qDebug("Timeout");
    killTimer(timeoutid);
    emit timeoutError();
}

void ViscaUdpController::resetSequenceNumber()
{
    QByteArray command = QByteArray::fromHex("01");
    sequenceNumber = 0;
    sendViscaCommand(command, 0x0200);
}

void ViscaUdpController::zoomIn(uint speed)  {
    QByteArray command = QByteArray::fromHex("8101040700");

    command[4] = 0x20 + std::clamp(speed, (uint)0, (uint)7);

    sendViscaCommand(command);
}

void ViscaUdpController::zoomOut(uint speed)  {
    QByteArray command = QByteArray::fromHex("8101040700");

    command[4] = 0x30 + std::clamp(speed, (uint)0, (uint)7);

    sendViscaCommand(command);
}

void ViscaUdpController::zoomSet(uint zoom)
{
    if (zoom > 0x6000)
        zoom=0x6000;

    QByteArray command=QByteArray::fromHex("8101044700000000");

    command[4] = (zoom & 0xF000) >> 12;
    command[5] = (zoom & 0x0F00) >> 8;
    command[6] = (zoom & 0x00F0) >> 4 ;
    command[7] = (zoom & 0x000F);

    sendViscaCommand(command);
}

void ViscaUdpController::inquireZoom() {
    auto s=sendViscaCommand(QByteArray::fromHex("81090447"), 0x0110);
    m_cbmap.insert(s, [=](QByteArray &data) {
        int z = (static_cast<uchar>(data[0]) << 12) |
                  (static_cast<uchar>(data[1]) << 8) |
                  (static_cast<uchar>(data[2]) << 4) |
                  (static_cast<uchar>(data[3]));
        qDebug() << "Zoom is " << z;

        m_zoomPosition=z;

        emit zoomPositionChanged();
    });
}

void ViscaUdpController::panTilt(int panSpeed, int tiltSpeed, int panDir, int tiltDir) {
    QByteArray command;
    command.append(0x81);
    command.append(0x01);
    command.append(0x06);
    command.append(0x01);
    command.append(panSpeed & 0x1F);
    command.append(tiltSpeed & 0x1F);
    command.append(panDir & 0x03);
    command.append(tiltDir & 0x03);

    sendViscaCommand(command);
}

void ViscaUdpController::setMemoryPreset(int preset)
{
    if (preset < 0 || preset > 15) return;
    QByteArray command = QByteArray::fromHex("8101043F0100");
    command[5] = preset;
    sendViscaCommand(command);
}

void ViscaUdpController::recallMemoryPreset(int preset)
{
    if (preset < 0 || preset > 15) return;
    QByteArray command = QByteArray::fromHex("8101043F0200");
    command[5] = preset;
    sendViscaCommand(command);
}

void ViscaUdpController::processIncomingData()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray buffer;
        buffer.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        killTimer(timeoutid);
        timeoutid=0;

        udpSocket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

        // Extract sequence number from response
        int seq = (static_cast<uchar>(buffer[4]) << 24) |
                  (static_cast<uchar>(buffer[5]) << 16) |
                  (static_cast<uchar>(buffer[6]) << 8) |
                  (static_cast<uchar>(buffer[7]));

        uchar type = buffer[0];
        uchar detail = buffer[1];

        qDebug() << "Response " << type << detail << " (Seq:" << seq << ") from camera: " << buffer.toHex(':');

        if (seq!=expectedSeq) {
            qWarning() << "Unexpected sequence ack" << seq << expectedSeq;
        }

        switch (type) {
        case 0x01: // Command
            switch (detail){
            case 0x00: // Command
            case 0x10: // Inquiry
                // XXX: We send these types, so a reply with them is an error, afaik
                qWarning("Invalid reply type");
                break;
            case 0x11: // Command response
                parseResponse(buffer, seq);
                //sequenceNumber++;
                break;
            }

            break;
        case 0x02: // Control
            switch (detail) {
            case 0x00: //
                //sequenceNumber++;
                break;
            case 0x01: // Control reply
                sequenceNumber=1;
                // Query power state
                requestPower();
                //clear();
                m_isConnected=true;
                emit isConnectedChanged();
                emit connected();
                break;
            }
            break;
        }
    }
}

void ViscaUdpController::parseResponse(const QByteArray &response, quint32 seq)
{
    if (response.size()<8) {
        return;
    }
    uchar ip=response.at(8);
    uchar y=ip >> 4;
    uchar r=response.at(9);

    uchar re=r >> 4; // Response code
    uchar des=r & 0x0F; // Socket

    if (r>=0x60 && response.size()>10) {
        uchar e=response.at(10);
        qDebug() << "Command error" << ip << r << e << re << des;
        emit commandError(e);
        return;
    }

    switch (re) {
    case 0x4: // ACK
        qDebug() << "Command ACK" << ip << y << r;
        break;
    case 0x5: // Completed
        if (response.size()>11) {
            auto data=response.mid(10);
            qDebug() << "Inquiery Completion" << data.toHex(':');
            auto cb=m_cbmap.value(seq);
            if (cb) {
                m_cbmap.remove(seq);
                cb(data);
            }
        } else {
            qDebug() << "Command Completion";
        }
        break;
    default:
        qWarning() << "Unknown respose code" << re << ip << y << r;
        break;
    }

}

bool ViscaUdpController::isPowered() const
{
    return m_isPowered;
}

bool ViscaUdpController::isConnected() const
{
    return m_isConnected;
}

int ViscaUdpController::zoomPosition() const
{
    return m_zoomPosition;
}
