/*
 *   khexdit - a little hex editor
 *   Copyright (C) 1996,97  Stephan Kulow
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "hexfile.h"
#include <qfile.h> 
#include <qwidget.h>
#include <qfontmet.h> 
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qkeycode.h>
#include <qmsgbox.h> 
#include <kapp.h>

#include "hexfile.moc"

static QBrush RedMarker(QColor(0xff,0,0));
static QBrush GreenMarker(QColor(0xe0,0xff,0xe0));

HexFile::HexFile(const char *filename,QWidget *parent, const char* name) :  
    QWidget(parent,name)
{
    init();
    open(filename);
}

HexFile::HexFile(QWidget *parent)  : QWidget(parent,"Nothing") {
    init();
    filename=new char[20];
    strcpy(filename,klocale->translate("Untitled"));
}

void HexFile::init() {
    UseBig = false;
    sideEdit = LEFT;
    pixmap=new QPixmap();
    dispFont=new QFont("courier", 14);
    filename = 0L;
    setFont(*dispFont);
    setBackgroundColor( QColor( 220, 220, 220));
    metrics = new QFontMetrics(fontMetrics());
    cursorHeight = metrics->ascent();
    maxWidth = 0;
    rows = 0;
    relcur = curx = cury = 0;
    char lineB[]="ABCDEF0123456789";
    char lineL[]="abcdef0123456789";
    for (size_t i=0;i<strlen(lineB);i++) {
	char t = (UseBig) ? lineB[i] : lineL[i];
	if (maxWidth < metrics->width(t))
	    maxWidth = metrics->width(t);
    }
    hexdata = 0;
    data_size = 0;
    currentByte = 0;
    LineOffset=maxWidth*49+10;
    labelOffset = 0;
    curx = cury = horoff=lineoffset=0;
    setFocusPolicy(QWidget::TabFocus);
    leftM = &RedMarker;
    rightM = &GreenMarker;
    modified = false;
    emit unsaved( false );
    scrollV=new QScrollBar(QScrollBar::Vertical,this);
    scrollV->show();
    scrollH=new QScrollBar(QScrollBar::Horizontal,this);
    scrollH->show();
    show();
}

HexFile::~HexFile() {
    delete dispFont;
    delete pixmap;
    delete metrics;
    delete scrollV;
    delete scrollH;
    delete [] filename;
    for (HexCursor *obj = rects.first(); obj; obj = rects.next())
	delete obj;
}

const char* HexFile::Title() {
    return filename;
}

int HexFile::save() {
    QMessageBox::message("Broken", "Saving is terrible broken.\nI disabled it.");
    return 0;
    QFile file(filename);
    file.open(IO_Truncate | IO_WriteOnly | IO_Raw);
    file.writeBlock(hexdata, data_size);
    file.close();
    modified = false;
    emit unsaved( false );
    return 0;
}

void HexFile::setFileName(const char *Filename) {
    delete [] filename;
    filename = new char[strlen(Filename)+1];
    strcpy(this->filename, Filename);
}

bool HexFile::open(const char *Filename) {
    QString fileString(Filename);
    QFile file(fileString);
    if (!file.open(IO_ReadOnly | IO_Raw)) {
	QString txt;
	txt.sprintf(klocale->translate("Error opening %s"),fileString.data());
	QMessageBox::message(klocale->translate("Error"),txt,
			     klocale->translate("Close"));
	return false;
    }
    delete [] filename;
    filename = new char[strlen(Filename)+1];
    strcpy(this->filename,Filename);
    hexdata = (char*)mmap(0, file.size(),  PROT_READ | PROT_WRITE, MAP_PRIVATE,
			  file.handle(), 0);
    data_size = file.size();
    lineoffset = 0;
    curx = 0;
    cury = 0;
    calcScrolls();
    fillPixmap();
    repaint( false );
    modified = false;
    emit unsaved( false );
    return true;
}

void HexFile::calcScrolls() {
    scrollV->setRange(0,maxLine());
    scrollV->setSteps(1,lines());
    scrollV->setValue(lineoffset/16);
}

int HexFile::maxLine() {
    int ml=(data_size / 16) - rows + 3;
    return ((ml < 0) ? 0 : ml);
}

int HexFile::lines() {
    return rows;
}

void HexFile::scrolled(int line) {
    static int old_x = -1 , old_y = -1;
    int offset = line*16;
    if (lineoffset == offset && old_y == cury && old_x == curx) 
	return;
    lineoffset=offset;
    old_x = curx;
    old_y = cury;
    if (!pixmap->isNull()) {
	fillPixmap();
	repaint( false );
    }
}

void HexFile::moved(int value) {
    horoff=value;
    repaint( false );
}

int HexFile::NormWidth() {
    return maxWidth*65+25;
}

int HexFile::HorOffset() {
    return horoff;
}

const char *HexFile::FileName() {
    return filename;
}

int hexvalue(int key) {
    if (key>='a' && key<='f')
	return key-'a'+10;
    if (key>='A' && key<='F')
	return key-'A'+10;
    return key-'0'; 
}

void HexFile::keyPressEvent (QKeyEvent* e) {
    int ox=curx;
    int oy=cury;
    int new_lineoffset = lineoffset;
    int ol=lineoffset;
    bool changed = false;
    int or=relcur;
    int key = e->ascii();
    if (sideEdit == LEFT) {
	if ((key>='a' && key<='f') || 
	    (key>='A' && key<='F') || 
	    (key>='0' && key<='9')) {
	    
	    int r =(unsigned char)hexdata[cury*16+curx+lineoffset];
	    
	    if (relcur) 
		hexdata[cury*16+curx+new_lineoffset] = r & 0xf0 | hexvalue(key);
	    else 
		hexdata[cury*16+curx+new_lineoffset] = r & 0x0f | hexvalue(key) << 4;
	    modified = changed = true;
	    emit unsaved( true );
	    relcur++;
	    if (relcur==2) {
		relcur = 0;
		curx++;
	    }
	} else
	    relcur = 0;
    } else if (key && e->key()<0x100) {
	hexdata[cury*16+curx+new_lineoffset] = key;
	modified = changed = true;
	emit unsaved( true );
	curx++;
    }
    key = e->key();
    switch (key) {
    case Key_Right:
	curx++;
	break;
    case Key_Left:
	curx--;
	break;
    case Key_Up:
	cury--;
	break;
    case Key_Down:
	cury++;
	break;
    case Key_Next:
	scrollV->addPage();
	break;
    case Key_Prior:
	scrollV->subtractPage();
	break;
    case Key_Tab:
    case Key_Backtab:
	changeSide();
	changed=true;
	break;
    }
    if (curx<0) {
	if (new_lineoffset+cury>0) {
	    curx=15;
	    cury--;
	} else 
	    curx=0;
    }
    if (curx>15) {
	curx=0;
	cury++;
    }
    if (cury<0) {
	cury=0;
	if (new_lineoffset>=16) {
	    new_lineoffset-=16;
	}
    }
    if (cury>=rows) {
	cury=rows-1;
	if (new_lineoffset/16<maxLine()) {
	    new_lineoffset +=16;
	}
    }

    if ((unsigned)(curx + new_lineoffset + cury *16) >= data_size) {
	curx = ox;
	cury = oy;
	new_lineoffset = ol;
	relcur = or;
    }
    if (new_lineoffset != ol) {
	scrolled(new_lineoffset/16);
	return;
    }
    if (curx != ox || cury != oy || changed || or!=relcur) {
	fillPixmap();
	repaint( false );
	return;
    }
    
    e->ignore();
}

void HexFile::paintCursor(QPainter& p) 
{
    if (!hexdata)
	return;
    
    char number[3]= "  ";
    char hilight[3];
    int w = calcPosition(curx);
    debug("paintCursor %d %d",w, cursorPosition);
    int offw = 0;

    if (UseBig)
	sprintf(number,"%02X",hexdata[currentByte]);
    else 
	sprintf(number,"%02x",hexdata[currentByte]);

    if (sideEdit == LEFT) {
	p.setPen(QColor(0xff,0xff,0xff));
	offw = maxWidth*relcur;
	
	hilight[0] = number[relcur];
	hilight[1] = 0;
    } else {
	p.setPen(QColor(0x0,0x0,0x0));
	hilight[0] = number[0];
	hilight[1] = number[1];
	hilight[2] = 0;
    }

    p.fillRect(w + offw, cursorPosition - cursorHeight ,
	       (1 + (sideEdit==RIGHT))*maxWidth,
	       cursorHeight + metrics->underlinePos(),
	       *leftM);
    p.drawText(w+offw,cursorPosition, hilight);

    p.setPen(QColor(0xff,0x0,0));
    p.fillRect(LineOffset+10+curx*maxWidth,
	       cursorPosition - cursorHeight +
	       2 * metrics->underlinePos(),
	       maxWidth,
	       cursorHeight,
	       *rightM);
    if (hexdata[currentByte] > 31)
	hilight[0] = hexdata[currentByte];
    else 
	hilight[0] = '.';
    hilight[1] = 0;

    p.setPen(QColor(0x20,0x20,0x80));
    p.drawText(LineOffset+10 + curx * maxWidth, cursorPosition, hilight);
}

void HexFile::changeSide() {
    sideEdit = (Side)(LEFT + RIGHT - sideEdit);
    QBrush *tmp=rightM;
    rightM = leftM;
    leftM = tmp;
}

void HexFile::calcCurrentByte() {
    currentByte = (LineOffset + cury) * 16 + curx;
}

void HexFile::mousePressEvent (QMouseEvent *e) {
    if (hexdata == 0)
	return;
    int neux ,neuy, mx ,cx;
    neuy = e->pos().y() / metrics->height();
    neux = 0;
    
    mx = e->pos().x();
    if (mx < LineOffset) {
	cx = 5 + 9*maxWidth;
	while ( (mx - cx)>4*maxWidth && neux<14) {
	    neux += 2;
	    cx += 5*maxWidth;
	}
	if (mx - cx > 2*maxWidth)
	    neux++;
	if (sideEdit != LEFT)
	    changeSide();
	
    } else {
	cx = LineOffset + 10;
	while (mx > cx) {
	    char r = hexdata[neuy*16+neux+lineoffset];
	    if (r<32)
		r='.';
	    cx += metrics->width(r);
	    neux++;
	}
	if (neux)
	    neux--;
	if (sideEdit != RIGHT)
	    changeSide();
    }
    cury = neuy;
    curx = neux;
    fillPixmap();
    repaint( false );
}

void HexFile::mouseReleaseEvent (QMouseEvent*) {
    /* TODO: for copy and paste */
}

