#include <QMessageBox>
#include <QDebug>

#include "mypostman.h"
#include "ui_mypostman.h"
#include "common.h"
#include "qjsonwebtoken.h"
#include "Fhir_Server.h"


MyPostman::MyPostman(QString Token, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MyPostman)
{
	ui->setupUi(this);

	QJsonWebToken jwt;
	jwt.setToken(Token);

	QJsonDocument payload = jwt.getPayloadJDoc();

	ui->textEdit_Scope->setText(payload["scope"].toString());


	ui->textEdit_Post->hide();
	TableViewInit(); //初始化表头

	m_accessManager = new QNetworkAccessManager(this);
	connect(m_accessManager,SIGNAL(finished(QNetworkReply *)),this,SLOT(httpReply(QNetworkReply*)));
}

MyPostman::~MyPostman()
{
    delete ui;
}


void MyPostman::on_btn_add_clicked()
{
	int index = ui->tabWidget->currentIndex();

	QList<QStandardItem *>item;
	QStandardItem * item1 = new QStandardItem("");
	QStandardItem * item2 = new QStandardItem("");
	item<<item1<<item2;

	if(index == 0)  //Params
	{
	    ParamModel->appendRow(item);
	    ui->tableView_Params->repaint();
	}
	else if(index == 1)  //Body
	{
	    BodyModel->appendRow(item);
	    ui->tableView_Body->repaint();
	}
	else if(index == 2)  //Headers
	{
	    HeaderModel->appendRow(item);
	    ui->tableView_Headers->repaint();
	}
}

void MyPostman::on_btn_send_clicked()
{
	QString type = ui->comboBox_prot->currentText();

	QString BaseUrl = ui->lineEdit_request->text();
	BaseUrl += "/" + ui->comboBox_httpType->currentText();
	if(!ui->lineEdit_id->text().isEmpty())
		BaseUrl += "/" + ui->lineEdit_id->text();

	if(type == "POST")
	{
		Request_POST(BaseUrl);
	}
	else if (type == "GET")
	{
		Request_GET(BaseUrl);
	}
	else if (type == "PUT")
	{
		Request_PUT(BaseUrl);
	}
	else if (type == "DELETE")
	{
		Request_DELETE(BaseUrl);
	}
}

void MyPostman::on_btn_clear_clicked()
{
    int index = ui->tabWidget->currentIndex();
    if(index == 0)  //Params
    {
        ParamModel->clear();
        ui->tableView_Params->setModel(ParamModel);
        ui->tableView_Params->repaint();
    }
    else if(index == 1)  //Body
    {
        BodyModel->clear();
        ui->tableView_Params->setModel(BodyModel);
        ui->tableView_Body->repaint();
    }
    else if(index == 2)  //Headers
    {
        HeaderModel->clear();
        ui->tableView_Params->setModel(HeaderModel);
        ui->tableView_Headers->repaint();
    }

    TableViewInit();    //重新初始化表头
}

void MyPostman::on_btn_delete_clicked()
{
    int index = ui->tabWidget->currentIndex();

    if(index == 0)  //Params
    {
        int count = ui->tableView_Params->currentIndex().row();
        int rows = ParamModel->rowCount();
        if(rows == 1)
        {
            return;
        }
        ParamModel->removeRow(count);
        ui->tableView_Params->repaint();

    }
    else if(index == 1)  //Body
    {
        int count = ui->tableView_Body->currentIndex().row();
        int rows = BodyModel->rowCount();
        if(rows == 1)
        {
            return;
        }
        BodyModel->removeRow(count);
        ui->tableView_Body->repaint();
    }
    else if(index == 2)  //Headers
    {
        int count = ui->tableView_Headers->currentIndex().row();
        int rows = HeaderModel->rowCount();
        if(rows == 1)
        {
            return;
        }
        HeaderModel->removeRow(count);
        ui->tableView_Headers->repaint();
    }
}



