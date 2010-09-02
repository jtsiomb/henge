#include "renderfunc.h"

using namespace henge;

RenderFunc::RenderFunc(void (*func)(unsigned int, void*), void *cls)
{
	this->func = func;
	closure = cls;
}

void RenderFunc::operator()(unsigned int msec) const
{
	func(msec, closure);
}
