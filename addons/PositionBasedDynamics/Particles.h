#ifndef ROXLU_PARTICLESH
#define ROXLU_PARTICLESH

#include "Spring.h"
#include "Particle.h"
#include "BendingConstraint.h"
#include "Vec3.h"
#include <vector>
#include <map>

using namespace roxlu;
using std::vector;
using std::map;

namespace pbd {

class Particles {
public:
	Particles();
	~Particles();
	
	BendingConstraint* createBendingConstraint(Particle* a, Particle* b, Particle* c, float max);
	BendingConstraint* addBendingConstraint(BendingConstraint* bc);
	
	Particle* createParticle(const float& x, const float& y, const float& z = 0.0f);
	Particle* createParticle(const Vec3& pos, float mass = 1.0f);
	Particle* addParticle(const float& x, const float& y, const float& z = 0.0f);
	Particle* addParticle(Particle* p);
	Particle* addParticle(const Vec3& pos, float mass = 1.0f);
	
	Spring* createSpring(Particle* a, Particle* b);
	Spring* addSpring(Particle* a, Particle* b);
	Spring* addSpring(Spring* spring);
	
	bool springExists(Particle* a, Particle* b);
	void removeSpring(Particle* a, Particle* b);
	void removeAllSprings();
	void removeDeadParticles();
	void addForce(const float& x, const float& y, const float& z);
	void addForce(const Vec3& f);
	void update(const float& dt);
	void draw();
	void repel(float f = 0.9);

	Particle* operator[](const unsigned int& dx);
	size_t size();
	

	vector<Spring*> springs;
	vector<Particle*> particles;
	vector<BendingConstraint*> bending_constraints;
	
	
	map<Particle*, Particle*> springs_ab;
	map<Particle*, Particle*> springs_ba;	

	float drag;
};



inline Particle* Particles::operator[](const unsigned int& dx) {
	return particles[dx];
}

inline size_t Particles::size() {
	return particles.size();
}

inline Particle* Particles::addParticle(const Vec3& pos, float mass) {
	return addParticle(createParticle(pos, mass));
}

inline Particle* Particles::addParticle(const float& x, const float& y, const float& z) {
	return addParticle(createParticle(x,y,z));
}

inline Particle* Particles::createParticle(const float& x, const float& y, const float& z) {
	return createParticle(Vec3(x,y,z));
}

inline Spring* Particles::addSpring(Particle* a, Particle* b) {
	return addSpring(createSpring(a,b));
}

} // pbd

#endif