void MyPostman::Request_GET(QString BaseUrl)
{
	this->scope_urls("GET");

	//check scope
	bool scope_match = false;
	foreach(Scope_Information GET, this->m_scope_urls)
	{
		scope_match = false;
		if(GET.BaseURL == ui->lineEdit_request->text())
		{
			scope_match = true;
		}
		if(scope_match)
		{
			scope_match &= (GET.Resource == ui->comboBox_httpType->currentText());
		}

		if(scope_match && GET.Resource == "Patient")
		{
			scope_match &= (GET.ID == ui->lineEdit_id->text());
		}
		if(scope_match && GET.Resource == "MedicationRequest")
		{
			QString patient;
			QString date;
			QString patient_input;
			QString date_input;

			if(GET.Parameters.keys().contains("patient"))	patient = GET.Parameters.value("patient");
			if(GET.Parameters.keys().contains("date"))	date = GET.Parameters.value("date");

			int count = ui->tableView_Params->model()->rowCount();
			for(int i=0;i<count;i++)
			{
				QModelIndex name = ParamModel->index(i,0,QModelIndex());
				QModelIndex value = ParamModel->index(i,1,QModelIndex());
				QString strName = name.data().toString();
				QString strValue = value.data().toString();
				if(strName == "" || strName == nullptr)	continue;

				if(strName == "patient")	patient_input = strValue;
				if(strName == "date")		date_input = strValue;
			}

			//check patient
			scope_match &= this->Patient_Check(patient, patient_input);

			//check date
			scope_match &= this->Date_Check(date, date_input);
		}

		//for debug only
		if(scope_match)
		{
			break;
		}
		else
		{
			qDebug() << GET.BaseURL << ui->lineEdit_request->text();
			qDebug() << GET.Resource << ui->comboBox_httpType->currentText();
			qDebug() << GET.ID << ui->lineEdit_id->text();
		}
	}

	if(!scope_match)
	{
		QMessageBox::information(this, "Out of scope", "scope not match");
		return;
	}


	QNetworkRequest request;
	int count = ui->tableView_Params->model()->rowCount();

	bool first = true;
	for(int i=0;i<count;i++)
	{
		QModelIndex name = ParamModel->index(i,0,QModelIndex());
		QModelIndex value = ParamModel->index(i,1,QModelIndex());
		QString strName = name.data().toString();
		QString strValue = value.data().toString();
		if(strName == "" || strName == nullptr)
		{
			continue;   //空数据不发给服务器
		}

		BaseUrl += (first==true)?"?":"&";
		BaseUrl += strName + "=" + strValue;
		first = false;
	}

	request.setUrl(QUrl(BaseUrl));
	m_accessManager->get(request);
	qDebug() << __LINE__ << "GET: BaseUrl =" << BaseUrl;
}

