//
// Created by byter on 2/16/18.
//

#ifndef THREE_PP_OCTAHEDRON_H
#define THREE_PP_OCTAHEDRON_H

#include <threepp/geometry/Polyhedron.h>

namespace three {
namespace geometry {

class DLX Octahedron : public Polyhedron
{
  Octahedron(const Octahedron &octahedron) : Polyhedron(octahedron) {}

protected:
  Octahedron(float radius, unsigned detail);

public:
  using Ptr = std::shared_ptr<Octahedron>;
  static Ptr make(float radius, unsigned detail) {
    return Ptr(new Octahedron(radius, detail));
  }

  Octahedron *cloned() const override {
    return new Octahedron(*this);
  }
};

namespace buffer {

class DLX Octahedron : public Polyhedron
{
  friend class three::geometry::Octahedron;

  Octahedron(const Octahedron &octahedron) : Polyhedron(octahedron) {}

protected:
  Octahedron(float radius, unsigned detail);

public:
  using Ptr = std::shared_ptr<Octahedron>;
  static Ptr make(float radius, unsigned detail) {
    return Ptr(new Octahedron(radius, detail));
  }

  Octahedron *cloned() const override {
    return new Octahedron(*this);
  }
};

}
}
}


#endif //THREE_PP_OCTAHEDRON_H
