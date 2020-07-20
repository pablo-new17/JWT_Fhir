#ifndef SCOPE_INFORMATION_H
#define SCOPE_INFORMATION_H

#include <QString>
#include <QMap>

class Scope_Information
{
public:
	Scope_Information();

	QString			Action;

	QString			BaseURL;
	QString			Resource;
	QString			ID;

	QMap<QString, QString>	Parameters;
};

#endif // SCOPE_INFORMATION_H
