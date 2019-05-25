
#include "Math.hpp"
#include "CameraUtils.hpp"

using namespace Heerbann;

Frustum::Frustum() {
	for (uint i = 0; i < 6u; ++i)
		planes.emplace_back(new Plane());
}

void Frustum::update(Mat4 _combined) {
	planes[FrustumPlane::Right]->set(Vec4(_combined[0][3] - _combined[0][0],
		_combined[1][3] - _combined[1][0],
		_combined[2][3] - _combined[2][0], 1.f),
		_combined[3][3] - _combined[3][0]);

	planes[FrustumPlane::Left]->set(Vec4(_combined[0][3] + _combined[0][0],
		_combined[1][3] + _combined[1][0],
		_combined[2][3] + _combined[2][0], 1.f),
		_combined[3][3] + _combined[3][0]);

	planes[FrustumPlane::Bottom]->set(Vec4(_combined[0][3] + _combined[0][1],
		_combined[1][3] + _combined[1][1],
		_combined[2][3] + _combined[2][1], 1.f),
		_combined[3][3] + _combined[3][1]);

	planes[FrustumPlane::Top]->set(Vec4(_combined[0][3] - _combined[0][1],
		_combined[1][3] - _combined[1][1],
		_combined[2][3] - _combined[2][1], 1.f),
		_combined[3][3] - _combined[3][1]);

	planes[FrustumPlane::Far]->set(Vec4(_combined[0][3] - _combined[0][2],
		_combined[1][3] - _combined[1][2],
		_combined[2][3] - _combined[2][2], 1.f),
		_combined[3][3] - _combined[3][2]);

	planes[FrustumPlane::Near]->set(Vec4(_combined[0][3] + _combined[0][2],
		_combined[1][3] + _combined[1][2],
		_combined[2][3] + _combined[2][2], 1.f),
		_combined[3][3] + _combined[3][2]);
}

bool Frustum::pointInFrustum(const Vec4& _point) {
	return pointInFrustum(_point.x, _point.y, _point.z);
}

bool Frustum::pointInFrustum(float _x, float _y, float _z) {
	for (uint i = 0; i < 6u; ++i) {
		Plane::PlaneSide result = planes[i]->testPoint(_x, _y, _z);
		if (result == Plane::PlaneSide::Back) return false;
	}
	return true;
}

bool Frustum::sphereInFrustum(const Vec4& _center, float _radius) {
	return sphereInFrustum(_center.x, _center.y, _center.z, _radius);
}

bool Frustum::sphereInFrustum(float _x, float _y, float _z, float _radius) {
	for (int i = 0; i < 6; ++i)
		if ((planes[i]->normal.x * _x + planes[i]->normal.y * _y + planes[i]->normal.z * _z) < (-_radius - planes[i]->d)) return false;
	return true;
}

bool Frustum::sphereInFrustumWithoutNearFar(const Vec4& _center, float _radius) {
	return sphereInFrustumWithoutNearFar(_center.x, _center.y, _center.z, _radius);
}

bool Frustum::sphereInFrustumWithoutNearFar(float _x, float _y, float _z, float _radius) {
	for (int i = 2; i < 6; ++i)
		if ((planes[i]->normal.x * _x + planes[i]->normal.y * _y + planes[i]->normal.z * _z) < (-_radius - planes[i]->d)) return false;
	return true;
}

bool Frustum::boundsInFrustum(BoundingBox* _box) {
	return boundsInFrustum(_box->cnt, _box->dim);
}

bool Frustum::boundsInFrustum(const Vec4& _center, const Vec4& _dim) {
	return boundsInFrustum(_center.x, _center.y, _center.z, _dim.x, _dim.y, _dim.z);
}

bool Frustum::boundsInFrustum(float _x, float _y, float _z, float _halfWidth, float _halfHeight, float _halfDepth) {
	for (int i = 0, len2 = 6; i < len2; ++i) {
		if (planes[i]->testPoint(_x + _halfWidth, _y + _halfHeight, _z + _halfDepth) != Plane::PlaneSide::Back) continue;
		if (planes[i]->testPoint(_x + _halfWidth, _y + _halfHeight, _z - _halfDepth) != Plane::PlaneSide::Back) continue;
		if (planes[i]->testPoint(_x + _halfWidth, _y - _halfHeight, _z + _halfDepth) != Plane::PlaneSide::Back) continue;
		if (planes[i]->testPoint(_x + _halfWidth, _y - _halfHeight, _z - _halfDepth) != Plane::PlaneSide::Back) continue;
		if (planes[i]->testPoint(_x - _halfWidth, _y + _halfHeight, _z + _halfDepth) != Plane::PlaneSide::Back) continue;
		if (planes[i]->testPoint(_x - _halfWidth, _y + _halfHeight, _z - _halfDepth) != Plane::PlaneSide::Back) continue;
		if (planes[i]->testPoint(_x - _halfWidth, _y - _halfHeight, _z + _halfDepth) != Plane::PlaneSide::Back) continue;
		if (planes[i]->testPoint(_x - _halfWidth, _y - _halfHeight, _z - _halfDepth) != Plane::PlaneSide::Back) continue;
		return false;
	}
	return true;
}

