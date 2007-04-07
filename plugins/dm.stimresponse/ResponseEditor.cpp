#include "ResponseEditor.h"

#include <gtk/gtk.h>
#include "gtkutil/ScrolledFrame.h"
#include "gtkutil/StockIconMenuItem.h"

namespace ui {

ResponseEditor::ResponseEditor(StimTypes& stimTypes) :
	ClassEditor(stimTypes)
{
	populatePage();
	createContextMenu();
}

void ResponseEditor::setEntity(SREntityPtr entity) {
	// Pass the call to the base class
	ClassEditor::setEntity(entity);
	
	if (entity != NULL) {
		GtkListStore* listStore = _entity->getResponseStore();
		gtk_tree_view_set_model(GTK_TREE_VIEW(_list), GTK_TREE_MODEL(listStore));
		g_object_unref(listStore); // treeview owns reference now
	}
}

void ResponseEditor::update() {
	
}

void ResponseEditor::populatePage() {
	GtkWidget* srHBox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(_pageVBox), GTK_WIDGET(srHBox), TRUE, TRUE, 0);
	
	// List and buttons below
	GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), 
		gtkutil::ScrolledFrame(_list), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), createListButtons(), FALSE, FALSE, 6);
	
	gtk_box_pack_start(GTK_BOX(srHBox),	vbox, FALSE, FALSE, 0);
	
	// Response effects section
    /*gtk_box_pack_start(GTK_BOX(_pageVBox),
    				   gtkutil::LeftAlignedLabel(LABEL_RESPONSE_EFFECTS),
    				   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(_pageVBox), 
					   gtkutil::LeftAlignment(createEffectWidgets(), 18, 1.0),
					   TRUE, TRUE, 0);*/
}

GtkWidget* ResponseEditor::createListButtons() {
	GtkWidget* hbox = gtk_hbox_new(TRUE, 6);
	
	_listButtons.add = gtk_button_new_with_label("Add new Response");
	gtk_button_set_image(
		GTK_BUTTON(_listButtons.add), 
		gtk_image_new_from_stock(GTK_STOCK_ADD, GTK_ICON_SIZE_BUTTON)
	);
	
	_listButtons.remove = gtk_button_new_with_label("Remove Response");
	gtk_button_set_image(
		GTK_BUTTON(_listButtons.remove), 
		gtk_image_new_from_stock(GTK_STOCK_DELETE, GTK_ICON_SIZE_BUTTON)
	);
	
	gtk_box_pack_start(GTK_BOX(hbox), _listButtons.add, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), _listButtons.remove, TRUE, TRUE, 0);
	
	g_signal_connect(G_OBJECT(_listButtons.add), "clicked", G_CALLBACK(onAddResponse), this);
	g_signal_connect(G_OBJECT(_listButtons.remove), "clicked", G_CALLBACK(onRemoveResponse), this);
	
	return hbox; 
}

// Create the context menus
void ResponseEditor::createContextMenu() {
	// Menu widgets
	_contextMenu.menu = gtk_menu_new();
		
	// Each menu gets a delete item
	_contextMenu.remove = gtkutil::StockIconMenuItem(GTK_STOCK_DELETE, "Delete Response");
	_contextMenu.add = gtkutil::StockIconMenuItem(GTK_STOCK_ADD, "Add Response");
	gtk_menu_shell_append(GTK_MENU_SHELL(_contextMenu.menu), _contextMenu.add);
	gtk_menu_shell_append(GTK_MENU_SHELL(_contextMenu.menu), _contextMenu.remove);

	// Connect up the signals
	g_signal_connect(G_OBJECT(_contextMenu.remove), "activate",
					 G_CALLBACK(onListContextMenuDelete), this);
	g_signal_connect(G_OBJECT(_contextMenu.add), "activate",
					 G_CALLBACK(onListContextMenuAdd), this);
					 
	// Show menus (not actually visible until popped up)
	gtk_widget_show_all(_contextMenu.menu);
}

void ResponseEditor::openContextMenu(GtkTreeView* view) {
	// Check the treeview this remove call is targeting
	if (view == GTK_TREE_VIEW(_list)) {
		gtk_menu_popup(GTK_MENU(_contextMenu.menu), NULL, NULL, 
					   NULL, NULL, 1, GDK_CURRENT_TIME);	
	}
}

void ResponseEditor::removeItem(GtkTreeView* view) {
	// Check the treeview this remove call is targeting
	if (view == GTK_TREE_VIEW(_list)) {	
		// Get the selected stim ID
		int id = getIdFromSelection();
		
		if (id > 0) {
			_entity->remove(id);
		}
	}
}

void ResponseEditor::selectionChanged() {
	
}

void ResponseEditor::addResponse() {
	if (_entity == NULL) return;

	// Create a new StimResponse object
	int id = _entity->add();
	
	// Get a reference to the newly allocated object
	StimResponse& sr = _entity->get(id);
	sr.set("class", "R");
	sr.set("type", _stimTypes.getFirstName());

	// Refresh the values in the liststore
	_entity->updateListStores();
}

// Static GTK Callbacks

// Delete context menu items activated
void ResponseEditor::onListContextMenuDelete(GtkWidget* w, ResponseEditor* self) {
	// Delete the selected stim from the list
	self->removeItem(GTK_TREE_VIEW(self->_list));
}

// Delete context menu items activated
void ResponseEditor::onListContextMenuAdd(GtkWidget* w, ResponseEditor* self) {
	self->addResponse();
}

void ResponseEditor::onAddResponse(GtkWidget* button, ResponseEditor* self) {
	self->addResponse();
}

void ResponseEditor::onRemoveResponse(GtkWidget* button, ResponseEditor* self) {
	// Delete the selected stim from the list
	self->removeItem(GTK_TREE_VIEW(self->_list));
}

} // namespace ui
