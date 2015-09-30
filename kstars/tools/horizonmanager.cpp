/*  Artificial Horizon Manager
    Copyright (C) 2015 Jasem Mutlaq <mutlaqja@ikarustech.com>

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
*/

#include "horizonmanager.h"

#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QHeaderView>

#include <KMessageBox>

#include <config-kstars.h>

#include "Options.h"
#include "kstars.h"
#include "kstarsdata.h"
#include "nan.h"
#include "skymap.h"
#include "projections/projector.h"
#include "linelist.h"
#include "skycomponents/artificialhorizoncomponent.h"
#include "skycomponents/skymapcomposite.h"


HorizonManagerUI::HorizonManagerUI( QWidget *p ) : QFrame( p ) {
    setupUi( this );
}


HorizonManager::HorizonManager( QWidget *w )
        : QDialog( w )
{    

    ui = new HorizonManagerUI( this );

    ui->setStyleSheet("QPushButton:checked { background-color: red; }");

    ui->addRegionB->setIcon(QIcon::fromTheme("list-add"));
    ui->addPointB->setIcon(QIcon::fromTheme("list-add"));
    ui->removeRegionB->setIcon(QIcon::fromTheme("list-remove"));
    ui->removePointB->setIcon(QIcon::fromTheme("list-remove"));
    ui->clearPointsB->setIcon(QIcon::fromTheme("edit-clear"));
    ui->saveB->setIcon(QIcon::fromTheme("document-save"));
    ui->selectPointsB->setIcon(QIcon::fromTheme("snap-orthogonal"));

    ui->tipLabel->setPixmap((QIcon::fromTheme("help-hint").pixmap(64,64)));

    ui->polygonValidatoin->setPixmap(QIcon::fromTheme("process-stop").pixmap(32,32));
    ui->polygonValidatoin->setToolTip(i18n("Region is invalid. The polygon must be closed and located at the horizon"));
    ui->polygonValidatoin->hide();

    setWindowTitle( i18n( "Artificial Horizon Manager" ) );

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(ui);
    setLayout(mainLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply|QDialogButtonBox::Close);
    mainLayout->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(slotSaveChanges()));

    livePreview = NULL;

    selectPoints = false;

    // Set up List view
    m_RegionsModel = new QStandardItemModel( 0, 3, this );
    m_RegionsModel->setHorizontalHeaderLabels( QStringList() << i18n( "Region") << i18nc( "Azimuth", "Az" ) << i18nc( "Altitude", "Alt" ));

    ui->regionsList->setModel( m_RegionsModel);

    ui->pointsList->setModel( m_RegionsModel );
    ui->pointsList->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
    ui->pointsList->verticalHeader()->hide();
    ui->pointsList->setColumnHidden(0, true);

    horizonComponent = KStarsData::Instance()->skyComposite()->artificialHorizon();

    // Get the map
    regionMap = horizonComponent->regionMap();

    foreach(QString key, regionMap.keys())
    {
        QStandardItem * regionItem = new QStandardItem(key);
        m_RegionsModel->appendRow(regionItem);

        SkyList* points = regionMap.value(key)->points();
        foreach ( SkyPoint* p,  *points)
        {
           QList<QStandardItem*> pointsList;
           pointsList << new QStandardItem("") << new QStandardItem( p->az().toDMSString()) << new QStandardItem( p->alt().toDMSString());
           regionItem->appendRow(pointsList);
        }
    }   

    ui->removeRegionB->setEnabled(true);

    connect(m_RegionsModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(verifyItemValue(QStandardItem*)));

    //Connect buttons
    connect( ui->addRegionB, SIGNAL( clicked() ), this, SLOT( slotAddRegion() ) );
    connect( ui->removeRegionB, SIGNAL( clicked() ), this, SLOT( slotRemoveRegion() ) );
    //connect( ui->addPointB, SIGNAL(clicked()), this, SLOT(slotAddPoint(SkyPoint*)));
    connect(ui->removeRegionB, SIGNAL(clicked()), this, SLOT(slotRemovePoint()));

    connect(ui->regionsList, SIGNAL(clicked(QModelIndex)), this, SLOT(slotSetShownRegion(QModelIndex)));

    connect(ui->addPointB, SIGNAL(clicked()), this, SLOT(slotAddPoint()));
    connect(ui->removePointB, SIGNAL(clicked()), this, SLOT(slotRemovePoint()));
    connect(ui->clearPointsB, SIGNAL(clicked()), this, SLOT(clearPoints()));
    connect(ui->selectPointsB, SIGNAL(clicked(bool)), this, SLOT(setSelectPoints(bool)));

    connect(ui->saveB, SIGNAL(clicked()), this, SLOT(slotSaveChanges()));

    if (regionMap.size() > 0)
    {
        ui->regionsList->selectionModel()->setCurrentIndex(m_RegionsModel->index(0,0), QItemSelectionModel::SelectCurrent);
        showRegion(0);
    }
}