BoundingBox* Frustum::toAABB(Camera* _cam) {
	if (bounds == nullptr) {
		bounds = new BoundingBox();
		clipPoints = std::vector<Vec4>(8);
		clipPoints[0] = Vec4(-1.f, -1.f, 0.f, 1.f);
		clipPoints[1] = Vec4(-1.f, 1.f, 0.f, 1.f);
		clipPoints[2] = Vec4(1.f, 1.f, 0.f, 1.f);
		clipPoints[3] = Vec4(1.f, -1.f, 0.f, 1.f);
		clipPoints[4] = Vec4(-1.f, -1.f, 1.f, 1.f);
		clipPoints[5] = Vec4(-1.f, 1.f, 1.f, 1.f);
		clipPoints[6] = Vec4(1.f, 1.f, 1.f, 1.f);
		clipPoints[7] = Vec4(1.f, -1.f, 1.f, 1.f);
	}
	bounds->clr();
	auto inv = INV(_cam->combined);
	for (int i = 0; i < 8; i++) {
		Vec4 res = inv * clipPoints[i];
		bounds->ext(res / res.w);
	}
	return bounds;
}

std::vector<Vec4> Frustum::getPoints(Camera* _cam) {
	if (bounds == nullptr) {
		bounds = new BoundingBox();
		clipPoints = std::vector<Vec4>(8);
		clipPoints[0] = Vec4(-1.f, -1.f, 0.f, 1.f);
		clipPoints[1] = Vec4(-1.f, 1.f, 0.f, 1.f);
		clipPoints[2] = Vec4(1.f, 1.f, 0.f, 1.f);
		clipPoints[3] = Vec4(1.f, -1.f, 0.f, 1.f);
		clipPoints[4] = Vec4(-1.f, -1.f, 1.f, 1.f);
		clipPoints[5] = Vec4(-1.f, 1.f, 1.f, 1.f);
		clipPoints[6] = Vec4(1.f, 1.f, 1.f, 1.f);
		clipPoints[7] = Vec4(1.f, -1.f, 1.f, 1.f);
	}
	std::vector<Vec4> out(8);
	auto inv = INV(_cam->combined);
	for (int i = 0; i < 8; i++)
		out[i] = inv * clipPoints[i];
	return out;
}

Ray::Ray() {
	origin = Vec4(0.f, 0.f, 0.f, 1.f);
	direction = Vec4(0.f, 0.f, 0.f, 1.f);
}

Ray::Ray(const Ray& _ray) {
	origin = Vec4(_ray.origin);
	direction = Vec4(_ray.direction);
}

Ray::Ray(const Vec4& _origin, const Vec4& _direction) {
	origin = Vec4(_origin);
	direction = Vec4(_direction);
}

Vec4 Ray::getEndPoint(float _distance) {
	Vec4 out(direction);
	out *= _distance;
	out += origin;
	return out;
}

Ray* Ray::set(Ray* _ray) {
	return set(_ray->origin, _ray->direction);
}

Ray* Ray::operator*(Mat4 _mat) {
	Vec4 tmp(origin);
	origin += direction;
	tmp = _mat * tmp;
	origin = _mat * origin;
	direction = Vec4(tmp - origin);
	return this;
}

Ray* Ray::set(const Vec4& _origin, const Vec4& _direction) {
	return set(_origin.x, _origin.y, _origin.z, _direction.x, _direction.y, _direction.z);
}

Ray* Ray::set(float _x, float _y, float _z, float _dx, float _dy, float _dz) {
	origin = Vec4(_x, _y, _z, 1.f);
	direction = Vec4(_dx, _dy, _dz, 1.f);
	return this;
}

Plane::Plane() : Plane(Vec4(0.f, 0.f, 0.f, 1.f), 0.f) {}

Plane::Plane(const Vec4& _normal, float _d) : normal(NOR(_normal)), d(_d) {}

Plane::Plane(const Plane& _plane) : Plane(_plane.normal, _plane.d) {}

Plane::Plane(const Vec4& _normal, const Vec4& _point) : Plane(_normal,
	-(_normal.x + _point.x + _normal.y + _point.y + _normal.z + _point.z)) {
}

Plane::Plane(const Vec4& _point1, const Vec4& _point2, const Vec4& _point3) {
	set(_point1, _point2, _point3);
}

Plane* Plane::set(const Plane& _plane) {
	return set(_plane.normal, _plane.d);
}

Plane* Plane::set(const Vec4& _normal, float _d) {
	normal = Vec4(NOR(_normal));
	d = _d;
	return this;
}

Plane* Plane::set(const Vec4& _point1, const Vec4& _point2, const Vec4& _point3) {
	normal = _point1 - _point2;
	normal = Vec4(NOR(CRS(Vec3(normal.x, normal.y, normal.z), Vec3(_point2.x - _point3.x, _point2.y - _point3.y, _point2.z - _point3.z))), 1.f);
	d = -(normal.x + _point1.x + normal.y + _point1.y + normal.z + _point1.z);
	return this;
}

Plane* Plane::set(float _nx, float _ny, float _nz, float _d) {
	normal = Vec4(_nx, _ny, _nz, 1.f);
	d = _d;
	return this;
}

float Plane::distance(const Vec4& _point) {
	return -(normal.x + _point.x + normal.y + _point.y + normal.z + _point.z) + d;
}

Plane::PlaneSide Plane::testPoint(const Vec4& _point) {
	float dist = distance(_point);
	if (ISNULL(dist))
		return Plane::PlaneSide::OnPlane;
	else if (dist < 0)
		return Plane::PlaneSide::Back;
	else
		return Plane::PlaneSide::Front;
}

Plane::PlaneSide Plane::testPoint(float _x, float _y, float _z) {
	return testPoint(Vec4(_x, _y, _z, 1.f));
}

bool Plane::isFrontFacing(const Vec4& _point) {
	float dot = normal.x + _point.x + normal.y + _point.y + normal.z + _point.z;
	return dot < 0.f || ISNULL(dot);
}

BoundingBox::BoundingBox() {
	clr();
}

BoundingBox::BoundingBox(BoundingBox* _box) {
	set(_box);
}

