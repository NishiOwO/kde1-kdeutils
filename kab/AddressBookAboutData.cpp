/**********************************************************************

	--- Qt Architect generated file ---

	File: AddressBookAboutData.cpp
	Last generated: Tue Dec 22 23:44:39 1998

	DO NOT EDIT!!!  This file will be automatically
	regenerated by qtarch.  All changes will be lost.

 *********************************************************************/

#include "AddressBookAboutData.h"
#include <kapp.h>

#define Inherited QDialog


AddressBookAboutData::AddressBookAboutData
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name, TRUE, 0 )
{
	frameBase = new QFrame( this, "FrameBase" );
	frameBase->setGeometry( 0, 0, 260, 330 );
	frameBase->setMinimumSize( 10, 10 );
	frameBase->setMaximumSize( 32767, 32767 );
	frameBase->setFrameStyle( 34 );

	labelFrame = new QLabel( this, "Label_4" );
	labelFrame->setGeometry( 10, 10, 240, 260 );
	labelFrame->setMinimumSize( 10, 10 );
	labelFrame->setMaximumSize( 32767, 32767 );
	labelFrame->setFrameStyle( 50 );
	labelFrame->setText( "" );
	labelFrame->setAlignment( 289 );
	labelFrame->setMargin( -1 );

	labelLogo = new QLabel( this, "" );
	labelLogo->setGeometry( 20, 20, 220, 130 );
	labelLogo->setMinimumSize( 10, 10 );
	labelLogo->setMaximumSize( 32767, 32767 );
	labelLogo->setFrameStyle( 34 );
	labelLogo->setText( "" );
	labelLogo->setAlignment( 1316 );
	labelLogo->setMargin( -1 );

	labelVersion = new QLabel( this, "Label_2" );
	labelVersion->setGeometry( 20, 150, 220, 30 );
	labelVersion->setMinimumSize( 10, 10 );
	labelVersion->setMaximumSize( 32767, 32767 );
	{
		QColorGroup normal( QColor( QRgb(0) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8388608) ), QColor( QRgb(16777215) ) );
		QColorGroup disabled( QColor( QRgb(8421504) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(8421504) ), QColor( QRgb(12632256) ) );
		QColorGroup active( QColor( QRgb(0) ), QColor( QRgb(12632256) ), QColor( QRgb(16777215) ), QColor( QRgb(6316128) ), QColor( QRgb(10789024) ), QColor( QRgb(0) ), QColor( QRgb(16777215) ) );
		QPalette palette( normal, disabled, active );
		labelVersion->setPalette( palette );
	}
	labelVersion->setText( i18n("kab 1.0, May 1999") );
	labelVersion->setAlignment( 292 );
	labelVersion->setMargin( -1 );

	labelAuthor = new QLabel( this, "Label_3" );
	labelAuthor->setGeometry( 20, 180, 220, 30 );
	labelAuthor->setMinimumSize( 10, 10 );
	labelAuthor->setMaximumSize( 32767, 32767 );
	labelAuthor->setText( "" );
	labelAuthor->setAlignment( 292 );
	labelAuthor->setMargin( -1 );

	buttonOK = new QPushButton( this, "PushButton_1" );
	buttonOK->setGeometry( 10, 280, 240, 40 );
	buttonOK->setMinimumSize( 10, 10 );
	buttonOK->setMaximumSize( 32767, 32767 );
	connect( buttonOK, SIGNAL(clicked()), SLOT(accept()) );
	buttonOK->setText( "" );
	buttonOK->setAutoRepeat( FALSE );
	buttonOK->setAutoResize( FALSE );

	urlName = new KURLLabel( this, "URLLabel" );
	urlName->setGeometry( 20, 210, 220, 40 );
	urlName->setMinimumSize( 10, 10 );
	urlName->setMaximumSize( 32767, 32767 );

	resize( 260,330 );
	setMinimumSize( 0, 0 );
	setMaximumSize( 32767, 32767 );
}


AddressBookAboutData::~AddressBookAboutData()
{
}
