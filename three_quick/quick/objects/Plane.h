//
// Created by byter on 12/14/17.
//

#ifndef THREEPP_QUICK_PLANE_H
#define THREEPP_QUICK_PLANE_H

#include "quick/scene/Scene.h"
#include <geometry/Plane.h>
#include <material/MeshBasicMaterial.h>
#include <material/MeshLambertMaterial.h>
#include <objects/Mesh.h>
#include "MeshCreator.h"

namespace three {
namespace quick {

class Plane : public ThreeQObject
{
  Q_OBJECT
  Q_PROPERTY(unsigned width READ width WRITE setWidth NOTIFY widthChanged)
  Q_PROPERTY(unsigned height READ height WRITE setHeight NOTIFY heightChanged)

  unsigned _width=1, _height=1;

  three::Mesh::Ptr _plane;

  MeshCreatorG<geometry::Plane> _creator {"plane"};

protected:
  three::Object3D::Ptr _create(Scene *scene) override
  {
    _creator.set(geometry::Plane::make(_width, _height, 1, 1));
    material()->identify(_creator);

    three::Mesh::Ptr mesh = _creator.mesh;

    return mesh;
  }

  void updateMaterial() override {
    material()->identify(_creator);
  }

public:
  Plane(QObject *parent = nullptr) : ThreeQObject(parent) {}

  unsigned width() const {return _width;}
  unsigned height() const {return _height;}

  void setWidth(unsigned width) {
    if(_width != width) {
      _width = width;
      emit widthChanged();
    }
  }
  void setHeight(unsigned height) {
    if(_height != height) {
      _height = height;
      emit heightChanged();
    }
  }

signals:
  void widthChanged();
  void heightChanged();
};

}
}

#endif //THREEPP_QUICK_PLANE_H