BoundingBox::BoundingBox(const Vec4& _min, const Vec4& _max) {
	set(_min, _max);
}

float BoundingBox::computeSurfaceArea() const {
	// Sum of "area" of all the sides.
	float sum = 0;

	// General formula for one side: hold one dimension constant
	// and multiply by all the other ones.
	for (uint d1 = 0; d1 < 3; ++d1) {
		// "Area" of current side.
		float product = 1;

		for (uint d2 = 0; d2 < 3; ++d2) {
			if (d1 == d2)
				continue;

			float dx = max[d2] - min[d2];
			product *= dx;
		}
		sum += product;
	}
	return 2.f * sum;
}

float BoundingBox::getSurfaceArea() const {
	return surfaceArea;
}

Vec4 BoundingBox::computeCentre() {
	return Vec4(); //TODO
}

BoundingBox* BoundingBox::set(BoundingBox* _box) {
	return set(_box->min, _box->max);
}

BoundingBox* BoundingBox::set(const Vec4& _min, const Vec4& _max) {
	min = Vec4(std::min(_min.x, _max.x), std::min(_min.y, _max.y), std::min(_min.z, _max.z), 1.f);
	max = Vec4(std::max(_min.x, _max.x), std::max(_min.y, _max.y), std::max(_min.z, _max.z), 1.f);
	cnt = (min + max) * 0.5f;
	dim = max - min;
	return this;
}

BoundingBox* BoundingBox::set(const std::vector<Vec4>& _points) {
	inf();
	for (auto& v : _points)
		ext(v);
	return this;
}

BoundingBox* BoundingBox::inf() {
	min = Vec4(INF, INF, INF, 1.f);
	max = Vec4(-INF, -INF, -INF, 1.f);
	cnt = Vec4(0.f, 0.f, 0.f, 1.f);
	dim = Vec4(0.f, 0.f, 0.f, 1.f);
	return this;
}

BoundingBox* BoundingBox::ext(const Vec4& _point) {
	return set(Vec4(std::min(min.x, _point.x), std::min(min.y, _point.y), std::min(min.z, _point.z), 1.f),
		Vec4(std::max(max.x, _point.x), std::max(max.y, _point.y), std::max(max.z, _point.z), 1.f));
}

BoundingBox* BoundingBox::clr() {
	return set(Vec4(0.f, 0.f, 0.f, 1.f), Vec4(0.f, 0.f, 0.f, 1.f));
}

bool BoundingBox::isValid() {
	return min.x < max.x && EQUAL(min.x, max.x) &&
		(min.y < max.y || EQUAL(min.y, max.y)) &&
		(min.z < max.z || EQUAL(min.z, max.z));
}

BoundingBox * BoundingBox::merge(BoundingBox* _box) {
	return ext(_box);
}

BoundingBox* BoundingBox::ext(BoundingBox* _box) {
	return ext(_box->min, _box->max);
}

BoundingBox * BoundingBox::ext(const Vec4& _min, const Vec4& _max) {
	return set(Vec4(std::min(min.x, _min.x), std::min(min.y, _min.y), std::min(min.z, _min.z), 1.f),
		Vec4(std::max(max.x, _max.x), std::max(max.y, _max.y), std::max(max.z, _max.z), 1.f));
}

BoundingBox* BoundingBox::ext(const Vec4& _centre, float _radius) {
	return set(Vec4(std::min(min.x, _centre.x - _radius), std::min(min.y, _centre.y - _radius), std::min(min.z, _centre.z - _radius), 1.f),
		Vec4(std::max(max.x, _centre.x + _radius), std::max(max.y, _centre.y + _radius), std::max(max.z, _centre.z + _radius), 1.f));
}

BoundingBox* BoundingBox::ext(BoundingBox* _box, Mat4 _transform) {
	ext(_transform * Vec4(_box->min.x, _box->min.y, _box->min.z, 1.f));
	ext(_transform * Vec4(_box->min.x, _box->min.y, _box->max.z, 1.f));
	ext(_transform * Vec4(_box->min.x, _box->max.y, _box->min.z, 1.f));
	ext(_transform * Vec4(_box->min.x, _box->max.y, _box->max.z, 1.f));
	ext(_transform * Vec4(_box->max.x, _box->min.y, _box->min.z, 1.f));
	ext(_transform * Vec4(_box->max.x, _box->min.y, _box->max.z, 1.f));
	ext(_transform * Vec4(_box->max.x, _box->max.y, _box->min.z, 1.f));
	ext(_transform * Vec4(_box->max.x, _box->max.y, _box->max.z, 1.f));
	return this;
}

BoundingBox * BoundingBox::ext(const sf::FloatRect& _rect) {
	return ext(Vec4(_rect.left, _rect.top - _rect.height, 0.f, 1.f),
		Vec4(_rect.left + _rect.width, _rect.top, 0.f, 1.f));
}

bool BoundingBox::contains(BoundingBox* _box) {
	return !isValid()
		|| ((min.x < _box->min.x || EQUAL(min.x, _box->min.x)) &&
		(min.y < _box->min.y || EQUAL(min.y, _box->min.y)) &&
			(min.z < _box->min.z || EQUAL(min.z, _box->min.z)) &&
			(max.x > _box->max.x || EQUAL(max.x, _box->max.x)) &&
			(max.y > _box->max.y || EQUAL(max.y, _box->max.y)) &&
			(max.z > _box->max.z || EQUAL(max.z, _box->max.z)));
}

bool BoundingBox::contains(const Vec4& _point) {
	return contains(_point.x, _point.y, _point.z);
}

