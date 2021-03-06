//
// Created by byter on 08.08.17.
//

#include "LinearGeometry.h"
#include "BufferGeometry.h"
#include <threepp/objects/Mesh.h>
#include <threepp/objects/Line.h>

#include "impl/raycast.h"

namespace three {

using namespace math;
using namespace impl;

struct array_hash {
  std::size_t operator () (const std::array<float, 3> &a) const {
    auto h = std::hash<float>{}(a[0]);
    hash_combine(h, a[1]);
    hash_combine(h, a[2]);

    return h;
  }
};

math::Vector3 LinearGeometry::centroid(const Face3 &face) const
{
  const Vertex &vA = _vertices[ face.a ];
  const Vertex &vB = _vertices[ face.b ];
  const Vertex &vC = _vertices[ face.c ];

  return (vA + vB + vC) / 3.0f;
}

LinearGeometry::LinearGeometry(const LinearGeometry &geometry)
   : Geometry(geometry::Typer(this))
{
  _vertices = geometry._vertices;
  _normals = geometry._normals;
  _colors = geometry._colors;
  _faces = geometry._faces;
  _faceVertexUvs = geometry._faceVertexUvs;
  _morphTargets = geometry._morphTargets;
  _morphNormals = geometry._morphNormals;
  _skinWeights = geometry._skinWeights;
  _skinIndices = geometry._skinIndices;
  _lineDistances = geometry._lineDistances;

  // update flags
  _elementsNeedUpdate = geometry._elementsNeedUpdate;
  _verticesNeedUpdate = geometry._verticesNeedUpdate;
  _uvsNeedUpdate = geometry._uvsNeedUpdate;
  _normalsNeedUpdate = geometry._normalsNeedUpdate;
  _colorsNeedUpdate = geometry._colorsNeedUpdate;
  _lineDistancesNeedUpdate = geometry._lineDistancesNeedUpdate;
  _groupsNeedUpdate = geometry._groupsNeedUpdate;
}

void LinearGeometry::raycast(const Mesh &mesh,
                             const Raycaster &raycaster,
                             const std::vector<math::Ray> &rays,
                             IntersectList &intersects)
{
  std::vector<UV_Array> &faceVertexUvs = _faceVertexUvs[0];

  for (size_t f=0, fl=_faces.size(); f < fl; f++) {

    const Face3 &face = _faces[f];
    Material::Ptr faceMaterial = mesh.materialCount() > 1 ? mesh.material(face.materialIndex) : mesh.material();

    if (!faceMaterial) continue;

    Vector3 fvA(_vertices[face.a]);
    Vector3 fvB(_vertices[face.b]);
    Vector3 fvC(_vertices[face.c]);

    if (faceMaterial->morphTargets) {

      math::Vector3 vA(0, 0, 0);
      math::Vector3 vB(0, 0, 0);
      math::Vector3 vC(0, 0, 0);

      for (size_t t = 0, tl = _morphTargets.size(); t < tl; t++) {

        float influence = mesh.morphTargetInfluence(t);

        if (influence == 0) continue;

        const std::vector<math::Vector3> &targets = _morphTargets[t].vertices;

        vA += (targets[face.a] - fvA) * influence;
        vB += (targets[face.b] - fvB) * influence;
        vC += (targets[face.c] - fvC) * influence;
      }

      fvA += vA;
      fvB += vB;
      fvC += vC;
    }

    Intersection intersection;
    unsigned rayIndex = 0;
    for(const auto &ray : rays) {
      if (checkIntersection(mesh, *faceMaterial, raycaster, ray, fvA, fvB, fvC, intersection)) {

        if (faceVertexUvs.size() > f) {

          UV_Array &uvs_f = faceVertexUvs[f];
          Vector2 uvA(uvs_f[0]);
          Vector2 uvB(uvs_f[1]);
          Vector2 uvC(uvs_f[2]);

          intersection.uv = uvIntersection(intersection.point, fvA, fvB, fvC, uvA, uvB, uvC);
        }

        intersection.face = face;
        intersection.faceIndex = (unsigned)f;
        intersection.object = &const_cast<Mesh &>(mesh);
        intersects.add(rayIndex, intersection);
      }
      rayIndex++;
    }
  }
}

void LinearGeometry::raycast(const Line &line,
                             const Raycaster &raycaster,
                             const std::vector<math::Ray> &rays,
                             IntersectList &intersects)
{
  Vector3 interSegment;
  Vector3 interRay;
  unsigned step = line.steps();

  float precisionSq = raycaster.linePrecision() * raycaster.linePrecision();

  for (size_t i = 0; i < _vertices.size() - 1; i += step ) {

    unsigned rayIndex = 0;
    for(const auto &ray : rays) {
      float distSq = ray.distanceSqToSegment(_vertices[i], _vertices[i + 1], &interRay, &interSegment);

      if (distSq > precisionSq) continue;

      interRay.apply(line.matrixWorld()); //Move back to world space for distance calculation

      float distance = raycaster.origin().distanceTo(interRay);

      if (distance < raycaster.near() || distance > raycaster.far()) continue;

      Intersection &intersect = intersects.add(rayIndex);
      rayIndex++;

      intersect.distance = distance;

      // What do we want? intersection point on the ray or on the segment??
      // point: raycaster.ray.at( distance ),
      intersect.point = interSegment.apply(line.matrixWorld());
      intersect.direction = ray.direction();
      intersect.faceIndex = i;
      intersect.object = &const_cast<Line &>(line);
    }
  }
}

void LinearGeometry::computeFaceNormals(std::vector<Face3> &faces,
                                                   const std::vector<Vertex> &vertices)
{
  for (Face3 & face : faces) {
    const Vertex &vA = vertices[ face.a ];
    const Vertex &vB = vertices[ face.b ];
    const Vertex &vC = vertices[ face.c ];

    Vertex cb = vC - vB;
    Vertex ab = vA - vB;

    face.normal = cb.cross( ab ).normalize();
  }
}

void LinearGeometry::computeVertexNormals(std::vector<Face3> &faces,
                                                     const std::vector<Vertex> &vertices,
                                                     bool areaWeighted)
{
  std::vector<Vertex> verts(vertices.size());

  if ( areaWeighted ) {
    // vertex normals weighted by triangle areas
    // http://www.iquilezles.org/www/articles/normals/normals.htm
    for (const Face3 &face :  faces) {

      const Vertex &vA = vertices[ face.a ];
      const Vertex &vB = vertices[ face.b ];
      const Vertex &vC = vertices[ face.c ];

      Vertex cb = vC - vB;
      Vertex ab = vA - vB;
      cb.cross( ab );

      verts[ face.a ] += cb;
      verts[ face.b ] += cb;
      verts[ face.c ] += cb;
    }
  }
  else {
    computeFaceNormals(faces, vertices);

    for (const Face3 &face : faces) {
      verts[ face.a ] += face.normal;
      verts[ face.b ] += face.normal;
      verts[ face.c ] += face.normal;
    }
  }

  for (Vertex &vertex : verts) {
    vertex.normalize();
  }

  for (Face3 &face : faces) {
    face.vertexNormals[ 0 ] = verts[ face.a ];
    face.vertexNormals[ 1 ] = verts[ face.b ];
    face.vertexNormals[ 2 ] = verts[ face.c ];
  }
}

LinearGeometry &LinearGeometry::computeMorphNormals()
{
  //copy faces
  std::vector<Face3> copiedFaces = _faces;

  // compute face and vertex normals for each morph
  for (size_t i=0, il=_morphTargets.size(); i<il; i++) {

    // create on first access

    MorphTarget &mt = _morphTargets[i];
    MorphNormal &mn = _morphNormals[i];

    if (mn.isEmpty() ) {
      for (const Face3 &face : _faces) {
        mn.faceNormals.emplace_back();
        mn.vertexNormals.emplace_back();
      }
    }

    // compute morph normals
    computeFaceNormals(copiedFaces, mt.vertices);
    computeVertexNormals(copiedFaces, mt.vertices, true);

    // store morph normals
    for (size_t i=0, il=copiedFaces.size(); i<il; i++) {

      const Face3 &face = copiedFaces[i];
      mn.faceNormals[i] = face.normal;

      mn.vertexNormals[i][0] = face.vertexNormals[ 0 ];
      mn.vertexNormals[i][1] = face.vertexNormals[ 1 ];
      mn.vertexNormals[i][2] = face.vertexNormals[ 2 ];
    }
  }
  return *this;
}


LinearGeometry &LinearGeometry::merge(const LinearGeometry &geometry,
                                      const math::Matrix4 &matrix,
                                      unsigned materialIndexOffset)
{
  unsigned vertexOffset = (unsigned)_vertices.size();
  std::vector<Vertex> &vertices1 = _vertices;
  const std::vector<Vertex> &vertices2 = geometry._vertices;
  auto &colors1 = _colors;
  auto &colors2 = geometry._colors;

  math::Matrix3 normalMatrix = matrix.normalMatrix();

  // vertices
  for (Vertex vertex : vertices2) {
    vertex.apply( matrix );
    vertices1.push_back(vertex);
  }

  // colors
  for (const Color &color : colors2) {
    colors1.push_back(color);
  }

  // faces
  for (const Face3 &face2 : geometry._faces) {
    Face3 face(face2.a + vertexOffset, face2.b + vertexOffset, face2.c + vertexOffset, face2.normal);

    face.normal.apply( normalMatrix ).normalize();

    for (unsigned i=0; i<3; i++) {
      face.vertexNormals[i].apply( normalMatrix ).normalize();
    }

    face.materialIndex = face.materialIndex + materialIndexOffset;

    _faces.push_back(face);
  }

  // uvs
  for (unsigned i = 0, il = (unsigned)geometry._faceVertexUvs[ 0 ].capacity(); i < il; i ++ )
  {
    _faceVertexUvs[0].push_back(geometry._faceVertexUvs[0][i]);
  }
  return *this;
}

/*
 * Checks for duplicate vertices with hashmap.
 * Duplicated vertices are removed
 * and faces' vertices are updated.
 */
size_t LinearGeometry::mergeVertices()
{
  float precisionPoints = 4; // number of decimal points, e.g. 4 for epsilon of 0.0001
  float precision = std::pow( 10.0f, precisionPoints );

  std::unordered_map<std::array<float, 3>, size_t, array_hash> verticesMap;
  std::vector<Vertex> unique;
  std::unordered_map<size_t, unsigned> changes;

  for (size_t i = 0, il = _vertices.size(); i < il; i ++ ) {
    const Vertex v = _vertices[ i ];

    std::array<float, 3> key {
      (float)round(v.x() * precision),
      (float)round(v.y() * precision),
      (float)round(v.z() * precision)};

    if (verticesMap.count(key) == 0) {
      verticesMap[key] = i;
      unique.push_back( _vertices[i] );
      changes[i] = unique.size() - 1;
    }
    else {
      //Duplicate vertex found. ', i, ' could be using ', verticesMap[key]);
      changes[ i ] = changes[ verticesMap[ key ] ];
    }
  }

  // if faces are completely degenerate after merging vertices, we
  // have to remove them from the geometry.
  std::vector<unsigned> faceIndicesToRemove;

  for (size_t i = 0, il = _faces.size(); i < il; i ++ ) {

    Face3 &face = _faces[i];

    face.a = changes[ face.a ];
    face.b = changes[ face.b ];
    face.c = changes[ face.c ];

    std::array<unsigned, 3> indices {face.a, face.b, face.c};

    // if any duplicate vertices are found in a Face3
    // we have to remove the face as nothing can be saved
    for (unsigned n = 0; n < 3; n ++ ) {

      if ( indices[ n ] == indices[ ( n + 1 ) % 3 ] ) {
        faceIndicesToRemove.push_back( i );
        break;
      }
    }
  }

  for (auto i = faceIndicesToRemove.size(); i > 0; i--) {

    unsigned idx = faceIndicesToRemove[i - 1];

    _faces.erase(_faces.begin()+idx);

    for (unsigned j = 0, jl = _faceVertexUvs.size(); j < jl; j ++ ) {
      if(idx < _faceVertexUvs[j].size())
        _faceVertexUvs[j].erase(_faceVertexUvs[j].begin()+idx);
    }
  }

  // Use unique set of vertices
  auto diff = _vertices.size() - unique.size();
  _vertices = unique;
  return diff;

}

void LinearGeometry::sortFacesByMaterialIndex()
{
  // tag faces
  std::unordered_map<unsigned, unsigned> idMap;
  for(unsigned i=0; i<_faces.size(); i++) {
    idMap[_faces[i].materialIndex] = i;
  }

  // sort faces
  std::sort(_faces.begin(), _faces.end(), [](const Face3 &f1, const Face3& f2) {
    return f1.materialIndex < f2.materialIndex;
  });

  // sort uvs
  auto &uvs1 = _faceVertexUvs[ 0 ];
  auto &uvs2 = _faceVertexUvs[ 1 ];

  std::vector<std::array<UV, 3>> newUvs1, newUvs2;

  if (uvs1.size() == _faces.size()) {
    for(const Face3 &face : _faces) {
      unsigned id = idMap[face.materialIndex];
      newUvs1.push_back( uvs1[ id ] );
    }
  }
  if (uvs2.size() == _faces.size()) {
    for(const Face3 &face : _faces) {
      unsigned id = idMap[face.materialIndex];
      newUvs2.push_back( uvs2[ id ] );
    }
  }

  _faceVertexUvs[ 0 ] = newUvs1;
  _faceVertexUvs[ 1 ] = newUvs2;

}

void LinearGeometry::addFace(const uint32_t a, const uint32_t b, const uint32_t c, const uint32_t materialIndex,
             const BufferAttributeT<float>::Ptr &normals, const BufferAttributeT<float>::Ptr &uvs,
             const BufferAttributeT<float>::Ptr &uv2s,
             const std::vector<Vertex> &tempNormals, const std::vector<UV> &tempUVs,
             const std::vector<UV> &tempUVs2)
{
}

LinearGeometry &LinearGeometry::set(const BufferGeometry &geometry )
{
  std::vector<Vertex> tempNormals;
  std::vector<UV> tempUVs;
  std::vector<UV> tempUVs2;

  const BufferAttributeT<float>::Ptr &positions = geometry.position();
  const BufferAttributeT<float>::Ptr &normals = geometry.normal();
  const BufferAttributeT<float>::Ptr &colors = geometry.color();
  const BufferAttributeT<float>::Ptr &uvs = geometry.uv();
  const BufferAttributeT<float>::Ptr &uv2s = geometry.uv2();

  for(size_t i = 0, j = 0; i < positions->size(); i += 3, j += 2 ) {

    _vertices.emplace_back((*positions)[i], (*positions)[i + 1], (*positions)[i + 2]);

    if(normals) {
      tempNormals.emplace_back((*normals)[i], (*normals)[i + 1], (*normals)[i + 2]);
    }

    if(colors) {
      _colors.emplace_back((*colors)[i], (*colors)[i + 1], (*colors)[i + 2]);
    }

    if(uvs) {
      tempUVs.emplace_back((*uvs)[j], (*uvs)[j + 1]);
    }

    if(uv2s) {
      tempUVs2.emplace_back((*uv2s)[j], (*uv2s)[j + 1]);
    }
  }

  auto addFace = [&](const uint32_t a, const uint32_t b, const uint32_t c, const uint32_t materialIndex)
  {
    std::vector<Vertex> vertexNormals;
    if(normals) vertexNormals = {tempNormals[a], tempNormals[b], tempNormals[c]};

    std::vector<Color> vertexColors;
    if(colors) vertexColors = {_colors[a], _colors[b], _colors[c]};

    Face3 face(a, b, c, vertexNormals, vertexColors, materialIndex);

    _faces.push_back(face);

    if (uvs) {
      _faceVertexUvs[0].push_back({tempUVs[a], tempUVs[b], tempUVs[c]});
    }

    if (uv2s) {
      _faceVertexUvs[1].push_back({tempUVs2[a], tempUVs2[b], tempUVs2[c]});
    }
  };

  const BufferAttributeT<uint32_t>::Ptr &indices = geometry.index();

  if (geometry.groups().size() > 0 ) {

    for(const Group &group : geometry.groups()) {

      for (auto j = group.start, jl = group.start + group.count; j < jl; j += 3 ) {
        if (indices)
          addFace((*indices)[j], (*indices)[j + 1], (*indices)[j + 2], group.materialIndex);
        else
          addFace(j, j + 1, j + 2, group.materialIndex);
      }
    }
  }
  else {
    if (indices) {
      for (size_t i = 0; i < indices->size(); i += 3 ) {
        addFace( (*indices)[ i ], (*indices)[ i + 1 ], (*indices)[ i + 2 ], 0);
      }
    } else {
      for (size_t i = 0; i < positions->size() / 3; i += 3 ) {
        addFace( i, i + 1, i + 2, 0);
      }
    }
  }

  computeFaceNormals(_faces, _vertices);

  _boundingBox = geometry.boundingBox();

  _boundingSphere = geometry.boundingSphere();

  return *this;
}

}