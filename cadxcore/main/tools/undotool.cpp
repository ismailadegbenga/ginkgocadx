/*
 * This file is part of Ginkgo CADx
 *
 * Copyright (c) 2015-2016 Gert Wollny
 * Copyright (c) 2008-2014 MetaEmotion S.L. All rights reserved.
 *
 * Ginkgo CADx is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser Public License
 * along with Ginkgo CADx; if not, see <http://www.gnu.org/licenses/>.
 *
 */


#include <sstream>
#include "undotool.h"

#if defined(ENABLE_UNDOTOOL)

#include <wx/menu.h>
#include <resources/ginkgoresourcesmanager.h>
#include <export/contracts/iviewcontract.h>
#include <main/gui/toolsystem/wxmenuitemtool.h>
#include <api/internationalization/internationalization.h>
#include <main/entorno.h>
#include <api/globals.h>
#include <main/controllers/controladoracciones.h>

//----------------------------------------------------------------------
#define ID_DESHACER  1721
#define ID_REHACER   1722

namespace GNC
{
namespace GUI
{
class EventHandlerUndo: public wxEvtHandler
{
public:
        EventHandlerUndo(wxEvtHandler* pParent, GNC::UndoTool* pHerramienta):wxEvtHandler()
        {
                m_pParent = pParent;
                m_pHerramienta=pHerramienta;
                m_pParent->Connect(ID_DESHACER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( EventHandlerUndo::OnDeshacer),NULL,this);
                m_pParent->Connect(ID_DESHACER, wxEVT_UPDATE_UI, wxUpdateUIEventHandler(EventHandlerUndo::OnUpdateDeshacerUI),NULL,this);
                m_pParent->Connect(ID_REHACER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( EventHandlerUndo::OnRehacer),NULL,this);
                m_pParent->Connect(ID_REHACER, wxEVT_UPDATE_UI, wxUpdateUIEventHandler(EventHandlerUndo::OnUpdateRehacerUI),NULL,this);
        }
        ~EventHandlerUndo()
        {
                m_pParent->Disconnect(ID_DESHACER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( EventHandlerUndo::OnDeshacer),NULL,this);
                m_pParent->Disconnect(ID_DESHACER, wxEVT_UPDATE_UI, wxUpdateUIEventHandler(EventHandlerUndo::OnUpdateDeshacerUI),NULL,this);
                m_pParent->Disconnect(ID_REHACER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( EventHandlerUndo::OnRehacer),NULL,this);
                m_pParent->Disconnect(ID_REHACER, wxEVT_UPDATE_UI, wxUpdateUIEventHandler(EventHandlerUndo::OnUpdateRehacerUI),NULL,this);
                m_pParent = NULL;
                m_pHerramienta = NULL;
        }

        void OnDeshacer(wxCommandEvent& event)
        {
                m_pHerramienta->Deshacer();
                event.Skip(false);
        }

        void OnUpdateDeshacerUI(wxUpdateUIEvent& event)
        {
                std::string titulo;
                event.Enable(m_pHerramienta->PuedeDeshacer(titulo));
                event.Skip(false);
        }

        void OnRehacer(wxCommandEvent& event)
        {
                m_pHerramienta->Rehacer();
                event.Skip(false);
        }

        void OnUpdateRehacerUI(wxUpdateUIEvent& event)
        {
                std::string titulo;
                event.Enable(m_pHerramienta->PuedeRehacer(titulo));
                event.Skip(false);
        }

        GNC::UndoTool* m_pHerramienta;
        wxEvtHandler* m_pParent;
};
}
}


GNC::GCS::ITool* GNC::UndoTool::NewTool()
{
        return new GNC::UndoTool();
}

GNC::UndoTool::UndoTool()
{
}
GNC::UndoTool::~UndoTool()
{
}

bool GNC::UndoTool::ExecuteAction()
{
        return false;
}

void GNC::UndoTool::AppendToolIn(wxEvtHandler* pParent, wxMenu* pMenu)
{
        if (AppendsInMenu()) {
                wxBitmap icon;
                pMenu->Append(new GNC::GUI::wxMenuItemTool(pMenu,ID_DESHACER,_("Undo")+wxString(wxT(" \tCtrl+z")), icon, new GNC::GUI::EventHandlerUndo(pParent, this)));
                pMenu->Append(new GNC::GUI::wxMenuItemTool(pMenu,ID_REHACER,_("Redo")+wxString(wxT(" \tCtrl+y")), icon, new GNC::GUI::EventHandlerUndo(pParent, this)));
        }
}

void GNC::UndoTool::Deshacer()
{
        return GNC::GCS::ControladorAcciones::Instance()->Deshacer(ViewContract->GetView());
}

void GNC::UndoTool::Rehacer()
{
        return GNC::GCS::ControladorAcciones::Instance()->Hacer(ViewContract->GetView());
}

bool GNC::UndoTool::PuedeDeshacer(std::string& nombreAccion)
{
        return GNC::GCS::ControladorAcciones::Instance()->PuedeDeshacer(ViewContract->GetView(),nombreAccion);
}
bool GNC::UndoTool::PuedeRehacer(std::string& nombreAccion)
{
        return GNC::GCS::ControladorAcciones::Instance()->PuedeHacer(ViewContract->GetView(),nombreAccion);
}

#endif