bool BoundingBox::contains(float _x, float _y, float _z) {
	return (min.x < _x || EQUAL(min.x, _x)) &&
		(max.x > _x || EQUAL(max.x, _x)) &&
		(min.y < _y || EQUAL(min.y, _y)) &&
		(max.y > _y || EQUAL(max.y, _y)) &&
		(min.z < _z || EQUAL(min.z, _z)) &&
		(max.z > _z || EQUAL(max.z, _z));
}

bool BoundingBox::intersects(BoundingBox* _box) {
	if (!isValid()) return false;

	// test using SAT (separating axis theorem)

	float lx = std::abs(cnt.x - _box->cnt.x);
	float sumx = (dim.x / 2.0f) + (_box->dim.x / 2.0f);

	float ly = std::abs(cnt.y - _box->cnt.y);
	float sumy = (dim.y / 2.0f) + (_box->dim.y / 2.0f);

	float lz = std::abs(cnt.z - _box->cnt.z);
	float sumz = (dim.z / 2.0f) + (_box->dim.z / 2.0f);

	return (lx <= sumx && ly <= sumy && lz <= sumz);
}

BoundingBox* BoundingBox::operator*= (Mat4 _transform) {
	float x0 = min.x, y0 = min.y, z0 = min.z, x1 = max.x, y1 = max.y, z1 = max.z;
	inf();
	ext(_transform * Vec4(x0, y0, z0, 1.f));
	ext(_transform * Vec4(x0, y0, z1, 1.f));
	ext(_transform * Vec4(x0, y1, z0, 1.f));
	ext(_transform * Vec4(x0, y1, z1, 1.f));
	ext(_transform * Vec4(x1, y0, z0, 1.f));
	ext(_transform * Vec4(x1, y0, z1, 1.f));
	ext(_transform * Vec4(x1, y1, z0, 1.f));
	ext(_transform * Vec4(x1, y1, z1, 1.f));
	return this;
}

Quat setFromAxes(const Vec3& _x, const Vec3& _y, const Vec3& _z) {
	
	Quat out;
	Vec3 x = NOR(_x);
	Vec3 y = NOR(_y);
	Vec3 z = NOR(_z);

	const float xx = x.x;
	const float xy = x.y;
	const float xz = x.z;

	const float yx = y.x;
	const float yy = y.y;
	const float yz = y.z;

	const float zx = z.x;
	const float zy = z.y;
	const float zz = z.z;

	// the trace is the sum of the diagonal elements; see
	// http://mathworld.wolfram.com/MatrixTrace.html
	const float t = xx + yy + zz;

	// we protect the division by s by ensuring that s>=1
	if (t >= 0) { // |w| >= .5
		float s = SQRT(t + 1); // |s|>=1 ...
		out.w = 0.5f * s;
		s = 0.5f / s; // so this division isn't bad
		out.x = (zy - yz) * s;
		out.y = (xz - zx) * s;
		out.z = (yx - xy) * s;
	} else if ((xx > yy) && (xx > zz)) {
		float s = SQRT(1.f + xx - yy - zz); // |s|>=1
		out.x = s * 0.5f; // |x| >= .5
		s = 0.5f / s;
		out.y = (yx + xy) * s;
		out.z = (xz + zx) * s;
		out.w = (zy - yz) * s;
	} else if (yy > zz) {
		float s = SQRT(1.f + yy - xx - zz); // |s|>=1
		out.y = s * 0.5f; // |y| >= .5
		s = 0.5f / s;
		out.x = (yx + xy) * s;
		out.z = (zy + yz) * s;
		out.w = (xz - zx) * s;
	} else {
		float s = SQRT(1.f + zz - xx - yy); // |s|>=1
		out.z = s * 0.5f; // |z| >= .5
		s = 0.5f / s;
		out.x = (xz + zx) * s;
		out.y = (zy + yz) * s;
		out.w = (yx - xy) * s;
	}

	return out;
}

AABBTreeNode::AABBTreeNode() {}

bool AABBTreeNode::isLeaf() const {
	return (left == NULL_NODE);
}

AABBTree::AABBTree(float _skinThickness, uint _nParticles) : skinThickness(_skinThickness) {

	// Initialise the AABBTree.
	root = NULL_NODE;
	nodeCount = 0;
	nodeCapacity = _nParticles;
	nodes.resize(nodeCapacity);

	// Build a linked list for the list of free nodes.
	for (uint i = 0; i < nodeCapacity - 1; i++) {
		nodes[i].next = i + 1;
		nodes[i].height = -1;
	}
	nodes[nodeCapacity - 1].next = NULL_NODE;
	nodes[nodeCapacity - 1].height = -1;

	// Assign the index of the first free node.
	freeList = 0;
}

AABBTree::AABBTree() : AABBTree(0.05f, 16u){}

uint AABBTree::allocateNode() {
	// Exand the node pool as needed.
	if (freeList == NULL_NODE) {
		assert(nodeCount == nodeCapacity);

		// The free list is empty. Rebuild a bigger pool.
		nodeCapacity *= 2;
		nodes.resize(nodeCapacity);

		// Build a linked list for the list of free nodes.
		for (uint i = nodeCount; i < nodeCapacity - 1; i++) {
			nodes[i].next = i + 1;
			nodes[i].height = -1;
		}
		nodes[nodeCapacity - 1].next = NULL_NODE;
		nodes[nodeCapacity - 1].height = -1;

		// Assign the index of the first free node.
		freeList = nodeCount;
	}

	// Peel a node off the free list.
	uint node = freeList;
	freeList = nodes[node].next;
	nodes[node].parent = NULL_NODE;
	nodes[node].left = NULL_NODE;
	nodes[node].right = NULL_NODE;
	nodes[node].height = 0;
	nodeCount++;

	return node;
}

