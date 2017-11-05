//
// Created by byter on 29.07.17.
//

#ifndef THREEQT_MESHBASICMATERIAL
#define THREEQT_MESHBASICMATERIAL

#include "Material.h"
#include "core/Color.h"

namespace three {

/**
 * @author mrdoob / http://mrdoob.com/
 * @author alteredq / http://alteredqualia.com/
 *
 * parameters = {
 *  color: <hex>,
 *  opacity: <float>,
 *  map: new THREE.Texture( <Image> ),
 *
 *  lightMap: new THREE.Texture( <Image> ),
 *  lightMapIntensity: <float>
 *
 *  aoMap: new THREE.Texture( <Image> ),
 *  aoMapIntensity: <float>
 *
 *  specularMap: new THREE.Texture( <Image> ),
 *
 *  alphaMap: new THREE.Texture( <Image> ),
 *
 *  envMap: new THREE.TextureCube( [posx, negx, posy, negy, posz, negz] ),
 *  combine: THREE.Multiply,
 *  reflectivity: <float>,
 *  refractionRatio: <float>,
 *
 *  depthTest: <bool>,
 *  depthWrite: <bool>,
 *
 *  wireframe: <boolean>,
 *  wireframeLinewidth: <float>,
 *
 *  skinning: <bool>,
 *  morphTargets: <bool>
 * }
 */
struct MeshBasicMaterial : public Material
{
  Color color {0xffffff}; // emissive

  Texture::Ptr map;

  float lightMapIntensity = 1.0;

  Texture::Ptr aoMap;
  float aoMapIntensity = 1.0;

  Texture::Ptr specularMap;

  Texture::Ptr alphaMap;

  CombineOperation combine = CombineOperation::Multiply;
  float reflectivity = 1;
  float refractionRatio = 0.98;

private:
  MeshBasicMaterial(bool morphTargets, bool skinning)
  {
    this->morphTargets = morphTargets;
    this->skinning = skinning;
  }

  MeshBasicMaterial() : Material(material::ResolverT<MeshBasicMaterial>::make(*this))
  {}

public:
  using Ptr = std::shared_ptr<MeshBasicMaterial>;

  static Ptr make(bool morphTargets, bool skinning) {
    return std::shared_ptr<MeshBasicMaterial>(new MeshBasicMaterial(morphTargets, skinning));
  }

  static Ptr make() {
    return std::shared_ptr<MeshBasicMaterial>(new MeshBasicMaterial());
  }
};

}


#endif //THREEQT_MESHBASICMATERIAL
