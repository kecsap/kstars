/** *************************************************************************
                          labelsitem.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : 10/06/2016
    copyright            : (C) 2016 by Artem Fedoskin
    email                : afedoskin3@gmail.com
 ***************************************************************************/
/** *************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "Options.h"
#include <QSGNode>
#include "labelsitem.h"
#include "skylabeler.h"
#include "skynodes/labelnode.h"
#include "skynodes/guidelabelnode.h"

#include "cometsitem.h"
#include "rootnode.h"

LabelsItem::LabelsItem()
    :m_rootNode(0)
{
    LabelTypeNode *stars = new LabelTypeNode;
    appendChildNode(stars);
    m_labelsLists.insert(label_t::STAR_LABEL, stars);

    LabelTypeNode *asteroids = new LabelTypeNode;
    appendChildNode(asteroids);
    m_labelsLists.insert(label_t::ASTEROID_LABEL, asteroids);

    LabelTypeNode *comets = new LabelTypeNode;
    appendChildNode(comets);
    m_labelsLists.insert(label_t::COMET_LABEL, comets);

    LabelTypeNode *planets = new LabelTypeNode;
    appendChildNode(planets);
    m_labelsLists.insert(label_t::PLANET_LABEL, planets);

    LabelTypeNode *jupiter_moons = new LabelTypeNode;
    appendChildNode(jupiter_moons);
    m_labelsLists.insert(label_t::JUPITER_MOON_LABEL, jupiter_moons);

    LabelTypeNode *saturn_moons = new LabelTypeNode;
    appendChildNode(saturn_moons);
    m_labelsLists.insert(label_t::SATURN_MOON_LABEL, saturn_moons);

    LabelTypeNode *deep_sky = new LabelTypeNode;
    appendChildNode(deep_sky);
    m_labelsLists.insert(label_t::DEEP_SKY_LABEL, deep_sky);

    LabelTypeNode *constellation = new LabelTypeNode;
    appendChildNode(constellation);
    m_labelsLists.insert(label_t::CONSTEL_NAME_LABEL, constellation);

    LabelTypeNode *satellite = new LabelTypeNode;
    appendChildNode(satellite);
    m_labelsLists.insert(label_t::SATELLITE_LABEL, satellite);

    LabelTypeNode *rude = new LabelTypeNode;
    appendChildNode(rude);
    m_labelsLists.insert(label_t::RUDE_LABEL, rude);

    LabelTypeNode *num_label = new LabelTypeNode;
    appendChildNode(num_label);
    m_labelsLists.insert(label_t::NUM_LABEL_TYPES, num_label);

    LabelTypeNode *horizon_label = new LabelTypeNode;
    appendChildNode(horizon_label);
    m_labelsLists.insert(label_t::HORIZON_LABEL, horizon_label);

    LabelTypeNode *equator = new LabelTypeNode;
    appendChildNode(equator);
    m_labelsLists.insert(label_t::EQUATOR_LABEL, equator);

    LabelTypeNode *ecliptic = new LabelTypeNode;
    appendChildNode(ecliptic);
    m_labelsLists.insert(label_t::ECLIPTIC_LABEL, ecliptic);

    skyLabeler = SkyLabeler::Instance();
    skyLabeler->reset();
}

LabelNode *LabelsItem::addLabel(SkyObject *skyObject, label_t labelType) {
    LabelNode *label = new LabelNode(skyObject, labelType);
    m_labelsLists.value(labelType)->appendChildNode(label);
    return label;
}

LabelNode *LabelsItem::addLabel(QString name, label_t labelType) {
    LabelNode *label = new LabelNode(name, labelType);
    m_labelsLists.value(labelType)->appendChildNode(label);
    return label;
}

GuideLabelNode *LabelsItem::addGuideLabel(QString name, label_t labelType) {
    GuideLabelNode *label = new GuideLabelNode(name, labelType);
    m_labelsLists.value(labelType)->appendChildNode(label);
    return label;
}

void LabelsItem::update() {
    SkyLabeler * skyLabeler = SkyLabeler::Instance();
    skyLabeler->reset();

    updateChildLabels(label_t::EQUATOR_LABEL);
    updateChildLabels(label_t::ECLIPTIC_LABEL);

    updateChildLabels(label_t::PLANET_LABEL);
    updateChildLabels(label_t::JUPITER_MOON_LABEL);

    updateChildLabels(label_t::SATURN_MOON_LABEL);
    updateChildLabels(label_t::ASTEROID_LABEL);

    if(getLabelNode(label_t::COMET_LABEL)->opacity()) {
        updateChildLabels(label_t::COMET_LABEL);
    }

    updateChildLabels(label_t::CONSTEL_NAME_LABEL);
    updateChildLabels(label_t::HORIZON_LABEL);

}

void LabelsItem::hideLabels(label_t labelType) {
    QSGOpacityNode *node = m_labelsLists[labelType];
    node->setOpacity(0);
    node->markDirty(QSGNode::DirtyOpacity);
}

void LabelsItem::setRootNode(RootNode *rootNode) {
    //Remove from previous parent if had any
    if(m_rootNode && parent() == m_rootNode) m_rootNode->removeChildNode(this);

    //Append to new parent if haven't already
    m_rootNode = rootNode;
    if(parent() != m_rootNode) m_rootNode->appendChildNode(this);
}

void LabelsItem::deleteLabels(label_t labelType) {
    if(labelType != label_t::NO_LABEL) {
        QSGOpacityNode *node = m_labelsLists[labelType];
        while(QSGNode *n = node->firstChild()) { node->removeChildNode(n); delete n; }
    }
}

void LabelsItem::updateChildLabels(label_t labelType) {
    QSGOpacityNode *node = m_labelsLists[labelType];
    node->setOpacity(1);
    node->markDirty(QSGNode::DirtyOpacity);

    QSGNode *n = node->firstChild();
    /*if( labelType == label_t::HORIZON_LABEL
            || labelType == label_t::ECLIPTIC_LABEL || labelType == label_t::EQUATOR_LABEL) {
        while( n != 0) {
            GuideLabelNode *label = static_cast<GuideLabelNode *>(n);
            if(label->visible()) {
                //if(SkyLabeler::Instance()->markRegion(label->left,label->right,label->top,label->bot)) {
                QString name = label->name();
                if(SkyLabeler::Instance()->markText(label->labelPos, name)) {
                    label->update();
                } else {
                    label->hide();
                }
            }
            n = n->nextSibling();
        }
    } else {*/
    while( n != 0) {
        LabelNode *label = static_cast<LabelNode *>(n);
        if(label->visible()) {
            if(label->zoomFont()) skyLabeler->resetFont();
            if(skyLabeler->markText(label->labelPos, label->name())) {
                label->update();
            } else {
                label->hide();
            }
            skyLabeler->useStdFont();
        }
        n = n->nextSibling();
    }

}