void AABBTree::freeNode(uint _node) {
	assert(_node < nodeCapacity);
	assert(0 < nodeCount);

	nodes[_node].next = freeList;
	nodes[_node].height = -1;
	freeList = _node;
	nodeCount--;
}

void AABBTree::insertParticle(uint _particle, Vec4& _position, float _radius) {
	// Make sure the particle doesn't already exist.
	assert(particleMap.count(_particle) != 0 && "[ERROR]: Particle already exists in AABBTree!");

	// Allocate a new node for the particle.
	uint node = allocateNode();

	// AABB size in each dimension.
	Vec4 size;

	// Compute the AABB limits.
	nodes[node].aabb->min = _position - _radius;
	nodes[node].aabb->max = _position + _radius;
	size = Vec4(nodes[node].aabb->max - nodes[node].aabb->min);

	// Fatten the AABB.
	nodes[node].aabb->min -= skinThickness * size;
	nodes[node].aabb->max += skinThickness * size;

	nodes[node].aabb->surfaceArea = nodes[node].aabb->computeSurfaceArea();
	nodes[node].aabb->cnt = nodes[node].aabb->computeCentre();

	// Zero the height.
	nodes[node].height = 0;

	// Insert a new leaf into the AABBTree.
	insertLeaf(node);

	// Add the new particle to the map.
	particleMap.insert(std::map<uint, uint>::value_type(_particle, node));

	// Store the particle index.
	nodes[node].particle = _particle;
}

void AABBTree::insertParticle(uint _particle, Vec4& _lowerBound, Vec4& _upperBound) {
	// Make sure the particle doesn't already exist.
	assert(particleMap.count(_particle) != 0 && "[ERROR]: Particle already exists in AABBTree!");

	// Allocate a new node for the particle.
	uint node = allocateNode();

	// AABB size in each dimension.
	Vec4 size;

	// Compute the AABB limits.
	for (uint i = 0; i < 4; ++i) {
		// Validate the bound.
		assert(_lowerBound[i] > _upperBound[i] && "[ERROR]: AABB lower bound is greater than the upper bound!");

		nodes[node].aabb->min[i] = _lowerBound[i];
		nodes[node].aabb->max[i] = _upperBound[i];
		size[i] = _upperBound[i] - _lowerBound[i];
	}

	// Fatten the AABB.
	nodes[node].aabb->min -= skinThickness * size;
	nodes[node].aabb->max += skinThickness * size;

	nodes[node].aabb->surfaceArea = nodes[node].aabb->computeSurfaceArea();
	nodes[node].aabb->cnt = nodes[node].aabb->computeCentre();

	// Zero the height.
	nodes[node].height = 0;

	// Insert a new leaf into the AABBTree.
	insertLeaf(node);

	// Add the new particle to the map.
	particleMap.insert(std::map<uint, uint>::value_type(_particle, node));

	// Store the particle index.
	nodes[node].particle = _particle;
}

void AABBTree::insertParticle(uint _particle, BoundingBox* _aabb) {
	insertParticle(_particle, _aabb->min, _aabb->max);
}

uint AABBTree::nParticles() {
	return static_cast<uint>(particleMap.size());
}

void AABBTree::removeParticle(uint _particle) {
	// Map iterator.
	std::map<uint, uint>::iterator it;

	// Find the particle.
	it = particleMap.find(_particle);

	// The particle doesn't exist.
	if (it == particleMap.end()) throw std::invalid_argument("[ERROR]: Invalid particle index!");

	// Extract the node index.
	uint node = it->second;

	// Erase the particle from the map.
	particleMap.erase(it);

	assert(node < nodeCapacity);
	assert(nodes[node].isLeaf());

	removeLeaf(node);
	freeNode(node);
}

void AABBTree::removeAll() {
	// Iterator pointing to the start of the particle map.
	std::map<uint, uint>::iterator it = particleMap.begin();

	// Iterate over the map.
	while (it != particleMap.end()) {
		// Extract the node index.
		uint node = it->second;

		assert(node < nodeCapacity);
		assert(nodes[node].isLeaf());

		removeLeaf(node);
		freeNode(node);

		++it;
	}

	// Clear the particle map.
	particleMap.clear();
}

bool AABBTree::updateParticle(uint _particle, Vec4 _position, float _radius, bool _alwaysReinsert) {

	// AABB bounds vectors.
	Vec4 lowerBound;
	Vec4 upperBound;

	// Compute the AABB limits.
	lowerBound = _position - _radius;
	upperBound = _position + _radius;

	// Update the particle.
	return updateParticle(_particle, lowerBound, upperBound, _alwaysReinsert);
}

bool AABBTree::updateParticle(uint particle, Vec4& lowerBound, Vec4& upperBound, bool alwaysReinsert) {

	// Map iterator.
	std::map<uint, uint>::iterator it;

	// Find the particle.
	it = particleMap.find(particle);

	// The particle doesn't exist.
	assert(it == particleMap.end() && "[ERROR]: Invalid particle index!");

	// Extract the node index.
	uint node = it->second;

	assert(node < nodeCapacity);
	assert(nodes[node].isLeaf());

	// AABB size in each dimension.
	Vec4 size;

	// Compute the AABB limits.
	for (uint i = 0; i < 4; i++) {
		// Validate the bound.
		assert(lowerBound[i] > upperBound[i] && "[ERROR]: AABB lower bound is greater than the upper bound!");

		size[i] = upperBound[i] - lowerBound[i];
	}

	// Create the new AABB.
	BoundingBox aabb(lowerBound, upperBound);

	// No need to update if the particle is still within its fattened AABB.
	if (!alwaysReinsert && nodes[node].aabb->contains(&aabb)) return false;

	// Remove the current leaf.
	removeLeaf(node);

	// Fatten the new AABB.
	aabb.min -= (size * skinThickness);
	aabb.max += (size * skinThickness);

	// Assign the new AABB.
	nodes[node].aabb->set(&aabb);

	// Update the surface area and centroid.
	nodes[node].aabb->surfaceArea = nodes[node].aabb->computeSurfaceArea();
	nodes[node].aabb->cnt = nodes[node].aabb->computeCentre();

	// Insert a new leaf node.
	insertLeaf(node);

	return true;
}

