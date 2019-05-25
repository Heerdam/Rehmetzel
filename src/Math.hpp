#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	struct Ray {
		Vec4 origin;
		Vec4 direction;

		Ray();
		Ray(const Ray&);
		Ray(const Vec4&, const Vec4&);

		Vec4 getEndPoint(float);

		Ray* operator*(Mat4);

		Ray* set(Ray*);
		Ray* set(const Vec4&, const Vec4&);
		Ray* set(float, float, float, float, float, float);
	};

	struct Plane {
		enum PlaneSide {
			OnPlane, Back, Front
		};

		Vec4 normal;
		float d; //The distance to the origin

		Plane();
		Plane(const Vec4&, float);
		Plane(const Plane&);
		Plane(const Vec4&, const Vec4&);
		Plane(const Vec4&, const Vec4&, const Vec4&);

		Plane* set(const Plane&);
		Plane* set(const Vec4&, float);
		Plane* set(const Vec4&, const Vec4&, const Vec4&);
		Plane* set(float, float, float, float);

		float distance(const Vec4&);

		PlaneSide testPoint(const Vec4&);
		PlaneSide testPoint(float, float, float);

		bool isFrontFacing(const Vec4&);

	};

	struct BoundingBox {
		Vec4 min, max;
		Vec4 cnt, dim;
		float surfaceArea;

		BoundingBox();
		BoundingBox(BoundingBox*);
		BoundingBox(const Vec4&, const Vec4&);

		float computeSurfaceArea() const;
		float getSurfaceArea() const;

		Vec4 computeCentre();

		BoundingBox* set(BoundingBox*);
		BoundingBox* set(const Vec4&, const Vec4&);
		BoundingBox* set(const std::vector<Vec4>&);

		BoundingBox* inf();
		BoundingBox* clr();
		bool isValid();

		BoundingBox* merge(BoundingBox*);
		BoundingBox* ext(BoundingBox*);
		BoundingBox* ext(const Vec4&, const Vec4&);
		BoundingBox* ext(const Vec4&, float);
		BoundingBox* ext(BoundingBox*, Mat4);
		BoundingBox* ext(const sf::FloatRect&);
		BoundingBox* ext(const Vec4&);
		

		BoundingBox* operator*= (Mat4);

		bool contains(BoundingBox*);
		bool contains(const Vec4&);
		bool contains(float, float, float);

		bool intersects(BoundingBox*);
	};

	struct Frustum {

	private:

		std::vector<Plane*> planes;
		BoundingBox* bounds = nullptr;
		std::vector<Vec4> clipPoints;

	public:

		enum FrustumPlane {
			Right, Left, Bottom, Top, Far, Near
		};

		Frustum();

		void update(Mat4);

		bool pointInFrustum(const Vec4&);
		bool pointInFrustum(float, float, float);

		bool sphereInFrustum(const Vec4&, float);
		bool sphereInFrustum(float, float, float, float);

		bool sphereInFrustumWithoutNearFar(const Vec4&, float);
		bool sphereInFrustumWithoutNearFar(float, float, float, float);

		bool boundsInFrustum(BoundingBox*);
		bool boundsInFrustum(const Vec4&, const Vec4&);
		bool boundsInFrustum(float, float, float, float, float, float);

		BoundingBox* toAABB(Camera*);
		std::vector<Vec4> getPoints(Camera*);
	};

	Quat setFromAxes(const Vec4&, const Vec4&, const Vec4&);

