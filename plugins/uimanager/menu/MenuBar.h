#pragma once

#include "MenuElement.h"
#include <wx/menu.h>

namespace ui
{

class MenuBar :
	public MenuElement
{
private:
	wxMenuBar* _menuBar;

public:
	MenuBar();

	virtual wxMenuBar* getMenuBar();

	// Makes sure this menubar and all child menus are constructed
	void ensureMenusConstructed();

protected:
	virtual void construct() override;
	virtual void deconstruct() override;

private:
	MenuElementPtr findMenu(wxMenu* menu);
	void onMenuOpen(wxMenuEvent& ev);
};

}