HorizonManager::~HorizonManager()
{
}



void HorizonManager::clearFields()
{

    //disable "Save changes" button
   // ui->saveButton->setEnabled( false );

}

void HorizonManager::showRegion( int regionID )
{
    if ( regionID < 0 || regionID >= m_RegionsModel->rowCount() )
        return;
    else
    {
       ui->pointsList->setRootIndex(m_RegionsModel->index(regionID, 0));
       ui->pointsList->setColumnHidden(0, true);

       QStandardItem *regionItem = m_RegionsModel->item(regionID, 0);

       ui->polygonValidatoin->hide();

       if (regionItem && regionItem->rowCount() > 4)
       {
           if (validatePolygon(regionID))
           {
               ui->polygonValidatoin->setPixmap(QIcon::fromTheme("dialog-ok").pixmap(32,32));
               ui->polygonValidatoin->setEnabled(true);
               ui->polygonValidatoin->setToolTip(i18n("Region is valid"));
           }
           else
           {
               ui->polygonValidatoin->setPixmap(QIcon::fromTheme("process-stop").pixmap(32,32));
               ui->polygonValidatoin->setEnabled(false);
               ui->polygonValidatoin->setToolTip(i18n("Region is invalid. The polygon must be closed"));
           }

           ui->polygonValidatoin->show();
       }

       ui->addPointB->setEnabled(true);
       ui->removePointB->setEnabled(true);
       ui->selectPointsB->setEnabled(true);
       ui->clearPointsB->setEnabled(true);
    }

    ui->saveB->setEnabled( true );
}

bool HorizonManager::validatePolygon(int regionID)
{
    QStandardItem *regionItem = m_RegionsModel->item(regionID, 0);

    const Projector *proj = SkyMap::Instance()->projector();

    if (regionItem == NULL)
        return false;

    QPolygonF poly;
    dms az, alt;
    SkyPoint p;

    for (int i=0; i < regionItem->rowCount(); i++)
    {        
        az  = dms::fromString(regionItem->child(i, 1)->data(Qt::DisplayRole).toString(), true);
        alt = dms::fromString(regionItem->child(i, 2)->data(Qt::DisplayRole).toString(), true);

        if (isnan(az.Degrees()) || isnan(alt.Degrees()))
            return false;

        p.setAz(az);
        p.setAlt(alt);
        p.HorizontalToEquatorial(KStarsData::Instance()->lst(), KStarsData::Instance()->geo()->lat());

        QPointF point = proj->toScreen(&p);
        poly << point;
    }

    return poly.isClosed();
}

void HorizonManager::slotAddRegion()
{
    terminateLivePreview();

    setPointSelection(false);

    QStandardItem* regionItem = new QStandardItem(i18n("Region %1", m_RegionsModel->rowCount()+1));
    m_RegionsModel->appendRow(regionItem);

    QModelIndex index = regionItem->index();
    ui->regionsList->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);

    showRegion(m_RegionsModel->rowCount()-1);

}


void HorizonManager::slotRemoveRegion()
{
    terminateLivePreview();

    setPointSelection(false);

    int regionID = ui->regionsList->currentIndex().row();
    deleteRegion(regionID);

    if (regionID > 0)
        showRegion(regionID-1);
    else if (m_RegionsModel->rowCount() == 0)
    {
        ui->polygonValidatoin->hide();
        m_RegionsModel->clear();
    }

}

void HorizonManager::deleteRegion( int regionID )
{
    if (regionID == -1)
        return;    

    if ( regionID < m_RegionsModel->rowCount() )
    {
        horizonComponent->removeRegion(m_RegionsModel->item(regionID, 0)->data(Qt::DisplayRole).toString());
        m_RegionsModel->removeRow( regionID );        

        //Redraw map
        SkyMap::Instance()->forceUpdate(false);
    }
}

