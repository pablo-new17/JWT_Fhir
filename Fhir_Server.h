#ifndef FHIR_SERVER_H
#define FHIR_SERVER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QPair>


class Fhir_Server : public QObject
{
	Q_OBJECT
public:
	explicit Fhir_Server(QObject *parent = nullptr);

	QString		Action;
	QString		BaseUrl;
	QString		Resource;
	QString		ID;

	QMap<QString, QString>	Parameters;
//	QList<QPair<QString, QString>>	Parameters;

public slots:
	void Send();
	QString getResult() const;
	QString getError() const;


signals:
	void Finished();

private slots:
	void httpReply(QNetworkReply* reply);

	void Request_POST(QString Url);
	void Request_GET(QString Url);
	void Request_PUT(QString Url);
	void Request_DELETE(QString Url);

private:
	QNetworkAccessManager*	m_accessManager;
	QString			m_Result;
	QString			m_Error;
};

#endif // FHIR_SERVER_H