#define NULL_NODE 0xffffffffu

	/*! \brief A AABBTreeNode of the AABB tree.

		Each AABBTreeNode of the tree contains an AABB object which corresponds to a
		particle, or a group of particles, in the simulation box. The AABB
		objects of individual particles are "fattened" before they are stored
		to avoid having to continually update and rebalance the tree when
		displacements are small.

		AABBTreeNodes are aware of their position within in the tree. The isLeaf member
		function allows the tree to query whether the AABBTreeNode is a leaf, i.e. to
		determine whether it holds a single particle.

		Adapted from: https://github.com/lohedges/aabbcc
	 */
	struct AABBTreeNode {

		AABBTreeNode();

		// The fattened axis-aligned bounding box.
		BoundingBox* aabb;

		// Index of the parent AABBTreeNode.
		uint parent;

		// Index of the next AABBTreeNode.
		uint next;

		// Index of the left-hand child.
		uint left;

		// Index of the right-hand child.
		uint right;

		// Height of the AABBTreeNode. This is 0 for a leaf and -1 for a free AABBTreeNode.
		int height;

		// The index of the particle that the AABBTreeNode contains (leaf AABBTreeNodes only).
		uint particle;

		//! Test whether the AABBTreeNode is a leaf.
		/*! \return
				Whether the AABBTreeNode is a leaf AABBTreeNode.
		 */
		bool isLeaf() const;
	};

	/*! \brief The dynamic AABB tree.

		The dynamic AABB tree is a hierarchical data structure that can be used
		to efficiently query overlaps between objects of arbitrary shape and
		size that lie inside of a simulation box. Support is provided for
		periodic and non-periodic boxes, as well as boxes with partial
		periodicity, e.g. periodic along specific axes.
	 */
	class AABBTree {
	public:
		//! Constructor (non-periodic).
		/*!
			\param skinThickness_
				The skin thickness for fattened AABBs, as a fraction
				of the AABB base length.

			\param nParticles
				The number of particles (for fixed particle number systems).

		 */
		AABBTree(float, uint);

		//! Constructor (custom periodicity).
		/*! \param dimension_
				The dimensionality of the system.

			\param skinThickness_
				The skin thickness for fattened AABBs, as a fraction
				of the AABB base length.

			\param periodicity_
				Whether the system is periodic in each dimension.

			\param boxSize_
				The size of the simulation box in each dimension.

			\param nParticles
				The number of particles (for fixed particle number systems).

			\param touchIsOverlap
				Does touching count as overlapping in query operations?
		 */
		AABBTree();

		//! Insert a particle into the tree (point particle).
		/*! \param index
				The index of the particle.

			\param position
				The position vector of the particle.

			\param radius
				The radius of the particle.
		 */
		void insertParticle(uint, Vec4&, float);

		//! Insert a particle into the tree (arbitrary shape with bounding box).
		/*! \param index
				The index of the particle.

			\param lowerBound
				The lower bound in each dimension.

			\param upperBound
				The upper bound in each dimension.
		 */
		void insertParticle(uint, Vec4&, Vec4&);
		void insertParticle(uint, BoundingBox*);

		// Return the number of particles in the tree.
		uint nParticles();

		//! Remove a particle from the tree.
		/*! \param particle
				The particle index (particleMap will be used to map the AABBTreeNode).
		 */
		void removeParticle(uint);

		// Remove all particles from the tree.
		void removeAll();

		//! Update the tree if a particle moves outside its fattened AABB.
		/*! \param particle
				The particle index (particleMap will be used to map the AABBTreeNode).

			\param position
				The position vector of the particle.

			\param radius
				The radius of the particle.

			\param alwaysReinsert
				Always reinsert the particle, even if it's within its old AABB (default:false)

			\return
				Whether the particle was reinserted.
		 */
		bool updateParticle(uint, Vec4, float, bool alwaysReinsert = false);

		//! Update the tree if a particle moves outside its fattened AABB.
		/*! \param particle
				The particle index (particleMap will be used to map the AABBTreeNode).

			\param lowerBound
				The lower bound in each dimension.

			\param upperBound
				The upper bound in each dimension.

			\param alwaysReinsert
				Always reinsert the particle, even if it's within its old AABB (default: false)
		 */
		bool updateParticle(uint, Vec4&, Vec4&, bool alwaysReinsert = false);

		//! Query the tree to find candidate interactions for a particle.
		/*! \param particle
				The particle index.

			\return particles
				A vector of particle indices.
		 */
		std::vector<uint> query(uint);

		//! Query the tree to find candidate interactions for an AABB.
		/*! \param particle
				The particle index.

			\param aabb
				The AABB.

			\return particles
				A vector of particle indices.
		 */
		std::vector<uint> query(uint, BoundingBox*);

		//! Query the tree to find candidate interactions for an AABB.
		/*! \param aabb
				The AABB.

			\return particles
				A vector of particle indices.
		 */
		std::vector<uint> query(BoundingBox*);

		//! Get a particle AABB.
		/*! \param particle
				The particle index.
		 */
		BoundingBox* getAABB(uint);

		//! Get the height of the tree.
		/*! \return
				The height of the binary tree.
		 */
		uint getHeight() const;

		//! Get the number of AABBTreeNodes in the tree.
		/*! \return
				The number of AABBTreeNodes in the tree.
		 */
		uint getNodeCount() const;

		//! Compute the maximum balancance of the tree.
		/*! \return
				The maximum difference between the height of two
				children of a AABBTreeNode.
		 */
		uint computeMaximumBalance() const;

		//! Compute the surface area ratio of the tree.
		/*! \return
				The ratio of the sum of the AABBTreeNode surface area to the surface
				area of the root AABBTreeNode.
		 */
		float computeSurfaceAreaRatio() const;

		// Validate the tree.
		void validate() const;

		// Rebuild an optimal tree.
		void rebuild();

	private:
		// The index of the root AABBTreeNode.
		uint root;

		// The dynamic tree.
		std::vector<AABBTreeNode> nodes;

		// The current number of AABBTreeNodes in the tree.
		uint nodeCount;

		// The current AABBTreeNode capacity.
		uint nodeCapacity;

		// The position of AABBTreeNode at the top of the free list.
		uint freeList;

		// The skin thickness of the fattened AABBs, as a fraction of the AABB base length.
		float skinThickness;

		// The size of the system in each dimension.
		//Vec4 boxSize;

		// The position of the negative minimum image.
		Vec4 negMinImage;

		// The position of the positive minimum image.
		Vec4 posMinImage;

		// A map between particle and AABBTreeNode indices.
		std::map<uint, uint> particleMap;

		//! Allocate a new AABBTreeNode.
		/*! \return
				The index of the allocated AABBTreeNode.
		 */
		uint allocateNode();

		//! Free an existing AABBTreeNode.
		/*! \param AABBTreeNode
				The index of the AABBTreeNode to be freed.
		 */
		void freeNode(uint);

		//! Insert a leaf into the tree.
		/*! \param leaf
				The index of the leaf AABBTreeNode.
		 */
		void insertLeaf(uint);

		//! Remove a leaf from the tree.
		/*! \param leaf
				The index of the leaf AABBTreeNode.
		 */
		void removeLeaf(uint);

		//! Balance the tree.
		/*! \param AABBTreeNode
				The index of the AABBTreeNode.
		 */
		uint balance(uint);

		//! Compute the height of the tree.
		/*! \return
				The height of the entire tree.
		 */
		uint computeHeight() const;

		//! Compute the height of a sub-tree.
		/*! \param AABBTreeNode
				The index of the root AABBTreeNode.

			\return
				The height of the sub-tree.
		 */
		uint computeHeight(uint) const;

		//! Assert that the sub-tree has a valid structure.
		/*! \param AABBTreeNode
				The index of the root AABBTreeNode.
		 */
		void validateStructure(uint) const;

		//! Assert that the sub-tree has valid metrics.
		/*! \param AABBTreeNode
				The index of the root AABBTreeNode.
		 */
		void validateMetrics(uint) const;

	};
}