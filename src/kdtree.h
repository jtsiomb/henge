#ifndef HENGE_KDTREE_H_
#define HENGE_KDTREE_H_

#include <list>

namespace henge {

template <typename T, typename S> struct kdnode;
template <typename T, typename S> class kdres;


template <typename T, typename S = float>
class kdtree {
private:
	int dim;
	kdnode<T, S> *root;

	void clear_rec(kdnode<T, S> *node);
	bool insert_rec(kdnode<T, S> *&node, const S *pos, const T &data, int dir);
	int nearest_rec(kdnode<T, S> *node, const S *pos, S range, kdres<T, S> *res) const;

public:
	kdtree(int k = 3);
	~kdtree();

	void clear();

	bool insert(const S *pos, const T &data);
	bool insert(S x, S y, S z, const T &data);

	kdres<T, S> *nearest(const S *pos, S range, bool ordered = false) const;
	kdres<T, S> *nearest(S x, S y, S z, S range, bool ordered = false) const;
};

template <typename T, typename S = float>
class kdres {
private:
	std::list<kdnode<T, S>*> rlist;
	typename std::list<kdnode<T, S>*>::iterator iter;
	int sz;

public:
	int size() const;

	void rewind();
	bool have_more() const;
	bool next();

	const T &get_data() const;
	const S *get_pos() const;

	friend class kdtree<T, S>;
};


}	// namespace henge

#include "kdtree.inl"

#endif	// HENGE_KDTREE_H_
