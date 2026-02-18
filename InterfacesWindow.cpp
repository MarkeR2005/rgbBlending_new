//---------------------------------------------------------------------------

#pragma hdrstop

#include "InterfacesWindow.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
std::vector<ISyncWindow*> ISyncWindow::instances_;
void ISyncWindow::emitToAll(){
		for (ISyncWindow* instance : ISyncWindow::instances_) {
			if (instance->getSync()) {
				instance->handleSync();
			}
		}
	};