void MyPostman::Request_POST(QString BaseUrl)
{
	this->scope_urls("POST");

	//check scope
	bool scope_match = false;
	foreach(Scope_Information POST, this->m_scope_urls)
	{
		scope_match = false;
		if(POST.BaseURL == ui->lineEdit_request->text())
		{
			scope_match = true;
		}
		if(scope_match)
		{
			scope_match &= (POST.Resource == ui->comboBox_httpType->currentText());
		}


		if(scope_match && POST.Resource == "MedicationRequest")
		{
			QString patient;
			QString patient_input;
			QString date;
			QString date_input;
			QString requester;
			QString requester_input;

			if(POST.Parameters.keys().contains("patient"))	patient = POST.Parameters.value("patient");
			if(POST.Parameters.keys().contains("date"))	date = POST.Parameters.value("date");
			if(POST.Parameters.keys().contains("requester"))requester = POST.Parameters.value("requester");

			QJsonParseError jsonError;
			QJsonDocument Post = QJsonDocument::fromJson(ui->textEdit_Post->toPlainText().toUtf8(), &jsonError);
			if(jsonError.error == QJsonParseError::NoError)
			{
				date_input = Post.object()["dosageInstruction"].toArray()[0].toObject()["timing"].toObject()["event"].toArray()[0].toString();
				patient_input = Post.object()["subject"].toObject()["reference"].toString().section("/", 1);
				requester_input = Post.object()["requester"].toObject()["reference"].toString().section("/", 1);
			}
			else
			{
				QMessageBox::information(this, "Post data", "data is not valid JSON format");
				return;
			}

			//check date
			scope_match &= this->Date_Check(date, date_input);

			//check patient
			scope_match &= this->Patient_Check(patient, patient_input);

			//check requester
			scope_match &= this->Requester_Check(requester, requester_input);
		}
		else if(scope_match && POST.Resource == "MedicationAdministration")
		{
			QString patient;
			QString patient_input;
			QString effective_DateTime;
			QString effective_DateTime_input;

			if(POST.Parameters.keys().contains("patient"))			patient = POST.Parameters.value("patient");
			if(POST.Parameters.keys().contains("effective-DateTime"))	effective_DateTime = POST.Parameters.value("effective-DateTime");

			QJsonParseError jsonError;
			QJsonDocument Post = QJsonDocument::fromJson(ui->textEdit_Post->toPlainText().toUtf8(), &jsonError);
			if(jsonError.error == QJsonParseError::NoError)
			{
				effective_DateTime_input = Post.object()["effectiveDateTime"].toString();
				patient_input = Post.object()["subject"].toObject()["reference"].toString().section("/", 1);;
			}

			//check date
			scope_match &= this->Date_Check(effective_DateTime, effective_DateTime_input);

			//check patient
			scope_match &= this->Patient_Check(patient, patient_input);
		}
		else if(scope_match && POST.Resource == "Observation")
		{
			QString patient;
			QString patient_input;
			QString date;
			QString date_input;

			if(POST.Parameters.keys().contains("patient"))	patient = POST.Parameters.value("patient");
			if(POST.Parameters.keys().contains("date"))	date = POST.Parameters.value("date");

			QJsonParseError jsonError;
			QJsonDocument Post = QJsonDocument::fromJson(ui->textEdit_Post->toPlainText().toUtf8(), &jsonError);
			if(jsonError.error == QJsonParseError::NoError)
			{
				date_input = Post.object()["dosageInstruction"].toArray()[0].toObject()["timing"].toObject()["event"].toArray()[0].toString();
				patient_input = Post.object()["subject"].toObject()["reference"].toString().section("/", 1);;
			}

			//check date
			scope_match &= this->Date_Check(date, date_input);

			//check patient
			scope_match &= this->Patient_Check(patient, patient_input);
		}


		//for debug only
		if(scope_match)
		{
			break;
		}
		else
		{
			qDebug() << POST.BaseURL << ui->lineEdit_request->text();
			qDebug() << POST.Resource << ui->comboBox_httpType->currentText();
			qDebug() << POST.ID << ui->lineEdit_id->text();
		}
	}

	if(!scope_match)
	{
		QMessageBox::information(this, "Out of scope", "scope not match");
		return;
	}

    QNetworkRequest request;
    QJsonObject obj1;

    bool first = true;
    int count = ui->tableView_Params->model()->rowCount();
    for(int i=0;i<count;i++)
    {
	    QModelIndex name = ParamModel->index(i,0,QModelIndex());
	    QModelIndex value = ParamModel->index(i,1,QModelIndex());
	    QString strName = name.data().toString();
	    QString strValue = value.data().toString();
	    if(strName == "" || strName == nullptr)
	    {
		    continue;   //空数据不发给服务器
	    }

	    BaseUrl += (first==true)?"?":"&";
	    BaseUrl += strName + "=" + strValue;
	    first = false;
    }

//    count = ui->tableView_Params->model()->rowCount();
//    for(int i=0;i<count;i++)
//    {
//        QModelIndex name = ParamModel->index(i,0,QModelIndex());
//        QModelIndex value = ParamModel->index(i,1,QModelIndex());
//        QString strName = name.data().toString();
//        QString strValue = value.data().toString();
//        if(strName.isEmpty())
//        {
//            continue;   //空数据不发给服务器
//        }
//        obj1.insert(strName,strValue);
//    }

    QJsonDocument docu(obj1);
//    docu.setObject;
    QByteArray postData;

    if(ui->checkBox_JSON->isChecked())
    {
	    postData = ui->textEdit_Post->toPlainText().toUtf8();
    }
    else
    {
	   postData = docu.toJson(QJsonDocument::Compact); //组给服务器的参数
    }
    request.setUrl(QUrl(BaseUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    m_accessManager->post(request, postData);
    qDebug() << __LINE__ <<"postData = "<<postData;
}

void MyPostman::Request_PUT(QString BaseUrl)
{
	this->scope_urls("PUT");

	//check scope
	bool scope_match = false;
	foreach(Scope_Information PUT, this->m_scope_urls)
	{
		scope_match = false;
		if(PUT.BaseURL == ui->lineEdit_request->text())
		{
			scope_match = true;
		}
		if(scope_match)
		{
			scope_match &= (PUT.Resource == ui->comboBox_httpType->currentText());
		}

		if(scope_match && PUT.Resource == "MedicationRequest")
		{
			Fhir_Server server;
			server.Action = "GET";
			server.BaseUrl = ui->lineEdit_request->text();
			server.Resource = "MedicationRequest/" + ui->lineEdit_id->text();

			QEventLoop loop;
			connect(&server, &Fhir_Server::Finished, &loop, &QEventLoop::quit);
			server.Send();
			loop.exec();

			QString patient;
			QString patient_input;
			QString date;
			QString date_input;
			QString requester;
			QString requester_input;

			if(PUT.Parameters.keys().contains("patient"))	patient = PUT.Parameters.value("patient");
			if(PUT.Parameters.keys().contains("date"))		date = PUT.Parameters.value("date");
			if(PUT.Parameters.keys().contains("requester"))	requester = PUT.Parameters.value("requester");

			QJsonParseError jsonError;
			QJsonDocument Post = QJsonDocument::fromJson(server.getResult().toUtf8(), &jsonError);
			if(jsonError.error == QJsonParseError::NoError)
			{
				date_input = Post.object()["dosageInstruction"].toArray()[0].toObject()["timing"].toObject()["event"].toArray()[0].toString();
				patient_input = Post.object()["subject"].toObject()["reference"].toString().section("/", 1);
				requester_input = Post.object()["requester"].toObject()["reference"].toString().section("/", 1);
			}

			//check date
			scope_match &= this->Date_Check(date, date_input);

			//check patient
			scope_match &= this->Patient_Check(patient, patient_input);

			//check requester
			scope_match &= this->Requester_Check(requester, requester_input);
		}
		else if(scope_match && PUT.Resource == "MedicationAdministration")
		{
			Fhir_Server server;
			server.Action = "GET";
			server.BaseUrl = ui->lineEdit_request->text();
			server.Resource = "MedicationAdministration/" + ui->lineEdit_id->text();

			QEventLoop loop;
			connect(&server, &Fhir_Server::Finished, &loop, &QEventLoop::quit);
			server.Send();
			loop.exec();

			QString patient;
			QString patient_input;
			QString effective_DateTime;
			QString effective_DateTime_input;

			if(PUT.Parameters.keys().contains("patient"))			patient = PUT.Parameters.value("patient");
			if(PUT.Parameters.keys().contains("effective-DateTime"))	effective_DateTime = PUT.Parameters.value("effective-DateTime");


			QJsonParseError jsonError;
			QJsonDocument Post = QJsonDocument::fromJson(server.getResult().toUtf8(), &jsonError);
			if(jsonError.error == QJsonParseError::NoError)
			{
				effective_DateTime_input = Post.object()["effectiveDateTime"].toString();
				patient_input = Post.object()["subject"].toObject()["reference"].toString().section("/", 1);
			}

			//check date
			scope_match &= this->Date_Check(effective_DateTime, effective_DateTime_input);

			//check patient
			scope_match &= this->Patient_Check(patient, patient_input);
		}
		else if(scope_match && PUT.Resource == "Observation")
		{
			Fhir_Server server;
			server.Action = "GET";
			server.BaseUrl = ui->lineEdit_request->text();
			server.Resource = "Observation/" + ui->lineEdit_id->text();

			QEventLoop loop;
			connect(&server, &Fhir_Server::Finished, &loop, &QEventLoop::quit);
			server.Send();
			loop.exec();

			QString patient;
			QString patient_input;
			QString date;
			QString date_input;

			if(PUT.Parameters.keys().contains("patient"))	patient = PUT.Parameters.value("patient");
			if(PUT.Parameters.keys().contains("date"))	date = PUT.Parameters.value("date");

			QJsonParseError jsonError;
			QJsonDocument Post = QJsonDocument::fromJson(server.getResult().toUtf8(), &jsonError);
			if(jsonError.error == QJsonParseError::NoError)
			{
				date_input = Post.object()["dosageInstruction"].toArray()[0].toObject()["timing"].toObject()["event"].toArray()[0].toString();
				patient_input = Post.object()["subject"].toObject()["reference"].toString().section("/", 1);
			}

			//check date
			scope_match &= this->Date_Check(date, date_input);

			//check patient
			scope_match &= this->Patient_Check(patient, patient_input);
		}

		//for debug only
		if(scope_match)
		{
			break;
		}
		else
		{
			qDebug() << PUT.BaseURL << ui->lineEdit_request->text();
			qDebug() << PUT.Resource << ui->comboBox_httpType->currentText();
			qDebug() << PUT.ID << ui->lineEdit_id->text();
		}
	}

	if(!scope_match)
	{
		QMessageBox::information(this, "Out of scope", "scope not match");
		return;
	}

	QNetworkRequest request;

	QJsonObject obj1;
	int count = ui->tableView_Body->model()->rowCount();

	for(int i=0;i<count;i++)
	{
	    QModelIndex name = BodyModel->index(i,0,QModelIndex());
	    QModelIndex value = BodyModel->index(i,1,QModelIndex());
	    QString strName = name.data().toString();
	    QString strValue = value.data().toString();
	    if(strName.isEmpty())
	    {
		continue;   //空数据不发给服务器
	    }
	    obj1.insert(strName,strValue);
	}

	QJsonDocument docu(obj1);
    //    docu.setObject;
	QByteArray postData = docu.toJson(QJsonDocument::Compact); //组给服务器的参数

	request.setUrl(QUrl(BaseUrl));
	request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
	m_accessManager->put(request, postData);
	qDebug() << __LINE__ <<"put Data = "<<postData;
}

void MyPostman::Request_DELETE(QString BaseUrl)
{
	this->scope_urls("DELETE");

	//check scope
	bool scope_match = false;
	foreach(Scope_Information DELETE, this->m_scope_urls)
	{
		scope_match = false;
		if(DELETE.BaseURL == ui->lineEdit_request->text())
		{
			scope_match = true;
		}
		if(scope_match)
		{
			scope_match &= (DELETE.Resource == ui->comboBox_httpType->currentText());
		}

		if(scope_match && DELETE.Resource == "MedicationRequest")
		{
			Fhir_Server server;
			server.Action = "GET";
			server.BaseUrl = ui->lineEdit_request->text();
			server.Resource = "MedicationRequest/" + ui->lineEdit_id->text();

			QEventLoop loop;
			connect(&server, &Fhir_Server::Finished, &loop, &QEventLoop::quit);
			server.Send();
			loop.exec();

			QString patient;
			QString patient_input;
			QString date;
			QString date_input;
			QString requester;
			QString requester_input;

			if(DELETE.Parameters.keys().contains("patient"))	patient = DELETE.Parameters.value("patient");
			if(DELETE.Parameters.keys().contains("date"))		date = DELETE.Parameters.value("date");
			if(DELETE.Parameters.keys().contains("requester"))	requester = DELETE.Parameters.value("requester");

			QJsonParseError jsonError;
			QJsonDocument Post = QJsonDocument::fromJson(server.getResult().toUtf8(), &jsonError);
			if(jsonError.error == QJsonParseError::NoError)
			{
				date_input = Post.object()["dosageInstruction"].toArray()[0].toObject()["timing"].toObject()["event"].toArray()[0].toString();
				patient_input = Post.object()["subject"].toObject()["reference"].toString();
				requester_input = Post.object()["requester"].toObject()["reference"].toString();
			}

			//check date
			scope_match &= this->Date_Check(date, date_input);

			//check patient
			scope_match &= this->Patient_Check(patient, patient_input);

			//check requester
			scope_match &= this->Requester_Check(requester, requester_input);
		}
		else if(scope_match && DELETE.Resource == "MedicationAdministration")
		{
			Fhir_Server server;
			server.Action = "GET";
			server.BaseUrl = ui->lineEdit_request->text();
			server.Resource = "MedicationAdministration/" + ui->lineEdit_id->text();

			QEventLoop loop;
			connect(&server, &Fhir_Server::Finished, &loop, &QEventLoop::quit);
			server.Send();
			loop.exec();

			QString patient;
			QString patient_input;
			QString effective_DateTime;
			QString effective_DateTime_input;

			if(DELETE.Parameters.keys().contains("patient"))			patient = DELETE.Parameters.value("patient");
			if(DELETE.Parameters.keys().contains("effective-DateTime"))	effective_DateTime = DELETE.Parameters.value("effective-DateTime");


			QJsonParseError jsonError;
			QJsonDocument Post = QJsonDocument::fromJson(server.getResult().toUtf8(), &jsonError);
			if(jsonError.error == QJsonParseError::NoError)
			{
				effective_DateTime_input = Post.object()["effectiveDateTime"].toString();
				patient_input = Post.object()["subject"].toObject()["reference"].toString();
			}

			//check date
			scope_match &= this->Date_Check(effective_DateTime, effective_DateTime_input);

			//check patient
			scope_match &= this->Patient_Check(patient, patient_input);
		}
		else if(scope_match && DELETE.Resource == "Observation")
		{
			Fhir_Server server;
			server.Action = "GET";
			server.BaseUrl = ui->lineEdit_request->text();
			server.Resource = "Observation/" + ui->lineEdit_id->text();

			QEventLoop loop;
			connect(&server, &Fhir_Server::Finished, &loop, &QEventLoop::quit);
			server.Send();
			loop.exec();

			QString patient;
			QString patient_input;
			QString date;
			QString date_input;

			if(DELETE.Parameters.keys().contains("patient"))	patient = DELETE.Parameters.value("patient");
			if(DELETE.Parameters.keys().contains("date"))	date = DELETE.Parameters.value("date");

			QJsonParseError jsonError;
			QJsonDocument Post = QJsonDocument::fromJson(server.getResult().toUtf8(), &jsonError);
			if(jsonError.error == QJsonParseError::NoError)
			{
				date_input = Post.object()["effectiveDateTime"].toString();
				patient_input = Post.object()["subject"].toObject()["reference"].toString();
			}

			//check date
			scope_match &= this->Date_Check(date, date_input);

			//check patient
			scope_match &= this->Patient_Check(patient, patient_input);
		}

		//for debug only
		if(scope_match)
		{
			break;
		}
		else
		{
			qDebug() << DELETE.BaseURL << ui->lineEdit_request->text();
			qDebug() << DELETE.Resource << ui->comboBox_httpType->currentText();
			qDebug() << DELETE.ID << ui->lineEdit_id->text();
		}
	}

	if(!scope_match)
	{
		QMessageBox::information(this, "Out of scope", "scope not match");
		return;
	}

	QNetworkRequest request;
	int count = ui->tableView_Body->model()->rowCount();

	for(int i=0;i<count;i++)
	{
	    QModelIndex name = BodyModel->index(i,0,QModelIndex());
	    QModelIndex value = BodyModel->index(i,1,QModelIndex());
	    QString strName = name.data().toString();
	    QString strValue = value.data().toString();
	    if(strName == "" || strName == nullptr)
	    {
		continue;   //空数据不发给服务器
	    }

	    BaseUrl += (i==0)?"?":"&";
	    BaseUrl += strName + "=" + strValue;
	}

	request.setUrl(QUrl(BaseUrl));
	m_accessManager->deleteResource(request);
	qDebug() << __LINE__ << "DELETE: BaseUrl =" << BaseUrl;
}


void MyPostman::httpReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = reply->readAll();
        qDebug()<<bytes;
        QString string = QString::fromUtf8(bytes);

        ui->textEdit_result->setText(string.toUtf8());
    }
    else
    {
        qDebug()<<"handle errors here";

	//statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
        QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

        qDebug() << "请求发生错误 code:" << statusCodeV.toInt();
	qDebug() << static_cast<int>(reply->error()) << ":" << reply->errorString();

        ui->textEdit_result->setText(qPrintable(reply->errorString()));
    }

    reply->deleteLater();
}