std::vector<uint> AABBTree::query(uint _particle) {
	// Make sure that this is a valid particle.
	assert(particleMap.count(_particle) == 0 && "[ERROR]: Invalid particle index!");

	// Test overlap of particle AABB against all other particles.
	return query(_particle, nodes[particleMap.find(_particle)->second].aabb);
}

std::vector<uint> AABBTree::query(uint _particle, BoundingBox* _aabb) {
	std::vector<uint> stack;
	stack.reserve(256);
	stack.push_back(root);

	std::vector<uint> particles;

	while (stack.size() > 0) {
		uint node = stack.back();
		stack.pop_back();

		// Copy the AABB.
		BoundingBox nodeAABB(nodes[node].aabb);

		if (node == NULL_NODE) continue;


		// Test for overlap between the AABBs.
		if (_aabb->intersects(&nodeAABB)) {
			// Check that we're at a leaf node.
			if (nodes[node].isLeaf()) {
				// Can't interact with itself.
				if (nodes[node].particle != _particle) {
					particles.push_back(nodes[node].particle);
				}
			} else {
				stack.push_back(nodes[node].left);
				stack.push_back(nodes[node].right);
			}
		}
	}

	return particles;
}

std::vector<uint> AABBTree::query(BoundingBox* _aabb) {
	// Make sure the AABBTree isn't empty.
	if (particleMap.size() == 0) 
		return std::vector<uint>();

	// Test overlap of AABB against all particles.
	return query(std::numeric_limits<uint>::max(), _aabb);
}

Heerbann::BoundingBox* AABBTree::getAABB(uint _particle) {
	return nodes[particleMap[_particle]].aabb;
}

void AABBTree::insertLeaf(uint _leaf) {
	if (root == NULL_NODE) {
		root = _leaf;
		nodes[root].parent = NULL_NODE;
		return;
	}

	// Find the best sibling for the node.

	auto leafAABB = nodes[_leaf].aabb;
	uint index = root;

	while (!nodes[index].isLeaf()) {
		// Extract the children of the node.
		uint left = nodes[index].left;
		uint right = nodes[index].right;

		float surfaceArea = nodes[index].aabb->getSurfaceArea();

		BoundingBox combinedAABB;
		combinedAABB.merge(nodes[index].aabb)->merge(leafAABB);
		float combinedSurfaceArea = combinedAABB.getSurfaceArea();

		// Cost of creating a new parent for this node and the new leaf.
		float cost = 2.f * combinedSurfaceArea;

		// Minimum cost of pushing the leaf further down the AABBTree.
		float inheritanceCost = 2.f * (combinedSurfaceArea - surfaceArea);

		// Cost of descending to the left.
		float costLeft;
		if (nodes[left].isLeaf()) {
			BoundingBox aabb;
			aabb.merge(leafAABB)->merge(nodes[left].aabb);
			costLeft = aabb.getSurfaceArea() + inheritanceCost;
		} else {
			BoundingBox aabb;
			aabb.merge(leafAABB)->merge(nodes[left].aabb);
			float oldArea = nodes[left].aabb->getSurfaceArea();
			float newArea = aabb.getSurfaceArea();
			costLeft = (newArea - oldArea) + inheritanceCost;
		}

		// Cost of descending to the right.
		float costRight;
		if (nodes[right].isLeaf()) {
			BoundingBox aabb;
			aabb.merge(leafAABB)->merge(nodes[right].aabb);
			costRight = aabb.getSurfaceArea() + inheritanceCost;
		} else {
			BoundingBox aabb;
			aabb.merge(leafAABB)->merge(nodes[right].aabb);
			float oldArea = nodes[right].aabb->getSurfaceArea();
			float newArea = aabb.getSurfaceArea();
			costRight = (newArea - oldArea) + inheritanceCost;
		}

		// Descend according to the minimum cost.
		if ((cost < costLeft) && (cost < costRight)) break;

		// Descend.
		if (costLeft < costRight) index = left;
		else                      index = right;
	}

	uint sibling = index;

	// Create a new parent.
	uint oldParent = nodes[sibling].parent;
	uint newParent = allocateNode();
	nodes[newParent].parent = oldParent;
	nodes[newParent].aabb->merge(leafAABB)->merge(nodes[sibling].aabb);
	nodes[newParent].height = nodes[sibling].height + 1;

	// The sibling was not the root.
	if (oldParent != NULL_NODE) {
		if (nodes[oldParent].left == sibling) nodes[oldParent].left = newParent;
		else                                  nodes[oldParent].right = newParent;

		nodes[newParent].left = sibling;
		nodes[newParent].right = _leaf;
		nodes[sibling].parent = newParent;
		nodes[_leaf].parent = newParent;
	}
	// The sibling was the root.
	else {
		nodes[newParent].left = sibling;
		nodes[newParent].right = _leaf;
		nodes[sibling].parent = newParent;
		nodes[_leaf].parent = newParent;
		root = newParent;
	}

	// Walk back up the AABBTree fixing heights and AABBs.
	index = nodes[_leaf].parent;
	while (index != NULL_NODE) {
		index = balance(index);

		uint left = nodes[index].left;
		uint right = nodes[index].right;

		assert(left != NULL_NODE);
		assert(right != NULL_NODE);

		nodes[index].height = 1 + std::max(nodes[left].height, nodes[right].height);
		nodes[index].aabb->merge(nodes[left].aabb)->merge(nodes[right].aabb);

		index = nodes[index].parent;
	}
}

