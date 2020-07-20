#include <QDateTime>

#include "Dialog_JWT_Encode.h"
#include "ui_Dialog_JWT_Encode.h"

#define JWT_SECRET "test235"

Dialog_JWT_Encode::Dialog_JWT_Encode(Person_Information info, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Dialog_JWT_Encode)
{
	ui->setupUi(this);

	this->m_Person = info;

	ui->lineEdit_roles->setText(this->m_Person.roles);
	ui->lineEdit_sub->setText(this->m_Person.sub);
	QString start_time = QString::number(QDateTime::currentSecsSinceEpoch());
	QString expire_time = QString::number(QDateTime::currentSecsSinceEpoch() + 300000);
	ui->lineEdit_iat->setText(start_time);
	ui->lineEdit_ndf->setText(start_time);
	ui->lineEdit_exp->setText(expire_time);

	this->m_Token.setSecret(JWT_SECRET);
	this->m_Token.setHeaderQStr("{"
				    "	\"alg\": \"HS256\","
				    "	\"typ\": \"JWT\""
				    "}");
	this->m_Token.appendClaim("iss", ui->lineEdit_iss->text());
	this->m_Token.appendClaim("aud", ui->lineEdit_aud->text());
	this->m_Token.appendClaim("sub", ui->lineEdit_sub->text());
	this->m_Token.appendClaim("roles", ui->lineEdit_roles->text());
	this->m_Token.appendClaim("iat", ui->lineEdit_iat->text());
	this->m_Token.appendClaim("ndf", ui->lineEdit_ndf->text());
	this->m_Token.appendClaim("exp", ui->lineEdit_exp->text());

	this->m_Token.appendClaim("scope", ui->textEdit_scope->toPlainText());
	this->Token = this->m_Token.getToken();
	ui->textEdit_Token->setText(this->Token);
}

Dialog_JWT_Encode::~Dialog_JWT_Encode()
{
	delete ui;
}

void Dialog_JWT_Encode::on_pushButton_clicked()
{
	this->m_Token.appendClaim("scope", ui->textEdit_scope->toPlainText());
	this->Token = this->m_Token.getToken();
	ui->textEdit_Token->setText(this->Token);
}
