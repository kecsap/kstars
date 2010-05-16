/***************************************************************************
                          kstarsdb.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : Wed May 12 2010
    copyright            : (C) 2010 by Victor Carbune
    email                : victor.carbune@kdemail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kstarsdb.h"

KStarsDB::KStarsDB()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
}

KStarsDB* KStarsDB::pinstance = 0;

bool KStarsDB::loadDatabase(QString dbfile = KSTARS_DBFILE)
{
    db.setDatabaseName(dbfile);
    return db.open();
}

bool KStarsDB::createDefaultDatabase(QString dbfile)
{
    db.setDatabaseName(dbfile);
    return createDefaultDatabase();
}

bool KStarsDB::createDefaultDatabase()
{
    if (db.open() == false) {
           kDebug() << i18n("Unable to open database file!");

           return false;
    }

    QSqlQuery query(db);

    // Enable Foreign Keys -- just to ensure integrity of the table!
    if (!query.exec("PRAGMA foreign_keys = ON")) {
        qDebug() << query.lastError();
    }

    // Create the Catalog entity
    if (!query.exec(QString("CREATE TABLE ctg (name TEXT, source TEXT, description TEXT)"))) {
        qDebug() << query.lastError();
    }

    // Insert the NGC catalog
    if (!query.exec(QString("INSERT INTO ctg VALUES(\"NGC\", \"www.google.com\", \"Basic KStars IC Catalog\")"))) {
        qDebug() << query.lastError();
    }

    // Create the constellation entity
    if (!query.exec(QString("CREATE TABLE cnst (shortname TEXT, latinname TEXT, boundary TEXT, description TEXT)"))) {
        qDebug() << query.lastError();
    }

    // Create the Deep Sky Object entity
    if (!query.exec(QString("CREATE TABLE dso (ra FLOAT, dec FLOAT, bmag FLOAT, angsize FLOAT, dist FLOAT, type INTEGER, minor FLOAT, major FLOAT, idCNST INTEGER, ") +
                QString("FOREIGN KEY (idCNST) REFERENCES cnst(rowid))"))) {
        qDebug() << query.lastError();
    }

    // Create the Object Designation entity
    if (!query.exec(QString("CREATE TABLE od (designation TEXT, idCTG INTEGER NOT NULL, idDSO INTEGER NOT NULL, ") +
                QString("FOREIGN KEY (idCTG) REFERENCES ctg, FOREIGN KEY (idDSO) REFERENCES dso)"))) {
        qDebug() << query.lastError();
    }

    db.close();
 
    return true;
}

void KStarsDB::migrateData(QString filename = "ngcic.dat")
{

    qDebug() << "GETTING HERE, DUDES!!";
/*
    KSFileReader fileReader;
    if ( ! fileReader.open( "ngcic.dat" ) ) return;

    while ( fileReader.hasMoreLines() ) {
        QString line, con, ss, name, name2, longname;
        QString cat, cat2, sgn;
        float mag(1000.0), ras, a, b;
        int type, ingc, imess(-1), rah, ram, dd, dm, ds, pa;
        int pgc, ugc;
        QChar iflag;

        line = fileReader.readLine();

        //Ignore comment lines
        while ( line.at(0) == '#' && fileReader.hasMoreLines() ) line = fileReader.readLine();

        //Ignore lines with no coordinate values
        while ( line.mid(6,8).trimmed().isEmpty() && fileReader.hasMoreLines() ) {
            line = fileReader.readLine();
        }

        iflag = line.at( 0 ); //check for NGC/IC catalog flag
        if ( iflag == 'I' ) cat = "IC";
        else if ( iflag == 'N' ) cat = "NGC";

        ingc = line.mid( 1, 4 ).toInt();  // NGC/IC catalog number
        if ( ingc==0 ) cat.clear(); //object is not in NGC or IC catalogs

        //coordinates
        rah = line.mid( 6, 2 ).toInt();
        ram = line.mid( 8, 2 ).toInt();
        ras = line.mid( 10, 4 ).toFloat();
        sgn = line.mid( 15, 1 ); //don't use at(), because it crashes on invalid index
        dd = line.mid( 16, 2 ).toInt();
        dm = line.mid( 18, 2 ).toInt();
        ds = line.mid( 20, 2 ).toInt();

        //B magnitude
        ss = line.mid( 23, 4 );
        if (ss == "    " ) { mag = 99.9f; } else { mag = ss.toFloat(); }

        //object type
        type = line.mid( 29, 1 ).toInt();

        //major and minor axes
        ss = line.mid( 31, 5 );
        if (ss == "      " ) { a = 0.0; } else { a = ss.toFloat(); }
        ss = line.mid( 37, 5 );
        if (ss == "     " ) { b = 0.0; } else { b = ss.toFloat(); }
        //position angle.  The catalog PA is zero when the Major axis
        //is horizontal.  But we want the angle measured from North, so
        //we set PA = 90 - pa.
        ss = line.mid( 43, 3 );
        if (ss == "   " ) { pa = 90; } else { pa = 90 - ss.toInt(); }

        //PGC number
        ss = line.mid( 47, 6 );
        if (ss == "      " ) { pgc = 0; } else { pgc = ss.toInt(); }

        //UGC number
        if ( line.mid( 54, 3 ) == "UGC" ) {
            ugc = line.mid( 58, 5 ).toInt();
        } else {
            ugc = 0;
        }

        //Messier number
        if ( line.mid( 70,1 ) == "M" ) {
            cat2 = cat;
            if ( ingc==0 ) cat2.clear();
            cat = 'M';
            imess = line.mid( 72, 3 ).toInt();
        }

        longname = line.mid( 76, line.length() ).trimmed();

        dms r;
        r.setH( rah, ram, int(ras) );
        dms d( dd, dm, ds );

        if ( sgn == "-" ) { d.setD( -1.0*d.Degrees() ); }

        bool hasName = true;
        QString snum;
        if ( cat=="IC" || cat=="NGC" ) {
            snum.setNum( ingc );
            name = cat + ' ' + snum;
        } else if ( cat=="M" ) {
            snum.setNum( imess );
            name = cat + ' ' + snum;
            if ( cat2=="NGC" ) {
                snum.setNum( ingc );
                name2 = cat2 + ' ' + snum;
            } else if ( cat2=="IC" ) {
                snum.setNum( ingc );
                name2 = cat2 + ' ' + snum;
            } else {
                name2.clear();
            }
        }
        else {
            if ( ! longname.isEmpty() ) name = longname;
            else {
                hasName = false;
                name = i18n( "Unnamed Object" );
            }
        }
    }
*/
}

KStarsDB* KStarsDB::Create()
{
    delete pinstance;
    pinstance = new KStarsDB();
    return pinstance;
}

KStarsDB* KStarsDB::Instance()
{
    return pinstance;
}
