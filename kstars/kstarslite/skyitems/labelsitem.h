/** *************************************************************************
                          labelsitem.h  -  K Desktop Planetarium
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
#ifndef LABELSITEM_H_
#define LABELSITEM_H_

#include "skylabeler.h"

#include <QSGOpacityNode>

typedef QSGOpacityNode LabelTypeNode;

class LabelNode;
class GuideLabelNode;
class RootNode;

class SkyObject;

/**
 * @class LabelsItem
 *
 * This class is in charge of labels in SkyMapLite. Labels can be instantiated by calling addLabel with
 * either SkyObject or plain QString as a name. There are two types of label nodes available - LabelNode
 * that can't be rotated and GuideLabelNode that supports rotation (but it not used now anywhere).
 *
 * To prevent labels from overlapping this class uses SkyLabeler. We check LabelNode for overlapping by
 * calling SkyLabeler::markText() (SkyLabeler::markRegion() for GuideLabelNode) update().
 *
 * Each of SkyItems that uses labels has its own label type in enum label_t (copied from SkyLabeler but
 * was a bit extended). Labels of particular type are reparented to LabelTypeNode(QSGOpacityNode) so
 * to hide all labels of some type you just need to set opacity of LabelTypeNode that corresponds to
 * this type to 0.
 *
 * Order of drawing can be changed in LabelsItem's constructor. Order of labels update can be changed in
 * update().
 *
 * This class is not derived from SkyItem as it doesn't have label type and SkyItem's header needs an
 * inclusion of this header to allow use of label_t enum. (Might be a good idea to fix this)
 *
 * @note font size is set in SkyLabeler::SkyLabeler() by initializing m_stdFont with default font
 *
 * @short handles labels in SkyMapLite
 * @author Artem Fedoskin
 * @version 1.0
 */

class LabelsItem : public QSGOpacityNode {

public:

    /**
     * @brief Constructor
     * @param rootNode parent RootNode that instantiates this object
     */
    LabelsItem();

    /**
     * @short The label_t enum. Holds types of labels
     */
    enum label_t {
        STAR_LABEL,
        ASTEROID_LABEL,
        COMET_LABEL,
        PLANET_LABEL,
        JUPITER_MOON_LABEL,
        SATURN_MOON_LABEL,
        DEEP_SKY_LABEL,
        CONSTEL_NAME_LABEL,
        SATELLITE_LABEL,
        RUDE_LABEL, ///Rude labels block other labels FIXME: find a better solution
        NUM_LABEL_TYPES,
        HORIZON_LABEL,
        EQUATOR_LABEL,
        ECLIPTIC_LABEL,
        NO_LABEL //used in LinesItem
    };

    /**
     * creates LabelNode with given skyObject and appends it to LabelTypeNode that corresponds
     * to type
     * @param skyObject for which the label is created
     * @param labelType type of LabelTypeNode to which this label has to be reparented
     * @return pointer to newly created LabelNode
     */
    LabelNode *addLabel(SkyObject *skyObject, label_t labelType);

    /**
     * @short does the same as above but with QString instead of SkyObject
     */

    LabelNode *addLabel(QString name, label_t labelType);

    /**
     * @short does the same as above but instead creates GuideLabelNode
     * @note currently GuideLabelNode is not used anywhere so it is not fully supported yet
     */

    GuideLabelNode *addGuideLabel(QString name, label_t labelType);

    /**
     * The order of labels update can be changed here.
     * @short updates all child labels
     */

    void update();

    /**
     * @short updates child labels of LabelTypeNode that corresponds to type in m_labelsLists
     * @param labelType type of LabelTypeNode (see m_labelsLists)
     */

    void updateChildLabels(label_t labelType);

    /**
     * @return LabelTypeNode that holds labels of labelType
     */

    LabelTypeNode *getLabelNode(label_t labelType) { return m_labelsLists.value(labelType); }

    /**
     * @short deletes all labels of type labelType
     */

    void deleteLabels(label_t labelType);

    /**
     * @short hides all labels of type labelType
     */

    void hideLabels(label_t labelType);

    /**
     * @short sets m_rootNode and appends to it this node
     * @param rootNode
     */
    void setRootNode(RootNode *rootNode);

    /**
     * @return pointer to RootNode that instantiated this object
     */

    RootNode *rootNode() { return m_rootNode; }

private:
    QMap<label_t, LabelTypeNode *> m_labelsLists;

    /**
     * @short because this class is not derived from SkyItem it has to store pointer to RootNode
     */

    RootNode *m_rootNode;
    SkyLabeler *skyLabeler;
};
#endif

