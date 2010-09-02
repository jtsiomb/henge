#ifndef HENGE_KDTREE_H_
#define HENGE_KDTREE_H_

#include <list>

namespace henge {

template <typename T, typename S> struct KDNode;
template <typename T, typename S> class KDRes;


template <typename T, typename S = float>
class KDTree {
private:
	int dim;
	KDNode<T, S> *root;

	void clear_rec(KDNode<T, S> *node);
	bool insert_rec(KDNode<T, S> *&node, const S *pos, const T &data, int dir);
	int nearest_rec(KDNode<T, S> *node, const S *pos, S range, KDRes<T, S> *res) const;

public:
	KDTree(int k = 3);
	~KDTree();

	void clear();

	bool insert(const S *pos, const T &data);
	bool insert(S x, S y, S z, const T &data);

	KDRes<T, S> *nearest(const S *pos, S range, bool ordered = false) const;
	KDRes<T, S> *nearest(S x, S y, S z, S range, bool ordered = false) const;
};

template <typename T, typename S = float>
class KDRes {
private:
	std::list<KDNode<T, S>*> rlist;
	typename std::list<KDNode<T, S>*>::iterator iter;
	int sz;

public:
	int size() const;

	void rewind();
	bool have_more() const;
	bool next();

	const T &get_data() const;
	const S *get_pos() const;

	friend class KDTree<T, S>;
};


}	// namespace henge

#include "kdtree.inl"

#endif	// HENGE_KDTREE_H_
