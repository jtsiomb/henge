namespace henge {

template <typename T, typename S>
struct KDNode {
	S *pos;
	int dir;
	T data;
	S dist_sq;		// used for result set ordering by distance

	KDNode<T, S> *left, *right;

	bool operator <(const KDNode<T, S> &rhs) const;
};


template <typename T, typename S>
bool KDNode<T, S>::operator <(const KDNode<T, S> &rhs) const
{
	return dist_sq < rhs.dist_sq;
}

template <typename T, typename S>
KDTree<T, S>::KDTree(int k)
{
	dim = k;
	root = 0;
}

template <typename T, typename S>
KDTree<T, S>::~KDTree()
{
	clear();
}


template <typename T, typename S>
void KDTree<T, S>::clear_rec(KDNode<T, S> *node)
{
	if(!node) return;

	clear_rec(node->left);
	clear_rec(node->right);

	delete [] node->pos;
	delete node;
}

template <typename T, typename S>
bool KDTree<T, S>::insert_rec(KDNode<T, S> *&node, const S *pos, const T &data, int dir)
{
	if(!node) {
		try {
			node = new KDNode<T, S>;
			node->pos = new S[dim];
		}
		catch(...) {
			delete node;
			return false;
		}

		memcpy(node->pos, pos, dim * sizeof *node->pos);
		node->data = data;
		node->dir = dir;
		node->left = node->right = 0;
		return true;
	}

	int new_dir = (node->dir + 1) % dim;
	if(pos[node->dir] < node->pos[node->dir]) {
		return insert_rec(node->left, pos, data, new_dir);
	}
	return insert_rec(node->right, pos, data, new_dir);
}

template <typename T, typename S>
int KDTree<T, S>::nearest_rec(KDNode<T, S> *node, const S *pos, S range, KDRes<T, S> *res) const
{
	int added_res = 0;

	if(!node) return 0;

	S dist_sq = 0;
	for(int i=0; i<dim; i++) {
		dist_sq += SQ(node->pos[i] - pos[i]);
	}
	if(dist_sq <= SQ(range)) {
		res->rlist.push_back(node);
		node->dist_sq = dist_sq;
		added_res = true;
	}

	S dx = pos[node->dir] - node->pos[node->dir];

	int ret = nearest_rec(dx <= 0.0 ? node->left : node->right, pos, range, res);
	if(fabs(dx) < range) {
		added_res += ret;
		ret = nearest_rec(dx <= 0.0 ? node->right : node->left, pos, range, res);
	}
	added_res += ret;

	return added_res;
}

template <typename T, typename S>
void KDTree<T, S>::clear()
{
	clear_rec(root);
}

template <typename T, typename S>
bool KDTree<T, S>::insert(const S *pos, const T &data)
{
	return insert_rec(root, pos, data, 0);
}

template <typename T, typename S>
bool KDTree<T, S>::insert(S x, S y, S z, const T &data)
{
	S *buf;
	try {
		buf = new S[dim];
	}
	catch(...) {
		return false;
	}
	memset(buf, 0, dim * sizeof *buf);

	if(dim > 0) buf[0] = x;
	if(dim > 1) buf[1] = y;
	if(dim > 2) buf[2] = z;

	bool res = insert(buf, data);

	delete [] buf;
	return res;
}

template <typename T, typename S>
KDRes<T, S> *KDTree<T, S>::nearest(const S *pos, S range, bool ordered) const
{
	KDRes<T, S> *res = new KDRes<T, S>;

	int num = nearest_rec(root, pos, range, res);

	if(ordered) {
		res->rlist.sort();
	}

	res->rewind();
	res->sz = num;
	return res;
}

template <typename T, typename S>
KDRes<T, S> *KDTree<T, S>::nearest(S x, S y, S z, S range, bool ordered) const
{
	S *buf;
	try {
		buf = new S[dim];
	}
	catch(...) {
		return false;
	}
	memset(buf, 0, dim * sizeof *buf);

	if(dim > 0) buf[0] = x;
	if(dim > 1) buf[1] = y;
	if(dim > 2) buf[2] = z;

	KDRes<T, S> *res = nearest(buf, range, ordered);

	delete [] buf;
	return res;
}


template <typename T, typename S>
int KDRes<T, S>::size() const
{
	return sz;
}

template <typename T, typename S>
void KDRes<T, S>::rewind()
{
	iter = rlist.begin();
}

template <typename T, typename S>
bool KDRes<T, S>::have_more() const
{
	return iter != rlist.end();
}

template <typename T, typename S>
bool KDRes<T, S>::next()
{
	iter++;
	return iter != rlist.end();
}

template <typename T, typename S>
const T &KDRes<T, S>::get_data() const
{
	return (*iter)->data;
}

template <typename T, typename S>
const S *KDRes<T, S>::get_pos() const
{
	return (*iter)->pos;
}

}		// namespace henge
