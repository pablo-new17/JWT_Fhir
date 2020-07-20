#include <QDebug>
#include <QJsonObject>
#include <QJsonParseError>

#include "Login_Dialog.h"
#include "ui_Login_Dialog.h"



Login_Dialog::Login_Dialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Login_Dialog)
{
	ui->setupUi(this);

	this->m_Server = new Fhir_Server(this);
	connect(this->m_Server, &Fhir_Server::Finished, this, &Login_Dialog::Get_User_Lists);

	this->m_Server->Action= "GET";
	this->m_Server->BaseUrl = "http://203.64.84.213:8080/hapi-fhir-jpaserver/fhir";
//	this->m_Server->BaseUrl = "https://hapi.fhir.org/baseR4";
	this->m_Server->Resource = "Person";

	this->m_Server->Send();
}

Login_Dialog::~Login_Dialog()
{
	delete ui;
}

Person_Information Login_Dialog::Person() const
{
	return this->m_Current;
}

void Login_Dialog::Get_User_Lists()
{
	qDebug() << __LINE__ << "Get_User_Lists()";

	QJsonParseError jsonError;

	this->m_FHIR_Person = QJsonDocument::fromJson(this->m_Server->getResult().toUtf8(), &jsonError);
	if(jsonError.error == QJsonParseError::NoError)
	{
		int Total = this->m_FHIR_Person.object()["total"].toInt();
		QJsonArray all = this->m_FHIR_Person.object()["entry"].toArray();

		for(int Index = 0; Index < Total; Index++)
		{
			QJsonObject person = all[Index].toObject();
			Person_Information info;
			QJsonObject resource = person["resource"].toObject();
			QJsonArray identifier = resource["identifier"].toArray();
			QJsonArray link = resource["link"].toArray();


			if(link.size() == 2)
			{
				info.roles = link[0].toObject()["target"].toObject()["reference"].toString();
				info.sub = link[1].toObject()["target"].toObject()["reference"].toString();
			}

			if(identifier.size() == 2)
			{
				if(identifier[0].toObject()["system"].toString()=="username")
					info.Username = identifier[0].toObject()["value"].toString();
				if(identifier[1].toObject()["system"].toString()=="password")
					info.Password = identifier[1].toObject()["value"].toString();
			}

			info.ID = resource["id"].toString().toInt();

			if(info.ID)
			{
				this->m_Entry.insert(info.ID, info);
				qDebug() << info.ID << info.sub << info.roles;
			}

		}
	}
	else
	{
		qDebug() << "Error: " << jsonError.errorString();
	}

}

void Login_Dialog::on_pushButton_Login_clicked()
{
	foreach(Person_Information info, this->m_Entry.values())
	{
		if(info.Username == ui->lineEdit_Username->text() &&
		   info.Password == ui->lineEdit_Password->text() )
		{
			this->m_Current = info;

			QDialog::accept();
		}
	}
}