void HexFile::focusInEvent ( QFocusEvent *) {
    repaint( false );
}

void HexFile::focusOutEvent ( QFocusEvent *) {
    repaint(false);
}

int HexFile::paintLabel( QPainter& p, long int label, int y) {
    char offset[10];
    char txt[2] = " ";

    if (UseBig)
	sprintf(offset,"%08lX ", label);
    else
	sprintf(offset,"%08lx ", label);

    p.setPen(QColor(0,0,0));
    for (int i=0;i<9;i++) {
	// to write a single character (perhaps easier?)
	txt[0]=offset[i];
	p.drawText(5+i*maxWidth, y, txt);
    }

    return 5 + 9 * maxWidth;
}

int HexFile::calcPosition( int field ) {
    return field * 2 * maxWidth + labelOffset + (field / 2) * maxWidth;
}

QColor HexFile::colorPosition(int field) {
    if ((field/2) % 2)
	return QColor(0x9f,0x9f,0x20);
    else
	return QColor(0x20,0x9f,0x9f);
}

int HexFile::fillLine(QPainter& p, int line) {
    int w=0, x, i;
    char txt[2]=" ";
    char number[8];
    char currentBuffer[17];
    memset(currentBuffer,' ',17);
    
    if (data_size<=(unsigned)(line-1)*16+lineoffset)
	return -1;

    int label = paintLabel(p, (line-1)*16 + lineoffset, line*metrics->height());
    if (!labelOffset)
	labelOffset = label;

    for (x = 0; x < 16; x++) {
	
	unsigned int r1;

	if (data_size<=(unsigned)(line-1)*16+x+lineoffset)
	    r1 = 0;
	else
	    r1=(unsigned char)hexdata[(line-1)*16+x+lineoffset];

	if (UseBig)
	    sprintf(number,"%02X",r1);
	else 
	    sprintf(number,"%02x",r1);

	p.setPen(colorPosition(x));
	
	txt[1]=0; w = calcPosition(x);
	for (i=0;i<2;i++) {
	    txt[0]=number[i];
	    p.drawText( w + i * maxWidth, line*metrics->height(), txt);
	}
	
	if ((cury == line-1)  && (curx == x)) {
	    cursorPosition = line*metrics->height();
	}
	    
	if (r1>31)
	    currentBuffer[x]=r1;
	else 
	    currentBuffer[x]='.';
    }
    if (x) {
	currentBuffer[16]=0;
	p.setPen(QColor(0x20,0x20,0x80));
	txt[1]=0; 
	w = LineOffset + 10;
	int h = line*metrics->height();
	
	for (i=0;i<16;i++) {
	    txt[0]=currentBuffer[i];
	    p.drawText( w + i * maxWidth, h, txt);
	}
	p.drawText(LineOffset+10, h, currentBuffer);
	
    } else return -1;

    return 0;
}

