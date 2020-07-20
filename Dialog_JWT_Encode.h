#ifndef DIALOG_JWT_ENCODE_H
#define DIALOG_JWT_ENCODE_H

#include <QDialog>

#include "Person_Information.h"
#include "qjsonwebtoken.h"

namespace Ui {
	class Dialog_JWT_Encode;
}

class Dialog_JWT_Encode : public QDialog
{
	Q_OBJECT

public:
	explicit Dialog_JWT_Encode(Person_Information info, QWidget *parent = nullptr);
	~Dialog_JWT_Encode();

	QString Token;

private slots:
	void on_pushButton_clicked();

private:
	Ui::Dialog_JWT_Encode*	ui;
	Person_Information	m_Person;
	QJsonWebToken		m_Token;
};

#endif // DIALOG_JWT_ENCODE_H
