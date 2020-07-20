#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <QDialog>
#include <QJsonDocument>
#include <QJsonArray>

#include "Fhir_Server.h"
#include "Person_Information.h"

namespace Ui {
	class Login_Dialog;
}

class Login_Dialog : public QDialog
{
	Q_OBJECT

public:
	explicit Login_Dialog(QWidget *parent = nullptr);
	~Login_Dialog();

	Person_Information Person() const;

private:
	Ui::Login_Dialog *ui;
	Fhir_Server* m_Server;

	QJsonDocument			m_FHIR_Person;
	int				m_Total;
	QMap<int, Person_Information>	m_Entry;
	Person_Information		m_Current;

private slots:
	void Get_User_Lists();
	void on_pushButton_Login_clicked();

};

#endif // LOGIN_DIALOG_H