void HexFile::fillPixmap() {
    debug("fillPixmap");
    if (!pixmap || pixmap->isNull())
	return;
    QPainter p(pixmap);
    p.setFont(*dispFont);
    pixmap->fill(QColor(220, 220, 220));
    int stat = 0;

    for (int y=1; y<=rows && !stat; y++) 
	stat = fillLine(p, y);
	
    p.drawLine(LineOffset,0,LineOffset,height());
    calcCurrentByte();
    
}

void HexFile::paintEvent(QPaintEvent *p) {
    if (pixmap && !pixmap->isNull() && pixmap->rect().intersects(p->rect()))
	bitBlt(this, p->rect().left(),p->rect().top(), 
	       pixmap, p->rect().left()+horoff,p->rect().top(),
	       p->rect().width(),p->rect().height(),
	       CopyROP);
    QPainter painter(this);
    paintCursor(painter);
}

void HexFile::resizeEvent(QResizeEvent *) {
    int scrollVWidth,scrollHHeight;
    
    if (!scrollV)
	return;
    
    scrollVWidth = scrollHHeight = 15;
    calcScrolls();
    rows=(height() - scrollHHeight - fontMetrics().descent()) / 
	fontMetrics().height() ;
    
    QObject::connect(scrollV, SIGNAL(valueChanged(int)),
		     SLOT(scrolled(int)));
    
    int diff=NormWidth()+scrollVWidth - width();
    if (diff < 0)
	diff = 0;
    scrollH->setRange(0,diff);
    scrollH->setSteps(13,diff);
    scrollH->setValue(0);
    scrollH->setGeometry(0,height()-scrollHHeight,
			 width()-scrollVWidth,scrollHHeight);
    QObject::connect(scrollH, SIGNAL(valueChanged(int)),
		     SLOT(moved(int)));
    
    scrollV->setGeometry(width()-scrollVWidth,0,
			 scrollVWidth,
			 height()-scrollHHeight);
    
    pixmap->resize(maxWidth*65+20,height()-scrollHHeight);
    fillPixmap();
    if (width()>pixmap->width()+scrollHHeight)
	horoff=0;
}



