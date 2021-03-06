#pragma once

#include "icommandsystem.h"

#include "wxutil/dialog/DialogBase.h"
#include "wxutil/fsview/FileSystemView.h"
#include "wxutil/WindowPosition.h"
#include "wxutil/PathEntry.h"
#include <wx/button.h>
#include <wx/radiobut.h>

class wxStdDialogButtonSizer;

namespace ui
{

/// Dialog for browsing and selecting a map from (V)FS
class MapSelector :
    public wxutil::DialogBase
{
private:
    wxPanel* _dialogPanel;
    wxButton* _openButton;
    wxButton* _reloadButton;

    // Main tree view with the folder hierarchy
    wxutil::FileSystemView* _treeView;

    wxRadioButton* _useModPath;
    wxRadioButton* _useCustomPath;
    wxutil::PathEntry* _customPath;

    // The window position tracker
    wxutil::WindowPosition _position;

    bool _handlingSelectionChange;
    std::set<std::string> _mapFileExtensions;

private:
    // Private constructor, creates widgets
    MapSelector();

    // Helper functions to configure GUI components
    void setupTreeView(wxWindow* parent);
    void setupPathSelector(wxSizer* parentSizer);

    // Populate the tree view with files
    void populateTree();

    // Get the path that should be used for map population
    // This reflects the settings made by the user on the top of the selector window
    std::string getBasePath();

    // Return the selected map path
    std::string getSelectedPath();
    // Return the archive path of the selection
    std::string getArchivePath();

    void onOpenPath(wxCommandEvent& ev);
    void onRescanPath(wxCommandEvent& ev);
    void onItemActivated(wxDataViewEvent& ev);
    void onSelectionChanged(wxutil::FileSystemView::SelectionChangedEvent& ev);
    void onFileViewTreePopulated();
    void updateButtons();
    void onPathSelectionChanged();
    void handleItemActivated();

public:
    int ShowModal() override;

    struct Result
    {
        std::string archivePath;  // path to containing archive
        std::string selectedPath; // relative path within the archive
    };

    /**
    * Display the Selector return the path of the file selected by the user.
    */
    static Result ChooseMapFile();

    static void OpenMapFromProject(const cmd::ArgumentList& args);
};

}
