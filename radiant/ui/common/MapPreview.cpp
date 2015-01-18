#include "MapPreview.h"

#include "ifilter.h"
#include "iscenegraph.h"
#include "iuimanager.h"
#include "iscenegraphfactory.h"
#include "math/AABB.h"

namespace ui
{

MapPreview::MapPreview(wxWindow* parent) : 
	RenderPreview(parent)
{}

void MapPreview::setRootNode(const scene::IMapRootNodePtr& root)
{
	getScene()->setRoot(root);

	if (getScene()->root() != NULL)
	{
		// Re-associate the rendersystem, we need to propagate this info to the nodes
		associateRenderSystem();

		// Trigger an initial update of the subgraph
		GlobalFilterSystem().updateSubgraph(getScene()->root());

		// Calculate camera distance so map is appropriately zoomed
        // TODO _camDist = -(getScene()->root()->worldAABB().getRadius() * 2.0f);

		// TODO _rotation = Matrix4::getIdentity();
	}
}

scene::IMapRootNodePtr MapPreview::getRootNode()
{
	return getScene()->root();
}

AABB MapPreview::getSceneBounds()
{
	if (!getScene()->root()) return RenderPreview::getSceneBounds();

	return getScene()->root()->worldAABB();
}

bool MapPreview::onPreRender()
{
	// Trigger scenegraph instantiation
	getScene();

	return getScene()->root() != NULL;
}

RenderStateFlags MapPreview::getRenderFlagsFill()
{
	return RenderPreview::getRenderFlagsFill() | RENDER_DEPTHWRITE | RENDER_DEPTHTEST;
}

} // namespace ui