//初始化TableView的表头
void MyPostman::TableViewInit()
{
	ParamModel = new QStandardItemModel;
	BodyModel = new QStandardItemModel;
	HeaderModel = new QStandardItemModel;
	QStringList head;
	head<<("key")<<("value");
	ParamModel->setHorizontalHeaderLabels(head);
	BodyModel->setHorizontalHeaderLabels(head);
	HeaderModel->setHorizontalHeaderLabels(head);

	QList<QStandardItem *>item;
	QStandardItem * item1 = new QStandardItem("");
	QStandardItem * item2 = new QStandardItem("");
	item<<item1<<item2;

	ui->tableView_Body->setModel(BodyModel);
	ui->tableView_Body->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	BodyModel->appendRow(item);

	ui->tableView_Params->setModel(ParamModel);
	ui->tableView_Params->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ParamModel->appendRow(item);

	ui->tableView_Headers->setModel(HeaderModel);
	ui->tableView_Headers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	HeaderModel->appendRow(item);
}

//初始化表头，不创建空行
void MyPostman::TableViewInitEmpty()
{
	ParamModel = new QStandardItemModel;
	BodyModel = new QStandardItemModel;
	HeaderModel = new QStandardItemModel;
	QStringList head;
	head<<("key")<<("value");
	ParamModel->setHorizontalHeaderLabels(head);
	BodyModel->setHorizontalHeaderLabels(head);
	HeaderModel->setHorizontalHeaderLabels(head);


	ui->tableView_Body->setModel(BodyModel);
	ui->tableView_Body->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


	ui->tableView_Params->setModel(ParamModel);
	ui->tableView_Params->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


	ui->tableView_Headers->setModel(HeaderModel);
	ui->tableView_Headers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

//清空tableview的数据，不包括表头
void MyPostman::TableClear()
{
	ui->tableView_Body->clearSpans();
	ui->tableView_Params->clearSpans();
	ui->tableView_Headers->clearSpans();
}

void MyPostman::on_tableView_Params_doubleClicked(const QModelIndex &index)
{
	//判断下是不是最后一个列
	int num = index.row();      //这是当前选中的行号
	QString str = index.data().toString();
	int Count = ui->tableView_Params->model()->rowCount();
	if(num == Count-1&& (str != "" || str != nullptr))
	{
	    QList<QStandardItem *>item;
	    QStandardItem * item1 = new QStandardItem("");
	    QStandardItem * item2 = new QStandardItem("");
	    item<<item1<<item2;
	    ParamModel->appendRow(item);
	    ui->tableView_Params->repaint();
	}
}

void MyPostman::on_tableView_Body_doubleClicked(const QModelIndex &index)
{
	//判断下是不是最后一个列
	int num = index.row();      //这是当前选中的行号
	int Count = ui->tableView_Body->model()->rowCount();
	QString str = index.data().toString();
	if(num == Count-1 && (str != "" || str != nullptr))
	{
	    QList<QStandardItem *>item;
	    QStandardItem * item1 = new QStandardItem("");
	    QStandardItem * item2 = new QStandardItem("");
	    item<<item1<<item2;
	    BodyModel->appendRow(item);
	    ui->tableView_Body->repaint();
	}
}

void MyPostman::on_tableView_Headers_doubleClicked(const QModelIndex &index)
{
	//判断下是不是最后一个列
	int num = index.row();      //这是当前选中的行号
	int Count = ui->tableView_Headers->model()->rowCount();
	QString str = index.data().toString();
	if(num == Count-1 &&  (str != "" || str != nullptr))
	{
	    QList<QStandardItem *>item;
	    QStandardItem * item1 = new QStandardItem("");
	    QStandardItem * item2 = new QStandardItem("");
	    item<<item1<<item2;
	    HeaderModel->appendRow(item);
	    ui->tableView_Headers->repaint();
	}
}

void MyPostman::on_checkBox_JSON_clicked(bool checked)
{
    if(checked)
    {
	    ui->textEdit_Post->show();
	    ui->tableView_Body->hide();
    }
    else
    {
	    ui->textEdit_Post->hide();
	    ui->tableView_Body->show();
    }
}



void MyPostman::scope_urls(QString Action)
{
	this->m_scope_urls.clear();

	QJsonParseError jsonError;
	QJsonDocument scope = QJsonDocument::fromJson(ui->textEdit_Scope->toPlainText().toUtf8(), &jsonError);

	if(jsonError.error != QJsonParseError::NoError)
	{
		QMessageBox::warning(this, "Scope error", jsonError.errorString());
		return;
	}


	foreach(auto method, scope.array())
	{
		if(method.toObject()["method"].toString() == Action)
		{
			foreach(auto url, method.toObject()["url"].toArray())
			{
				Scope_Information GET;
				GET.BaseURL = url.toString().section("/", 0, 4);
				QString Remain = url.toString().section("/", 5);
				QString RemainURL = Remain.section("?", 0, 0);
				QString Parameter = Remain.section("?", 1);
				GET.Resource = RemainURL.section("/", 0, 0);
				GET.ID = RemainURL.section("/", 1, 1);

				foreach(QString arg, Parameter.split("&"))
				{
					GET.Parameters.insert(arg.section("=", 0, 0), arg.section("=", 1, 1));
				}

//				qDebug() << info.BaseURL << info.Resource << info.ID << info.Parameters;
				m_scope_urls.append(GET);
			}
		}
	}
}

bool MyPostman::Date_Check(QString scope_date, QString input_date)
{
	if(!scope_date.isEmpty())
	{
		if(input_date.isEmpty())
		{
			QList<QStandardItem *> items;

			items.append(new QStandardItem("date"));
			items.append(new QStandardItem(scope_date));

//			ParamModel->appendRow(items);
			return false;
		}
		else
		{
			if(scope_date.startsWith("gt"))
			{
				QDate time1 = QDate::fromString(scope_date.mid(2), "yyyy-MM-dd");
				QDate time2 = QDate::fromString(input_date.mid(input_date.startsWith("gt")?2:0), "yyyy-MM-dd");

				return (time2 >= time1);
			}
			else if(scope_date.startsWith("lt"))
			{
				QDate time1 = QDate::fromString(scope_date.mid(2), "yyyy-MM-dd");
				QDate time2 = QDate::fromString(input_date.mid(input_date.startsWith("lt")?2:0), "yyyy-MM-dd");

				return (time2 <= time1);
			}
			else
			{
				return (scope_date == input_date);
			}
		}
	}

	return true;
}

bool MyPostman::Patient_Check(QString scope_patient, QString input_patient)
{
	if(!scope_patient.isEmpty())
	{
		if(input_patient.isEmpty())
		{
			QList<QStandardItem *> items;

			items.append(new QStandardItem("patient"));
			items.append(new QStandardItem(scope_patient));

//			ParamModel->appendRow(items);
			return false;
		}
		else
		{
			return  (scope_patient == input_patient);
		}
	}

	return true;
}

bool MyPostman::Requester_Check(QString scope_requester, QString input_requester)
{
	if(!scope_requester.isEmpty())
	{
		if(input_requester.isEmpty())
		{
			QList<QStandardItem *> items;

			items.append(new QStandardItem("requester"));
			items.append(new QStandardItem(scope_requester));

//			ParamModel->appendRow(items);
			return false;
		}
		else
		{
			return  (scope_requester == input_requester);
		}
	}

	return true;
}
