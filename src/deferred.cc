#include "renderer.h"

using namespace henge;

deferred_renderer::deferred_renderer()
{
	init();
}

deferred_renderer::~deferred_renderer()
{
	shutdown();
}

bool deferred_renderer::init()
{
	return false;
}

void deferred_renderer::shutdown()
{
}

void deferred_renderer::render(const scene *scn, unsigned int msec) const
{
}
