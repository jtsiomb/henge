#include "renderer.h"

using namespace henge;

DeferredRenderer::DeferredRenderer()
{
	init();
}

DeferredRenderer::~DeferredRenderer()
{
	shutdown();
}

bool DeferredRenderer::init()
{
	return false;
}

void DeferredRenderer::shutdown()
{
}

void DeferredRenderer::render(const Scene *scn, unsigned int msec) const
{
}