void HorizonManager::slotSaveChanges()
{
    terminateLivePreview();
    setPointSelection(false);

    for (int i=0; i < m_RegionsModel->rowCount(); i++)
    {
        if (validatePolygon(i) == false)
        {
            KMessageBox::error(KStars::Instance(), i18n("%1 polygon is invalid.", m_RegionsModel->item(i,0)->data(Qt::DisplayRole).toString()));
            return;
        }
    }


    for (int i=0; i < m_RegionsModel->rowCount(); i++)
    {
        QStandardItem *regionItem = m_RegionsModel->item(i,0);
        QString regionName = regionItem->data(Qt::DisplayRole).toString();

        horizonComponent->removeRegion(regionName);

        LineList *list = new LineList();
        dms az, alt;
        SkyPoint *p;

        for (int j=0; j < regionItem->rowCount(); j++)
        {
            az  = dms::fromString(regionItem->child(j, 1)->data(Qt::DisplayRole).toString(), true);
            alt = dms::fromString(regionItem->child(j, 2)->data(Qt::DisplayRole).toString(), true);

            p = new SkyPoint();
            p->setAz(az);
            p->setAlt(alt);
            p->HorizontalToEquatorial(KStarsData::Instance()->lst(), KStarsData::Instance()->geo()->lat());

            list->append(p);
        }

        horizonComponent->addRegion(regionName, list);
    }

    horizonComponent->save();

    SkyMap::Instance()->forceUpdate();


}

void HorizonManager::slotSetShownRegion( QModelIndex idx )
{
    showRegion ( idx.row() );
}

void HorizonManager::processSkyPoint(QStandardItem *item, int row)
{
    QStandardItem *azItem = item->child(row, 1);
    QStandardItem *altItem= item->child(row, 2);

    dms az = dms::fromString(azItem->data(Qt::DisplayRole).toString(), true);
    dms alt= dms::fromString(altItem->data(Qt::DisplayRole).toString(), true);

    // First & Last points always have altitude of zero
    if ( (row == 0 && alt.Degrees() != 0) || (alt.Degrees() < 0))
    {
        altItem->setData(QVariant(0), Qt::DisplayRole);
        return;
    }

    SkyPoint *point = NULL;

    //qDebug() << "Item row count is " << item->rowCount();

    if (livePreview == NULL)
    {
        livePreview = new LineList();

        if (row > 0)
        {
            LineList *prevList = regionMap.value(item->parent()->data(Qt::DisplayRole).toString());
            for (int i=0; i < row; i++)
                livePreview->append(prevList->at(i));
        }
        horizonComponent->setLivePreview(livePreview);
    }

    if (item->rowCount() >= livePreview->points()->count())
    {
        point = new SkyPoint();
        livePreview->append(point);
    }
    else
        point = livePreview->at(item->rowCount());

    point->setAz(az);
    point->setAlt(alt);
    point->HorizontalToEquatorial(KStarsData::Instance()->lst(), KStarsData::Instance()->geo()->lat());

    qDebug() << "Received Az: " << point->az().toDMSString() << " Alt: " << point->alt().toDMSString();

    SkyMap::Instance()->forceUpdate(false);

    bool finalPoint=false;

    if (item->rowCount() >= 5)
    {
        dms firstAz, firstAlt;
        firstAz  = dms::fromString(item->child(0, 1)->data(Qt::DisplayRole).toString(), true);
        firstAlt = dms::fromString(item->child(0, 2)->data(Qt::DisplayRole).toString(), true);

        if (fabs(firstAz.Degrees()-point->az().Degrees()) <= 5)
        {
            point->setAz(firstAz.Degrees());
            point->setAlt(0);

            m_RegionsModel->disconnect(this);
            azItem->setData(firstAz.toDMSString(), Qt::DisplayRole);
            altItem->setData(QVariant(0), Qt::DisplayRole);
            connect(m_RegionsModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(verifyItemValue(QStandardItem*)));

            setPointSelection(false);

            finalPoint = true;
        }
    }

    if (finalPoint && item->rowCount() > 4)
    {
        if (validatePolygon(ui->regionsList->currentIndex().row()))
        {
            ui->polygonValidatoin->setPixmap(QIcon::fromTheme("dialog-ok").pixmap(32,32));
            ui->polygonValidatoin->setEnabled(true);
            ui->polygonValidatoin->setToolTip(i18n("Region is valid"));
        }
        else
        {
            ui->polygonValidatoin->setPixmap(QIcon::fromTheme("process-stop").pixmap(32,32));
            ui->polygonValidatoin->setEnabled(false);
            ui->polygonValidatoin->setToolTip(i18n("Region is invalid. The polygon must be closed and located at the horizon"));
        }

        ui->polygonValidatoin->show();
    }

}

