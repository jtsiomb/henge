#define HENGE_ANIM_KEY_FLT_SCALE	65535.0f

inline Quaternion catmull_rom_spline(const Quaternion &q1, const Quaternion &q2, const Quaternion &q3, const Quaternion &q4, float t)
{
	return slerp(q2, q3, t);
}

template <typename T>
TrackKey<T>::TrackKey() : time(0) {}


template <typename T>
TrackKey<T>::TrackKey(const T &v, int t)
{
	val = v;
	time = t;
}

template <typename T>
TrackKey<T>::TrackKey(const T &v, float t)
{
	val = v;
	time = (int)(HENGE_ANIM_KEY_FLT_SCALE * t);
}

template <typename T>
bool TrackKey<T>::operator ==(const TrackKey &k) const
{
	return time == k.time;
}

template <typename T>
bool TrackKey<T>::operator <(const TrackKey &k) const
{
	return time < k.time;
}


template <typename T>
Track<T>::Track()
{
	interp = INTERP_LINEAR;
	extrap = EXTRAP_CLAMP;
}

template <typename T>
TrackKey<T> *Track<T>::get_nearest_key(int time)
{
	size_t sz = keys.size();
	if(!sz) return 0;
	return get_nearest_key(0, keys.size() - 1, time);
}

// performs binary search to find the nearest keyframe to a specific time value
template <typename T>
TrackKey<T> *Track<T>::get_nearest_key(int start, int end, int time)
{
	if(start == end) return &keys[start];
	if(end - start == 1) {
		return abs((int)(time - keys[start].time)) < abs((int)(keys[end].time - time)) ? &keys[start] : &keys[end];
	}

	int mid = (start + end) / 2;
	if(time < keys[mid].time) return get_nearest_key(start, mid, time);
	if(time > keys[mid].time) return get_nearest_key(mid + 1, end, time);
	return &keys[mid];
}

/* returns (through parameters) pointers to the 2 keys that bound the interval
 * containing the specified time value.
 */
template <typename T>
void Track<T>::get_key_interval(int time, const TrackKey<T> **start, const TrackKey<T> **end) const
{
	const TrackKey<T> *nearest = ((Track<T>*)this)->get_nearest_key(time);
	if(!nearest) {
		*start = *end = 0;
		return;
	}

	*start = nearest;
	*end = 0;

	if(time < nearest->time && nearest->time != keys[0].time) {
		*start = nearest - 1;
		*end = nearest;
	} else if(time > nearest->time && nearest->time != keys[keys.size() - 1].time) {
		*start = nearest;
		*end = nearest + 1;
	}
}

template <typename T>
void Track<T>::reset(const T &val)
{
	keys.clear();
	interp = INTERP_LINEAR;
	extrap = EXTRAP_CLAMP;
	def_val = val;
}

template <typename T>
void Track<T>::set_interpolator(Interpolator interp)
{
	this->interp = interp;
}

template <typename T>
Interpolator Track<T>::get_interpolator() const
{
	return interp;
}

template <typename T>
void Track<T>::set_extrapolator(Extrapolator extrap)
{
	this->extrap = extrap;
}

template <typename T>
Extrapolator Track<T>::get_extrapolator() const
{
	return extrap;
}

template <typename T>
void Track<T>::add_key(const TrackKey<T> &key)
{
	if(!keys.empty()) {
		TrackKey<T> *nearest = get_nearest_key(key.time);
		if(nearest->time == key.time) {
			nearest->val = key.val;
		} else {
			keys.push_back(key);
			sort(keys.begin(), keys.end());
		}
	} else {
		keys.push_back(key);
	}
}

template <typename T>
TrackKey<T> *Track<T>::get_key(int time)
{
	TrackKey<T> *key = get_nearest_key(time);
	if(!key) return 0;
	return (key->time == time) ? key : 0;
}

template <typename T>
void Track<T>::delete_key(int time)
{
	TrackKey<T> key;
	key.time = time;
	typename std::vector<TrackKey<T> >::iterator iter = find(keys.begin(), keys.end(), key);
	if(iter != keys.end()) {
		keys.erase(iter);
	}
}

template <typename T>
int Track<T>::get_key_count() const
{
	return (int)keys.size();
}

template <typename T>
const TrackKey<T> &Track<T>::get_key_at(int idx) const
{
	return keys[idx];
}

template <typename T>
T Track<T>::operator()(int time) const
{
	if(keys.empty()) {
		return def_val;
	}

	/* if we're outside of our defined track range, use the designated
	 * extrapolator to modify the time
	 */
	if(keys.size() > 1) {
		int first = keys.begin()->time;
		int last = (keys.end() - 1)->time;

		if(first != last && (time < first || time > last)) {
			switch(extrap) {
			case EXTRAP_REPEAT:
				while(time < first) {
					time += last - first;
				}
				time = (time - first) % (last - first) + first;
				break;

			case EXTRAP_PINGPONG:
			default:
				// TODO implement
				break;
			}
		}
	}

	const TrackKey<T> *start, *end;
	((Track<T>*)this)->get_key_interval(time, &start, &end);
	if(!start && !end) {
		return def_val;
	}

	switch(interp) {
	case INTERP_STEP:
		return start->val;

	case INTERP_LINEAR:
		if(end) {
			// find the parametric location of the given keyframe in the range we have
			float t = (float)(time - start->time) / (float)(end->time - start->time);

			return lerp(start->val, end->val, t);
		} else {
			return start->val;
		}

	case INTERP_CUBIC:
		if(end) {
			float t = (float)(time - start->time) / (float)(end->time - start->time);

			int sidx = start - &keys[0];
			int eidx = end - &keys[0];

			T v0 = (sidx > 0  ? start - 1 : start)->val;
			T v1 = start->val;
			T v2 = end->val;
			T v3 = (eidx < (int)keys.size() - 1 ? end + 1 : end)->val;

			return catmull_rom_spline(v0, v1, v2, v3, t);

		} else {
			return start->val;
		}


	default:
		return start->val;
	}
}


template <typename T>
T Track<T>::operator()(float t) const
{
	return (*this)((int)(HENGE_ANIM_KEY_FLT_SCALE * t));
}
