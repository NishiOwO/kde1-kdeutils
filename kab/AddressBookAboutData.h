/**********************************************************************

	--- Qt Architect generated file ---

	File: AddressBookAboutData.h
	Last generated: Mon Nov 16 23:03:47 1998

	DO NOT EDIT!!!  This file will be automatically
	regenerated by qtarch.  All changes will be lost.

 *********************************************************************/

#ifndef AddressBookAboutData_included
#define AddressBookAboutData_included

#include <qdialog.h>
#include <qlabel.h>
#include "kurllabel.h"
#include <qpushbt.h>
#include <qframe.h>

class AddressBookAboutData : public QDialog
{
    Q_OBJECT

public:

    AddressBookAboutData
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~AddressBookAboutData();

public slots:


protected slots:


protected:
    QFrame* frameBase;
    QLabel* labelFrame;
    QLabel* labelLogo;
    QLabel* labelVersion;
    QLabel* labelAuthor;
    QPushButton* buttonOK;
    KURLLabel* urlName;

};

#endif // AddressBookAboutData_included
