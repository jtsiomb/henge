#include "renderfunc.h"

using namespace henge;

render_func::render_func(void (*func)(unsigned int, void*), void *cls)
{
	this->func = func;
	closure = cls;
}

void render_func::operator()(unsigned int msec) const
{
	func(msec, closure);
}
