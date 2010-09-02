#ifndef HENGE_RENDERFUNC_H_
#define HENGE_RENDERFUNC_H_

namespace henge {

class RenderFunc {
private:
	void *closure;
	void (*func)(unsigned int, void*);

public:
	RenderFunc(void (*func)(unsigned int, void*) = 0, void *cls = 0);

	void operator()(unsigned int msec = 0) const;
};

}	// namespace henge

#endif	// HENGE_RENDERFUNC_H_
