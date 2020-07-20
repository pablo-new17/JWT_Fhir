#ifndef MYPOSTMAN_H
#define MYPOSTMAN_H

#include <QMainWindow>

#include "common.h"
#include "Scope_Information.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MyPostman; }
QT_END_NAMESPACE

class MyPostman : public QMainWindow
{
    Q_OBJECT

public:
    MyPostman(QString Token, QWidget *parent = nullptr);
    ~MyPostman();

    void Request_POST(QString BaseUrl);
    void Request_GET(QString BaseUrl);
    void Request_PUT(QString BaseUrl);
    void Request_DELETE(QString BaseUrl);

    void TableViewInit(); //初始化表头

    void TableViewInitEmpty();  // 初始化表头，但不新建空行

    void TableClear();      //清空tableview的数据


private slots:
    void on_btn_send_clicked();

    void on_btn_add_clicked();

    void on_btn_clear_clicked();

    void on_btn_delete_clicked();


    void httpReply(QNetworkReply * reply);

    void on_tableView_Params_doubleClicked(const QModelIndex &index);

    void on_tableView_Body_doubleClicked(const QModelIndex &index);

    void on_tableView_Headers_doubleClicked(const QModelIndex &index);


    void on_checkBox_JSON_clicked(bool checked);

private:
    Ui::MyPostman *ui;
    QNetworkAccessManager *m_accessManager;

    QStandardItemModel * ParamModel;
    QStandardItemModel * BodyModel;
    QStandardItemModel * HeaderModel;

    QList<Scope_Information>	m_scope_urls;

private slots:
    bool Date_Check(QString scope_date, QString input_date);
    bool Patient_Check(QString scope_patient, QString input_patient);
    bool Requester_Check(QString scope_requester, QString input_requester);

    void scope_urls(QString Action);
};
#endif // MYPOSTMAN_H
