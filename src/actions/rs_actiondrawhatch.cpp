/****************************************************************************
**
** This file is part of the CADuntu project, a 2D CAD program
**
** Copyright (C) 2010 R. van Twisk (caduntu@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by 
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!  
**
**********************************************************************/

#include "rs_actiondrawhatch.h"

#include "rs_snapper.h"
#include "rs_graphicview.h"
#include "rs_information.h"



RS_ActionDrawHatch::RS_ActionDrawHatch(RS_EntityContainer& container,
                                       RS_GraphicView& graphicView)
        :RS_PreviewActionInterface("Draw Hatch",
                           container, graphicView) {

    hatch = NULL;
}


QAction* RS_ActionDrawHatch::createGUIAction(RS2::ActionType /*type*/, QObject* /*parent*/) {
 /* RVT_PORT   QAction* action = new QAction(tr("Hatch"), tr("&Hatch"),
                                  QKeySequence(), NULL); */
    QAction* action = new QAction(tr("Hatch"),  NULL);
    action->setStatusTip(tr("Draw Hatches and Solid Fills"));
    return action;
}


void RS_ActionDrawHatch::init(int status) {
    RS_ActionInterface::init(status);

    RS_Hatch tmp(container, data);
    tmp.setLayerToActive();
    tmp.setPenToActive();
    if (RS_DIALOGFACTORY->requestHatchDialog(&tmp)) {
        data = tmp.getData();
        trigger();
        finish();
        graphicView->redraw(RS2::RedrawDrawing); 

    } else {
        finish();
    }
}



void RS_ActionDrawHatch::trigger() {

    RS_DEBUG->print("RS_ActionDrawHatch::trigger()");

    //if (pos.valid) {
    //deletePreview();
	RS_Entity* e;

	// deselect unhatchable entities:
    for (e=container->firstEntity(RS2::ResolveNone); e!=NULL;
            e=container->nextEntity(RS2::ResolveNone)) {
        if (e->isSelected() && 
		    (e->rtti()==RS2::EntityHatch ||
			 e->rtti()==RS2::EntityEllipse || e->rtti()==RS2::EntityPoint ||
			 e->rtti()==RS2::EntityText || 
			 RS_Information::isDimension(e->rtti()))) {
			e->setSelected(false);
        }
    }
    for (e=container->firstEntity(RS2::ResolveAll); e!=NULL;
            e=container->nextEntity(RS2::ResolveAll)) {
        if (e->isSelected() && 
		    (e->rtti()==RS2::EntityHatch ||
			 e->rtti()==RS2::EntityEllipse || e->rtti()==RS2::EntityPoint ||
			 e->rtti()==RS2::EntityText || 
			 RS_Information::isDimension(e->rtti()))) {
			e->setSelected(false);
        }
    }

	// look for selected contours:
    bool haveContour = false;
    for (e=container->firstEntity(RS2::ResolveAll); e!=NULL;
            e=container->nextEntity(RS2::ResolveAll)) {
        if (e->isSelected()) {
            haveContour = true;
        }
    }

    if (!haveContour) {
        std::cerr << "no contour selected\n";
        return;
    }

    hatch = new RS_Hatch(container, data);
    hatch->setLayerToActive();
    hatch->setPenToActive();
    RS_EntityContainer* loop = new RS_EntityContainer(hatch);
    loop->setPen(RS_Pen(RS2::FlagInvalid));

    // add selected contour:
    for (RS_Entity* e=container->firstEntity(RS2::ResolveAll); e!=NULL;
            e=container->nextEntity(RS2::ResolveAll)) {

        if (e->isSelected()) {
            e->setSelected(false);
			// entity is part of a complex entity (spline, polyline, ..):
			if (e->getParent()!=NULL && 
			    (e->getParent()->rtti()==RS2::EntitySpline ||
				 e->getParent()->rtti()==RS2::EntityPolyline)) {
				e->getParent()->setSelected(false);
			}
            RS_Entity* cp = e->clone();
            cp->setPen(RS_Pen(RS2::FlagInvalid));
            cp->reparent(loop);
            loop->addEntity(cp);
        }
    }

    hatch->addEntity(loop);
	if (hatch->validate()) {
		container->addEntity(hatch);

		if (document) {
			document->startUndoCycle();
			document->addUndoable(hatch);
			document->endUndoCycle();
		}
		hatch->update();

		graphicView->redraw(RS2::RedrawDrawing);

		RS_DIALOGFACTORY->commandMessage(tr("Hatch created successfully."));
	}
	else {
		delete hatch;
		hatch = NULL;
		RS_DIALOGFACTORY->commandMessage(tr("Invalid hatch area. Please check that "
		"the entities chosen form one or more closed contours."));
	}
    //}
}



void RS_ActionDrawHatch::mouseMoveEvent(RS_MouseEvent*) {
    RS_DEBUG->print("RS_ActionDrawHatch::mouseMoveEvent begin");

    /*if (getStatus()==SetPos) {
        RS_Vector mouse = snapPoint(e);
        pos = mouse;


        deletePreview();
        if (hatch!=NULL && !hatch->isVisible()) {
            hatch->setVisible(true);
        }
        offset = RS_Vector(graphicView->toGuiDX(pos.x),
                           -graphicView->toGuiDY(pos.y));
        drawPreview();
}*/

    RS_DEBUG->print("RS_ActionDrawHatch::mouseMoveEvent end");
}



void RS_ActionDrawHatch::mouseReleaseEvent(RS_MouseEvent* e) {
    if (RS2::qtToRsButtonState(e->button())==RS2::LeftButton) {
        RS_Vector mouse = snapPoint(e);

        switch (getStatus()) {
        case ShowDialog:
            break;

        default:
            break;
        }
    } else if (RS2::qtToRsButtonState(e->button())==RS2::RightButton) {
        //deletePreview();
        init(getStatus()-1);
    }
}



void RS_ActionDrawHatch::updateMouseButtonHints() {
    RS_DIALOGFACTORY->updateMouseWidget("", "");
}



void RS_ActionDrawHatch::updateMouseCursor() {
    graphicView->setMouseCursor(RS2::CadCursor);
}



void RS_ActionDrawHatch::updateToolBar() {
    RS_DIALOGFACTORY->requestToolBar(RS2::ToolBarMain);
}


// EOF