void HorizonManager::addSkyPoint(SkyPoint *skypoint)
{
   if (selectPoints == false)
       return;

   slotAddPoint();

   QStandardItem *regionItem = m_RegionsModel->item(ui->regionsList->currentIndex().row(), 0);

   if (regionItem)
   {
       QStandardItem *az = regionItem->child(regionItem->rowCount()-1, 1);
       QStandardItem *alt= regionItem->child(regionItem->rowCount()-1, 2);

       az->setData(skypoint->az().toDMSString(), Qt::DisplayRole);
       alt->setData(skypoint->alt().toDMSString(), Qt::DisplayRole);
   }
}

void HorizonManager::slotAddPoint()
{
    QStandardItem *regionItem = m_RegionsModel->item(ui->regionsList->currentIndex().row(), 0);

    if (regionItem == NULL)
        return;

    QList<QStandardItem*> pointsList;
    pointsList << new QStandardItem("") << new QStandardItem("") << new QStandardItem("");
    regionItem->appendRow(pointsList);

    m_RegionsModel->setHorizontalHeaderLabels( QStringList() << i18n( "Region") << i18nc( "Azimuth", "Az" ) << i18nc( "Altitude", "Alt" ));
    ui->pointsList->setColumnHidden(0, true);
    ui->pointsList->setRootIndex(regionItem->index());
}

void HorizonManager::slotRemovePoint()
{
    QStandardItem *regionItem = m_RegionsModel->item(ui->regionsList->currentIndex().row(), 0);

    if (regionItem)
    {
        int row = ui->pointsList->currentIndex().row();
        regionItem->removeRow(row);

        if (regionItem->rowCount() < 4)
            ui->polygonValidatoin->hide();
        else
        {
            if (validatePolygon(ui->regionsList->currentIndex().row()))
            {
                ui->polygonValidatoin->setPixmap(QIcon::fromTheme("dialog-ok").pixmap(32,32));
                ui->polygonValidatoin->setEnabled(true);
                ui->polygonValidatoin->setToolTip(i18n("Region is valid"));
            }
            else
            {
                ui->polygonValidatoin->setPixmap(QIcon::fromTheme("process-stop").pixmap(32,32));
                ui->polygonValidatoin->setEnabled(false);
                ui->polygonValidatoin->setToolTip(i18n("Region is invalid. The polygon must be closed"));
            }
        }

        if (livePreview && row < livePreview->points()->count())
        {
            delete livePreview->points()->takeAt(row);

            if (livePreview->points()->count() == 0)
                terminateLivePreview();
            else
                SkyMap::Instance()->forceUpdate(false);

        }
    }
}

void HorizonManager::clearPoints()
{
    QStandardItem *regionItem = m_RegionsModel->item(ui->regionsList->currentIndex().row(), 0);

    //qDebug() << "Row " << ui->regionsList->currentIndex().row();

    if (regionItem)
    {
        regionItem->removeRows(0, regionItem->rowCount());

        ui->polygonValidatoin->hide();
    }

    terminateLivePreview();
}

void HorizonManager::setSelectPoints(bool enable)
{
    selectPoints = enable;
    ui->selectPointsB->clearFocus();
}

void HorizonManager::verifyItemValue(QStandardItem *item)
{
    bool azOK=true, altOK=true;

    if (item->column() >= 1)
    {
        QStandardItem *parent = item->parent();

        dms azAngle  = dms::fromString(parent->child(item->row(),1)->data(Qt::DisplayRole).toString(), true);
        dms altAngle = dms::fromString(parent->child(item->row(),2)->data(Qt::DisplayRole).toString(), true);

        if (isnan(azAngle.Degrees()))
            azOK = false;
        if (isnan(altAngle.Degrees()))
            altOK = false;

        if ( (item->column() == 1 && azOK == false) || (item->column() == 2 && altOK == false) )

        {
            KMessageBox::error(KStars::Instance(), i18n("Invalid angle value: %1", item->data(Qt::DisplayRole).toString()));
            item->setData(QVariant(0), Qt::DisplayRole);
            return;
        }
        else if (azOK && altOK)
            processSkyPoint(item->parent(), item->row());
    }
}

void HorizonManager::terminateLivePreview()
{
    if (livePreview)
    {
        while (livePreview->points()->size() > 0)
            delete livePreview->points()->takeAt(0);

        delete(livePreview);

        livePreview = NULL;

        horizonComponent->setLivePreview(NULL);
    }
}

void HorizonManager::setPointSelection(bool enable)
{
    selectPoints = enable;
    ui->selectPointsB->setChecked(enable);
}