void AABBTree::removeLeaf(uint _leaf) {
	if (_leaf == root) {
		root = NULL_NODE;
		return;
	}

	uint parent = nodes[_leaf].parent;
	uint grandParent = nodes[parent].parent;
	uint sibling;

	if (nodes[parent].left == _leaf) sibling = nodes[parent].right;
	else sibling = nodes[parent].left;

	// Destroy the parent and connect the sibling to the grandparent.
	if (grandParent != NULL_NODE) {
		if (nodes[grandParent].left == parent) nodes[grandParent].left = sibling;
		else                                   nodes[grandParent].right = sibling;

		nodes[sibling].parent = grandParent;
		freeNode(parent);

		// Adjust ancestor bounds.
		uint index = grandParent;
		while (index != NULL_NODE) {
			index = balance(index);

			uint left = nodes[index].left;
			uint right = nodes[index].right;

			nodes[index].aabb->merge(nodes[left].aabb)->merge(nodes[right].aabb);
			nodes[index].height = 1 + std::max(nodes[left].height, nodes[right].height);

			index = nodes[index].parent;
		}
	} else {
		root = sibling;
		nodes[sibling].parent = NULL_NODE;
		freeNode(parent);
	}
}

uint AABBTree::balance(uint _node) {
	assert(_node != NULL_NODE);

	if (nodes[_node].isLeaf() || (nodes[_node].height < 2))
		return _node;

	uint left = nodes[_node].left;
	uint right = nodes[_node].right;

	assert(left < nodeCapacity);
	assert(right < nodeCapacity);

	int currentBalance = nodes[right].height - nodes[left].height;

	// Rotate right branch up.
	if (currentBalance > 1) {
		uint rightLeft = nodes[right].left;
		uint rightRight = nodes[right].right;

		assert(rightLeft < nodeCapacity);
		assert(rightRight < nodeCapacity);

		// Swap node and its right-hand child.
		nodes[right].left = _node;
		nodes[right].parent = nodes[_node].parent;
		nodes[_node].parent = right;

		// The node's old parent should now point to its right-hand child.
		if (nodes[right].parent != NULL_NODE) {
			if (nodes[nodes[right].parent].left == _node) nodes[nodes[right].parent].left = right;
			else {
				assert(nodes[nodes[right].parent].right == _node);
				nodes[nodes[right].parent].right = right;
			}
		} else root = right;

		// Rotate.
		if (nodes[rightLeft].height > nodes[rightRight].height) {
			nodes[right].right = rightLeft;
			nodes[_node].right = rightRight;
			nodes[rightRight].parent = _node;
			nodes[_node].aabb->merge(nodes[left].aabb)->merge(nodes[rightRight].aabb);
			nodes[right].aabb->merge(nodes[_node].aabb)->merge(nodes[rightLeft].aabb);

			nodes[_node].height = 1 + std::max(nodes[left].height, nodes[rightRight].height);
			nodes[right].height = 1 + std::max(nodes[_node].height, nodes[rightLeft].height);
		} else {
			nodes[right].right = rightRight;
			nodes[_node].right = rightLeft;
			nodes[rightLeft].parent = _node;
			nodes[_node].aabb->merge(nodes[left].aabb)->merge(nodes[rightLeft].aabb);
			nodes[right].aabb->merge(nodes[_node].aabb)->merge(nodes[rightRight].aabb);

			nodes[_node].height = 1 + std::max(nodes[left].height, nodes[rightLeft].height);
			nodes[right].height = 1 + std::max(nodes[_node].height, nodes[rightRight].height);
		}

		return right;
	}

	// Rotate left branch up.
	if (currentBalance < -1) {
		uint leftLeft = nodes[left].left;
		uint leftRight = nodes[left].right;

		assert(leftLeft < nodeCapacity);
		assert(leftRight < nodeCapacity);

		// Swap node and its left-hand child._
		nodes[left].left = _node;
		nodes[left].parent = nodes[_node].parent;
		nodes[_node].parent = left;

		// The node's old parent should now point to its left-hand child.
		if (nodes[left].parent != NULL_NODE) {
			if (nodes[nodes[left].parent].left == _node) nodes[nodes[left].parent].left = left;
			else {
				assert(nodes[nodes[left].parent].right == _node);
				nodes[nodes[left].parent].right = left;
			}
		} else root = left;

		// Rotate.
		if (nodes[leftLeft].height > nodes[leftRight].height) {
			nodes[left].right = leftLeft;
			nodes[_node].left = leftRight;
			nodes[leftRight].parent = _node;
			nodes[_node].aabb->merge(nodes[right].aabb)->merge(nodes[leftRight].aabb);
			nodes[left].aabb->merge(nodes[_node].aabb)->merge(nodes[leftLeft].aabb);

			nodes[_node].height = 1 + std::max(nodes[right].height, nodes[leftRight].height);
			nodes[left].height = 1 + std::max(nodes[_node].height, nodes[leftLeft].height);
		} else {
			nodes[left].right = leftRight;
			nodes[_node].left = leftLeft;
			nodes[leftLeft].parent = _node;
			nodes[_node].aabb->merge(nodes[right].aabb)->merge(nodes[leftLeft].aabb);
			nodes[left].aabb->merge(nodes[_node].aabb)->merge(nodes[leftRight].aabb);

			nodes[_node].height = 1 + std::max(nodes[right].height, nodes[leftLeft].height);
			nodes[left].height = 1 + std::max(nodes[_node].height, nodes[leftRight].height);
		}

		return left;
	}

	return _node;
}

