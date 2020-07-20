#include <QApplication>

#include "mypostman.h"
#include "common.h"

#include "Login_Dialog.h"
#include "Person_Information.h"
#include "Dialog_JWT_Encode.h"

void initDB();
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);



	Login_Dialog l;
	l.exec();
	Dialog_JWT_Encode j(l.Person());
	l.close();

	j.exec();
	MyPostman w(j.Token);
	l.close();

	w.show();

	return a.exec();
}
