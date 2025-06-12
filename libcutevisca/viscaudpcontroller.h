#ifndef VISCAUDPCONTROLLER_H
#define VISCAUDPCONTROLLER_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QByteArray>
#include <QDebug>

class ViscaUdpController : public QObject {
    Q_OBJECT
    //Q_PROPERTY(int portNumber READ portNumber WRITE setPortNumber NOTIFY portNumberChanged FINAL)
    Q_PROPERTY(bool isPowered READ isPowered NOTIFY isPoweredChanged FINAL)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged FINAL)

    Q_PROPERTY(int zoomPosition READ zoomPosition NOTIFY zoomPositionChanged FINAL)

public:
    explicit ViscaUdpController(QObject *parent = nullptr);

    Q_INVOKABLE bool connectCamera();
    Q_INVOKABLE void disconnectCamera();

    Q_INVOKABLE void setCamera(const QString &host, uint port=52381);

    quint32 sendViscaCommand(const QByteArray &command, uint type=0x0100);

    void resetSequenceNumber();

    Q_INVOKABLE void clear()  { sendViscaCommand(QByteArray::fromHex("81010001")); }

    Q_INVOKABLE void requestVersion()  { sendViscaCommand(QByteArray::fromHex("81090002"), 0x0110); }
    Q_INVOKABLE void requestPower()  { sendViscaCommand(QByteArray::fromHex("81090400"), 0x0110); }

    // Power Control
    Q_INVOKABLE void powerOn()  { sendViscaCommand(QByteArray::fromHex("8101040002")); }
    Q_INVOKABLE void powerOff() { sendViscaCommand(QByteArray::fromHex("8101040003")); }

    // Zoom Control
    Q_INVOKABLE void zoomIn()  { sendViscaCommand(QByteArray::fromHex("8101040702")); }
    Q_INVOKABLE void zoomOut() { sendViscaCommand(QByteArray::fromHex("8101040703")); }
    Q_INVOKABLE void zoomStop(){ sendViscaCommand(QByteArray::fromHex("8101040700")); }
    Q_INVOKABLE void zoomIn(uint speed);
    Q_INVOKABLE void zoomOut(uint speed);
    Q_INVOKABLE void zoomSet(uint zoom);

    Q_INVOKABLE void inquireZoom();

    // Shutter
    Q_INVOKABLE void shutterReset(){ sendViscaCommand(QByteArray::fromHex("81040A00")); }
    Q_INVOKABLE void shutterUp(){ sendViscaCommand(QByteArray::fromHex("81040A02")); }
    Q_INVOKABLE void shutterDown(){ sendViscaCommand(QByteArray::fromHex("81040A03")); }

    Q_INVOKABLE void expModeAuto(){ sendViscaCommand(QByteArray::fromHex("8101043900")); }
    Q_INVOKABLE void expModeManual(){ sendViscaCommand(QByteArray::fromHex("8101043903")); }
    Q_INVOKABLE void expModeShutterPriority(){ sendViscaCommand(QByteArray::fromHex("810104390A")); }
    Q_INVOKABLE void expModeIrisPriority(){ sendViscaCommand(QByteArray::fromHex("810104390B")); }
    Q_INVOKABLE void expModeGainPriority(){ sendViscaCommand(QByteArray::fromHex("810104390E")); }

    Q_INVOKABLE void inquireExpMode() { sendViscaCommand(QByteArray::fromHex("81090439"), 0x0110); };

    Q_INVOKABLE void irisReset(){ sendViscaCommand(QByteArray::fromHex("810100B00")); }
    Q_INVOKABLE void irisUp(){ sendViscaCommand(QByteArray::fromHex("810100B02")); }
    Q_INVOKABLE void irisDown(){ sendViscaCommand(QByteArray::fromHex("810100B03")); }

    // Exposure compensation
    Q_INVOKABLE void expReset(){ sendViscaCommand(QByteArray::fromHex("8101040E00")); }
    Q_INVOKABLE void expUp(){ sendViscaCommand(QByteArray::fromHex("8101040E02")); }
    Q_INVOKABLE void expDown(){ sendViscaCommand(QByteArray::fromHex("8101040E03")); }

    // WB
    Q_INVOKABLE void wbAuto1(){ sendViscaCommand(QByteArray::fromHex("8101043500")); }
    Q_INVOKABLE void wbIndoor(){ sendViscaCommand(QByteArray::fromHex("8101043501")); }
    Q_INVOKABLE void wbOutdoor(){ sendViscaCommand(QByteArray::fromHex("8101043502")); }
    Q_INVOKABLE void wbOnePush(){ sendViscaCommand(QByteArray::fromHex("8101043503")); }
    Q_INVOKABLE void wbAuto2(){ sendViscaCommand(QByteArray::fromHex("8101043504")); }
    Q_INVOKABLE void wbManual(){ sendViscaCommand(QByteArray::fromHex("8101043505")); }

    Q_INVOKABLE void wbTrigger(){ sendViscaCommand(QByteArray::fromHex("8101041005")); }

    // Focus
    Q_INVOKABLE void focusAuto(){ sendViscaCommand(QByteArray::fromHex("8101043802")); }
    Q_INVOKABLE void focusManual(){ sendViscaCommand(QByteArray::fromHex("8101043803")); }

    Q_INVOKABLE void focusTrigger(){ sendViscaCommand(QByteArray::fromHex("8101041801")); }
    Q_INVOKABLE void focusInfinity(){ sendViscaCommand(QByteArray::fromHex("8101041802")); }

    // Tally
    Q_INVOKABLE void tallyOn(){ sendViscaCommand(QByteArray::fromHex("81017E010A0002")); }
    Q_INVOKABLE void tallyOff(){ sendViscaCommand(QByteArray::fromHex("81017E010A0003")); }

    // Pan-Tilt Control (speed 10 for example)
    Q_INVOKABLE void panTilt(int panSpeed, int tiltSpeed, int panDir, int tiltDir);

    Q_INVOKABLE void panLeft()  { panTilt(10, 10, 1, 3); }
    Q_INVOKABLE void panRight() { panTilt(10, 10, 2, 3); }

    Q_INVOKABLE void panLeftUp()  { panTilt(10, 10, 1, 1); }
    Q_INVOKABLE void panRightDown() { panTilt(10, 10, 2, 2); }

    Q_INVOKABLE void panLeftDown()  { panTilt(10, 10, 1, 2); }
    Q_INVOKABLE void panRightUp() { panTilt(10, 10, 2, 1); }

    Q_INVOKABLE void tiltUp()   { panTilt(10, 10, 3, 1); }
    Q_INVOKABLE void tiltDown() { panTilt(10, 10, 3, 2); }
    Q_INVOKABLE void stopMove() { panTilt(10, 10, 3, 3); }

    Q_INVOKABLE void homePosition(){ sendViscaCommand(QByteArray::fromHex("81010604")); }
    Q_INVOKABLE void resetPosition(){ sendViscaCommand(QByteArray::fromHex("81010605")); }

    // Preset Memory Control
    Q_INVOKABLE void setMemoryPreset(int preset);
    Q_INVOKABLE void recallMemoryPreset(int preset);

    bool isPowered() const;
    bool isConnected() const;

    int zoomPosition() const;

private slots:
    void processIncomingData();

private:
    QUdpSocket *udpSocket;
    QString cameraIP = "192.168.0.100";  // Change this to your camera's IP
    quint16 cameraPort = 52381;          // Default VISCA UDP port
    quint16 localPort = 52381;           // Local port for listening
    quint32 sequenceNumber = 0;              // Sequence counter for commands
    quint32 expectedSeq = 0;
    int timeoutid=0;
    bool m_isPowered=false;
    bool m_isConnected=false;

    QMap<quint32, std::function<void(QByteArray &data)>>m_cbmap;

    void parseResponse(const QByteArray &response, quint32 seq);

    int m_zoomPosition;

signals:
    void connected();
    void networkError(int error);
    void timeoutError();
    void commandError(int error);
    void isPoweredChanged();
    void isConnectedChanged();
    void zoomPositionChanged();

protected:
    void timerEvent(QTimerEvent *event);
};

#endif // VISCAUDPCONTROLLER_H
