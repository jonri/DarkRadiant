#include "Toggle.h"

#include "itextstream.h"
#include "Accelerator.h"
#include <wx/menu.h>
#include <wx/menuitem.h>
#include <wx/toolbar.h>
#include <wx/tglbtn.h>
#include <wx/button.h>

namespace ui
{

Toggle::Toggle(const ToggleCallback& callback) :
	_callback(callback),
	_callbackActive(false),
	_toggled(false)
{}

Toggle::~Toggle()
{
}

bool Toggle::empty() const {
	return false;
}

bool Toggle::isToggle() const {
	return true;
}

// Set the toggled state to true/false, according to <toggled> and update
// any associated widgets or notify any callbacks.
bool Toggle::setToggled(const bool toggled)
{
	if (_callbackActive) {
		return false;
	}

	// Update the toggle status and export it to the GTK button
	_toggled = toggled;
	updateWidgets();

	return true;
}

void Toggle::updateWidgets()
{
	_callbackActive = true;

	for (MenuItems::const_iterator i = _menuItems.begin(); i != _menuItems.end(); ++i)
	{
		(*i)->Check(_toggled);
	}

	for (ToolItems::const_iterator i = _toolItems.begin(); i != _toolItems.end(); ++i)
	{
		(*i)->GetToolBar()->ToggleTool((*i)->GetId(), _toggled);
	}

	for (Buttons::const_iterator i = _buttons.begin(); i != _buttons.end(); ++i)
	{
		(*i)->SetValue(_toggled);
	}

	_callbackActive = false;
}

// On key press >> toggle the internal state
void Toggle::keyDown() {
	toggle();
}

bool Toggle::isToggled() const {
	return _toggled;
}

void Toggle::connectMenuItem(wxMenuItem* item)
{
	if (!item->IsCheckable())
	{
		rWarning() << "Cannot connect non-checkable menu item to this event." << std::endl;
		return;
	}

	if (_menuItems.find(item) != _menuItems.end())
	{
		rWarning() << "Cannot connect to the same menu item more than once." << std::endl;
		return;
	}

	_menuItems.insert(item);

	item->Check(_toggled);

	// Connect the togglebutton to the callback of this class
	assert(item->GetMenu());
	item->GetMenu()->Bind(wxEVT_MENU, &Toggle::onMenuItemClicked, this, item->GetId());
}

void Toggle::disconnectMenuItem(wxMenuItem* item)
{
	if (!item->IsCheckable())
	{
		rWarning() << "Cannot disconnect from non-checkable menu item." << std::endl;
		return;
	}

	if (_menuItems.find(item) == _menuItems.end())
	{
		rWarning() << "Cannot disconnect from unconnected menu item." << std::endl;
		return;
	}

	_menuItems.erase(item);

	// Connect the togglebutton to the callback of this class
	assert(item->GetMenu());
	item->GetMenu()->Unbind(wxEVT_MENU, &Toggle::onMenuItemClicked, this, item->GetId());
}

void Toggle::onMenuItemClicked(wxCommandEvent& ev)
{
	// Make sure the event is actually directed at us
	for (MenuItems::const_iterator i = _menuItems.begin(); i != _menuItems.end(); ++i)
	{
		if ((*i)->GetId() == ev.GetId())
		{
			toggle();
			return;
		}
	}

	ev.Skip();
}

void Toggle::connectToolItem(const wxToolBarToolBase* item)
{
	if (_toolItems.find(item) != _toolItems.end())
	{
		rWarning() << "Cannot connect to the same tool item more than once." << std::endl;
		return;
	}

	_toolItems.insert(item);

	// Update the widget first
	item->GetToolBar()->ToggleTool(item->GetId(), _toggled);

	// Connect the to the callback of this class
	item->GetToolBar()->Bind(wxEVT_TOOL, &Toggle::onToolItemClicked, this, item->GetId());
}

void Toggle::disconnectToolItem(const wxToolBarToolBase* item)
{
	if (_toolItems.find(item) == _toolItems.end())
	{
		//rWarning() << "Cannot disconnect from unconnected tool item." << std::endl;
		return;
	}

	_toolItems.erase(item);

	// Connect the to the callback of this class
	item->GetToolBar()->Unbind(wxEVT_TOOL, &Toggle::onToolItemClicked, this, item->GetId());
}

void Toggle::onToolItemClicked(wxCommandEvent& ev)
{
	// Make sure the event is actually directed at us
	for (ToolItems::const_iterator i = _toolItems.begin(); i != _toolItems.end(); ++i)
	{
		if ((*i)->GetId() == ev.GetId())
		{
			toggle();
			return;
		}
	}

	ev.Skip();
}

void Toggle::connectToggleButton(wxToggleButton* button)
{
	if (_buttons.find(button) != _buttons.end())
	{
		rWarning() << "Cannot connect to the same button more than once." << std::endl;
		return;
	}

	_buttons.insert(button);

	// Update the widget's state to match the internal one
	button->SetValue(_toggled);

	// Connect the to the callback of this class
	button->Connect(wxEVT_TOGGLEBUTTON, wxCommandEventHandler(Toggle::onToggleButtonClicked), NULL, this);
}

void Toggle::disconnectToggleButton(wxToggleButton* button)
{
	if (_buttons.find(button) == _buttons.end())
	{
		rWarning() << "Cannot disconnect from unconnected button." << std::endl;
		return;
	}

	_buttons.erase(button);

	// Connect the to the callback of this class
	button->Disconnect(wxEVT_TOGGLEBUTTON, wxCommandEventHandler(Toggle::onToggleButtonClicked), NULL, this);
}

void Toggle::onToggleButtonClicked(wxCommandEvent& ev)
{
	toggle();
}

// Invoke the registered callback and update/notify
void Toggle::toggle()
{
	if (_callbackActive) {
		return;
	}

	// Check if the toggle event is enabled
	if (_enabled) {
		// Invert the <toggled> state
		_toggled = !_toggled;

		// Call the connected function with the new state
		_callback(_toggled);
	}

	// Update any attached GtkObjects in any case
	updateWidgets();
}

}
