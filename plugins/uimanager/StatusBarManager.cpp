#include "StatusBarManager.h"

#include "itextstream.h"

#include <wx/panel.h>
#include <wx/artprov.h>
#include <wx/frame.h>

#include "LocalBitmapArtProvider.h"

namespace ui
{

StatusBarManager::StatusBarManager() :
	_tempParent(new wxFrame(NULL, wxID_ANY, "")),
	_statusBar(new wxPanel(_tempParent, wxID_ANY))
{
	_statusBar->SetName("Statusbar");

	_tempParent->Hide();

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	_statusBar->SetSizer(sizer);
}

StatusBarManager::~StatusBarManager()
{
	_tempParent->Destroy();
}

wxWindow* StatusBarManager::getStatusBar()
{
	return _statusBar;
}

void StatusBarManager::addElement(const std::string& name, wxWindow* widget, int pos)
{
	// Get a free position
	int freePos = getFreePosition(pos);

	StatusBarElementPtr element(new StatusBarElement(widget));

	// Store this element
	_elements.insert(ElementMap::value_type(name, element));
	_positions.insert(PositionMap::value_type(freePos, element));

	rebuildStatusBar();
}

wxWindow* StatusBarManager::getElement(const std::string& name)
{
	// Look up the key
	ElementMap::const_iterator found = _elements.find(name);

	// return NULL if not found
	return (found != _elements.end()) ? found->second->toplevel : NULL;
}

void StatusBarManager::addTextElement(const std::string& name, const std::string& icon, int pos)
{
	// Get a free position
	int freePos = getFreePosition(pos);

	wxPanel* textPanel = new wxPanel(_statusBar, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER);
	textPanel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
	textPanel->SetName("Statusbarconainer " + name);

	if (!icon.empty())
	{
		wxStaticBitmap* img = new wxStaticBitmap(textPanel, wxID_ANY, 
			wxArtProvider::GetBitmap(LocalBitmapArtProvider::ArtIdPrefix() + icon));
		textPanel->GetSizer()->Add(img, 0, wxEXPAND | wxALL, 1);
	}

	wxStaticText* label = new wxStaticText(textPanel, wxID_ANY, "");
	textPanel->GetSizer()->Add(label, 1, wxEXPAND | wxALL, 1);

	StatusBarElementPtr element(new StatusBarElement(textPanel, label));

	// Store this element
	_elements.insert(ElementMap::value_type(name, element));
	_positions.insert(PositionMap::value_type(freePos, element));

	rebuildStatusBar();
}

void StatusBarManager::setText(const std::string& name, const std::string& text)
{
	// Look up the key
	ElementMap::const_iterator found = _elements.find(name);

	// return NULL if not found
	if (found != _elements.end() && found->second->label != NULL)
	{
		// Set the text
		found->second->text = text;
		found->second->label->SetLabelMarkup(text);

		if (!text.empty())
		{
			found->second->label->SetMinClientSize(found->second->label->GetVirtualSize());
		}
		else
		{
			found->second->label->SetMinClientSize(wxSize(20, -1)); // reset to 20 pixels if empty text is passed
		}
		
		requestIdleCallback();
	}
	else
	{
		rError() << "Could not find text status bar element with the name "
			<< name << std::endl;
	}
}

void StatusBarManager::onIdle()
{
	_statusBar->PostSizeEvent();
}

int StatusBarManager::getFreePosition(int desiredPosition)
{
	// Do we have an easy job?
	if (_positions.empty())
	{
		// nothing to calculate
		return desiredPosition;
	}

	PositionMap::const_iterator i = _positions.find(desiredPosition);

	if (i == _positions.end()) {
		return desiredPosition;
	}

	// Let's see if there's space between the desired position and the next larger one
	i = _positions.upper_bound(desiredPosition);

	if (i == _positions.end()) {
		// There is no position larger than the desired one, return this one
		return desiredPosition + 1;
	}
	// Found an existing position which is larger than the desired one
	else if (i->first == desiredPosition + 1) {
		// No space between these two items, put to back
		return _positions.rbegin()->first + 1;
	}
	else {
		return desiredPosition + 1;
	}
}

void StatusBarManager::rebuildStatusBar()
{
	if (_elements.empty()) return; // done here if empty

	// Resize the table to fit the widgets
	_statusBar->GetSizer()->Clear(false); // detach all children

	std::size_t col = 0;

	for (PositionMap::const_iterator i = _positions.begin(); i != _positions.end(); ++i)
	{
		int flags = wxEXPAND | wxTOP | wxBOTTOM;

		// The first and the last status bar widget get a left/right border
		if (col == 0)
		{
			flags |= wxLEFT;
		}
		else if (col == _positions.size() - 1)
		{
			flags |= wxRIGHT;
		}

		_statusBar->GetSizer()->Add(i->second->toplevel, 10, flags, 3);

		col++;
	}

	_statusBar->Show();
}

} // namespace ui
