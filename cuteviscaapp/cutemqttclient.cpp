#include "cutemqttclient.h"

CuteMqttClient::CuteMqttClient(QObject *parent) : QMqttClient(parent)
{

}

int CuteMqttClient::publish(const QString &topic, const QString &message, int qos, bool retain)
{
    return QMqttClient::publish(QMqttTopicName(topic), message.toUtf8(), qos, retain);
}

void CuteMqttClient::subscribe(const QString &topic)
{
    QMqttClient::subscribe(QMqttTopicFilter(topic));
}

QString CuteMqttClient::topicString(const QMqttTopicName &topic) const {
    return topic.name();
}
