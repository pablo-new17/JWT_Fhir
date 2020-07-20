#include <QNetworkReply>

#include "Fhir_Server.h"

Fhir_Server::Fhir_Server(QObject *parent) : QObject(parent)
{
	this->m_accessManager = new QNetworkAccessManager(this);

	connect(m_accessManager, &QNetworkAccessManager::finished, this, &Fhir_Server::httpReply);

}

void Fhir_Server::Send()
{
	QString Url = this->BaseUrl + "/" + Resource;

	if(!ID.isEmpty())	Url += "/" + ID;

	if(Action == "POST")
	{
		Request_POST(Url);
	}
	else if (Action == "GET")
	{
		Request_GET(Url);
	}
	else if (Action == "PUT")
	{
		Request_PUT(Url);
	}
	else if (Action == "DELETE")
	{
		Request_DELETE(Url);
	}
	else
	{
		this->m_Error = tr("Unknow/Unsupport action");
		emit this->Finished();
	}
}


void Fhir_Server::httpReply(QNetworkReply* reply)
{
	this->m_Result = "";
	this->m_Error = "";

	if (reply->error() == QNetworkReply::NoError)
	{
	    QByteArray bytes = reply->readAll();
//	    qDebug()<<bytes;
	    this->m_Result = QString::fromUtf8(bytes);

	}
	else
	{
	    qDebug()<<"request errors";
	    QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

	    qDebug() << "error code:" << statusCodeV.toInt();
	    qDebug() << static_cast<int>(reply->error()) << ":" << reply->errorString();

	    this->m_Error = qPrintable(reply->errorString());
	    qDebug()<<this->m_Error;
	}

	reply->deleteLater();
	emit this->Finished();
}


void Fhir_Server::Request_GET(QString Url)
{
	QNetworkRequest request;

	bool First = true;
	foreach(QString Key, Parameters.keys())
	{
		QString strName = Key;
		QString strValue = Parameters.value(Key);
		if(strName.isEmpty())	continue;

		Url += (First)?"?":"&";
		Url += strName + "=" + strValue;

		First = false;
	}

	request.setUrl(QUrl(Url));
	m_accessManager->get(request);
	qDebug() << __LINE__ << "GET: Url =" << Url;
}

void Fhir_Server::Request_POST(QString Url)
{

}

void Fhir_Server::Request_PUT(QString Url)
{

}

void Fhir_Server::Request_DELETE(QString Url)
{

}

QString Fhir_Server::getError() const
{
	return m_Error;
}

QString Fhir_Server::getResult() const
{
	return m_Result;
}
