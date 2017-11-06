#include "GuiView.h"

#include "igl.h"
#include "math/Matrix4.h"
#include <functional>

namespace gui
{

namespace
{
	const int DEFAULT_WIDTH = 640;
	const int DEFAULT_HEIGHT = 480;
}

GuiView::GuiView(wxWindow* parent) :
	GLWidget(parent, std::bind(&GuiView::draw, this), "GUI")
{
	SetMinSize(wxSize(DEFAULT_WIDTH, DEFAULT_HEIGHT));

	// greebo: The "size-allocate" event is needed to determine the window size, as expose-event is
	// often called for subsets of the widget and the size info in there is therefore not reliable.
	Connect(wxEVT_SIZE, wxSizeEventHandler(GuiView::onSizeAllocate), NULL, this);

	// Ignore visibility flag and turn invisible background images to visible ones
	_renderer.setIgnoreVisibility(true);
}

void GuiView::redraw()
{
	Refresh();
}

void GuiView::setGui(const IGuiPtr& gui)
{
	// Check for equality
	if (gui != _gui)
	{
		_gui = gui;
		_renderer.setGui(gui);
	}
}

const IGuiPtr& GuiView::getGui()
{
	return _gui;
}

void GuiView::initialiseView()
{
	
}

void GuiView::setGLViewPort()
{
	GlobalOpenGL().assertNoErrors();

	double width = _windowDims[0];
	double height = _windowDims[1];
	double aspectRatio = static_cast<double>(DEFAULT_WIDTH) / DEFAULT_HEIGHT;

	if (width / height > aspectRatio)
	{
		width = height * aspectRatio;
	}
	else
	{
		height = width / aspectRatio;
	}

	glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));

	GlobalOpenGL().assertNoErrors();
}

void GuiView::draw()
{
	if (_gui == NULL) return;

	GlobalOpenGL().assertNoErrors();

	// Clear the window
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.0, 0.0, 0.0, 0);
	glClearDepth(100.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set up the camera
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

    // Enable depth buffer writing, to be safe
    glDepthMask(GL_TRUE);

	GlobalOpenGL().assertNoErrors();

	// Prepare the GUI for rendering, like re-compiling texts etc.
	// This has to be performed before states are initialised
	_gui->pepareRendering();

	GlobalOpenGL().assertNoErrors();

	setGLViewPort();

	GlobalOpenGL().assertNoErrors();

	// Set up the scale
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	GlobalOpenGL().assertNoErrors();

	_renderer.render();
}

void GuiView::onSizeAllocate(wxSizeEvent& ev)
{
	// Store the window dimensions for later calculations
	_windowDims = Vector2(ev.GetSize().GetWidth(), ev.GetSize().GetHeight());

	// Queue an expose event
	Refresh();
}

} // namespace