uint AABBTree::computeHeight() const {
	return computeHeight(root);
}

uint AABBTree::computeHeight(uint _node) const {
	assert(_node < nodeCapacity);

	if (nodes[_node].isLeaf()) return 0;

	uint height1 = computeHeight(nodes[_node].left);
	uint height2 = computeHeight(nodes[_node].right);

	return 1 + std::max(height1, height2);
}

uint AABBTree::getHeight() const {
	if (root == NULL_NODE) return 0;
	return nodes[root].height;
}

uint AABBTree::getNodeCount() const {
	return nodeCount;
}

uint AABBTree::computeMaximumBalance() const {
	uint maxBalance = 0;
	for (uint i = 0; i < nodeCapacity; i++) {
		if (nodes[i].height <= 1)
			continue;

		assert(nodes[i].isLeaf() == false);

		uint balance = std::abs(nodes[nodes[i].left].height - nodes[nodes[i].right].height);
		maxBalance = std::max(maxBalance, balance);
	}

	return maxBalance;
}

float AABBTree::computeSurfaceAreaRatio() const {
	if (root == NULL_NODE) return 0.0;

	float rootArea = nodes[root].aabb->computeSurfaceArea();
	float totalArea = 0.0;

	for (uint i = 0; i < nodeCapacity; i++) {
		if (nodes[i].height < 0) continue;

		totalArea += nodes[i].aabb->computeSurfaceArea();
	}

	return totalArea / rootArea;
}

void AABBTree::validate() const {
#ifndef NDEBUG
	validateStructure(root);
	validateMetrics(root);

	uint freeCount = 0;
	uint freeIndex = freeList;

	while (freeIndex != NULL_NODE) {
		assert(freeIndex < nodeCapacity);
		freeIndex = nodes[freeIndex].next;
		freeCount++;
	}

	assert(getHeight() == computeHeight());
	assert((nodeCount + freeCount) == nodeCapacity);
#endif
}

void AABBTree::rebuild() {
	std::vector<uint> nodeIndices(nodeCount);
	uint count = 0;

	for (uint i = 0; i < nodeCapacity; i++) {
		// Free node.
		if (nodes[i].height < 0) continue;

		if (nodes[i].isLeaf()) {
			nodes[i].parent = NULL_NODE;
			nodeIndices[count] = i;
			count++;
		} else freeNode(i);
	}

	while (count > 1) {
		float minCost = std::numeric_limits<float>::max();
		int iMin = -1, jMin = -1;

		for (uint i = 0; i < count; i++) {
			auto aabbi = nodes[nodeIndices[i]].aabb;

			for (uint j = i + 1; j < count; j++) {
				auto aabbj = nodes[nodeIndices[j]].aabb;
				BoundingBox aabb;
				aabb.merge(aabbi)->merge(aabbj);
				float cost = aabb.getSurfaceArea();

				if (cost < minCost) {
					iMin = i;
					jMin = j;
					minCost = cost;
				}
			}
		}

		uint index1 = nodeIndices[iMin];
		uint index2 = nodeIndices[jMin];

		uint parent = allocateNode();
		nodes[parent].left = index1;
		nodes[parent].right = index2;
		nodes[parent].height = 1 + std::max(nodes[index1].height, nodes[index2].height);
		nodes[parent].aabb->merge(nodes[index1].aabb)->merge(nodes[index2].aabb);
		nodes[parent].parent = NULL_NODE;

		nodes[index1].parent = parent;
		nodes[index2].parent = parent;

		nodeIndices[jMin] = nodeIndices[count - 1];
		nodeIndices[iMin] = parent;
		count--;
	}

	root = nodeIndices[0];

	validate();
}

void AABBTree::validateStructure(uint _node) const {
	if (_node == NULL_NODE) return;

	if (_node == root) assert(nodes[_node].parent == NULL_NODE);

	uint left = nodes[_node].left;
	uint right = nodes[_node].right;

	if (nodes[_node].isLeaf()) {
		assert(left == NULL_NODE);
		assert(right == NULL_NODE);
		assert(nodes[_node].height == 0);
		return;
	}

	assert(left < nodeCapacity);
	assert(right < nodeCapacity);

	assert(nodes[left].parent == _node);
	assert(nodes[right].parent == _node);

	validateStructure(left);
	validateStructure(right);
}

void AABBTree::validateMetrics(uint _node) const {
	if (_node == NULL_NODE) return;

	uint left = nodes[_node].left;
	uint right = nodes[_node].right;

	if (nodes[_node].isLeaf()) {
		assert(left == NULL_NODE);
		assert(right == NULL_NODE);
		assert(nodes[_node].height == 0);
		return;
	}

	assert(left < nodeCapacity);
	assert(right < nodeCapacity);

	int height1 = nodes[left].height;
	int height2 = nodes[right].height;
	int height = 1 + std::max(height1, height2);
	assert(nodes[_node].height == height);

	BoundingBox aabb;
	aabb.merge(nodes[left].aabb)->merge(nodes[right].aabb);

	for (uint i = 0; i < 3; ++i) {
		assert(EQUAL(aabb.min[i], nodes[_node].aabb->min[i]));
		assert(EQUAL(aabb.max[i], nodes[_node].aabb->max[i]));
	}

	validateMetrics(left);
	validateMetrics(right);
}

Quat Heerbann::setFromAxes(const Vec4&, const Vec4&, const Vec4&) {
	return Quat();
}
