/***************************************************************************
                          timedialog.h  -  K Desktop Planetarium
                             -------------------
    begin                : Sun Feb 11 2001
    copyright            : (C) 2001 by Jason Harris
    email                : jharris@30doradus.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TIMEDIALOG_H
#define TIMEDIALOG_H

#include <kdialogbase.h>
#include <qdatetime.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

class QHBoxLayout;
class QVBoxLayout;
class KDatePicker;
class QSpinBox;
class QLabel;
class QPushButton;
class KStars;

/**Dialog for adjusting the Time and Date.  Contains a KDatePicker widget
	*for selecting the date, and three QSpinBoxes for selecting the Hour,
	*Minute and Second.  There is also a "Now" QPushbutton for selecting the
	*Time and Date from the system clock.
	*@short Dialog for adjusting the Time and Date.
	*@author Jason Harris
	*@version 0.9
	*/

class TimeDialog : public KDialogBase {
  Q_OBJECT
public:
/**
	*Constructor.  Creates widgets and packs them into QLayouts.
	*Connects	Signals and Slots.
	*/
	TimeDialog( QDateTime now, QWidget* parent = 0, bool isUTCNow = false );

/**
	*Destructor (empty)
	*/
	~TimeDialog() {}

/**@returns a QTime object with the selected time
	*/
	QTime selectedTime( void );

/**@returns a QDate object with the selected date
	*/
	QDate selectedDate( void );

/**@returns a QDateTime object with the selected date and time
	*/
	QDateTime selectedDateTime( void );

public slots:
/**
	*When the "Now" QPushButton is pressed, read the time and date
	*from the system clock.  Change the selected date in the KDatePicker
	*to the system's date, and the displayed Hour, Minute and Second
	*to the system time.
	*/
  void setNow( void );

/**
	*When the value of the HourBox QSpinBox is changed, prefix a "0" to
	*the displayed text, if the value is less than 10.
	*
	*It would be nice if I could use one slot for these three widgets;
	*my understanding is that the slot has no knowledge of which
	*widget sent the signal...
	*/
	void HourPrefix( int value );

/**
	*When the value of the MinuteBox QSpinBox is changed, prefix a "0" to
	*the displayed text, if the value is less than 10.
	*/
	void MinutePrefix( int value );

/**
	*When the value of the SecondBox QSpinBox is changed, prefix a "0" to
	*the displayed text, if the value is less than 10.
	*/
	void SecondPrefix( int value );

protected:
   bool event( QEvent* );
	void keyPressEvent( QKeyEvent* );

private:
  KStars *ksw;
  bool UTCNow;
  QHBoxLayout *hlay;
  QVBoxLayout *vlay;
  KDatePicker *dPicker;
  QSpinBox* HourBox;
  QLabel* TextLabel1;
  QSpinBox* MinuteBox;
  QLabel* TextLabel1_2;
  QSpinBox* SecondBox;
  QPushButton* NowButton;
};

#endif
