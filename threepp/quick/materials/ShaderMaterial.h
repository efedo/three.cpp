//
// Created by byter on 12/29/17.
//

#ifndef THREEPPQ_SHADERMATERIAL_H
#define THREEPPQ_SHADERMATERIAL_H

#include <QVariantMap>
#include <threepp/material/ShaderMaterial.h>
#include <threepp/renderers/gl/shader/ShaderLib.h>
#include <threepp/quick/Three.h>
#include "Material.h"

namespace three {
namespace quick {

class ShaderMaterial : public Material
{
Q_OBJECT
  Q_PROPERTY(QString shaderID READ shaderID WRITE setShaderID NOTIFY shaderIDChanged)
  Q_PROPERTY(QByteArray vertexShader READ vertexShader WRITE setVertexShader NOTIFY vertexShaderChanged)
  Q_PROPERTY(QByteArray fragmentShader READ fragmentShader WRITE setFragmentShader NOTIFY fragmentShaderChanged)
  Q_PROPERTY(bool depthTest READ depthTest WRITE setDepthTest NOTIFY depthTestChanged)
  Q_PROPERTY(bool depthWrite READ depthWrite WRITE setDepthWrite NOTIFY depthWriteChanged)
  Q_PROPERTY(QVariantMap uniforms READ uniforms WRITE setUniforms NOTIFY uniformsChanged)

  QString _shaderID;
  QByteArray _vertexShader, _fragmentShader;
  bool _depthTest = true;
  bool _depthWrite = true;
  QVariantMap _uniforms;

  three::ShaderMaterial::Ptr _material;

protected:
  virtual three::ShaderMaterial::Ptr createMaterial();

  three::Material::Ptr material() const override {return _material;}

public:
  explicit ShaderMaterial(QObject *parent = nullptr)
     : Material(material::Typer(this), parent) {}
  explicit ShaderMaterial(three::ShaderMaterial::Ptr material, QObject *parent = nullptr)
     : Material(material::Typer(this), parent), _material(material) {}

  QString shaderID() const {return _shaderID;}
  QByteArray vertexShader() const {return _vertexShader;}
  QByteArray fragmentShader() const {return _fragmentShader;}
  bool depthTest() const {return _depthTest;}
  bool depthWrite() const {return _depthWrite;}
  QVariantMap &uniforms() {return _uniforms;}

  void setShaderID(const QString &shaderID) {
    if(_shaderID != shaderID) {
      _shaderID = shaderID;
      emit shaderIDChanged();
    }
  }
  void setVertexShader(const QByteArray &shader) {
    if(_vertexShader != shader) {
      _vertexShader = shader;
      emit vertexShaderChanged();
    }
  }
  void setFragmentShader(const QByteArray &shader) {
    if(_fragmentShader != shader) {
      _fragmentShader = shader;
      emit fragmentShaderChanged();
    }
  }
  void setDepthTest(bool depthTest) {
    if(_depthTest != depthTest) {
      _depthTest = depthTest;
      emit depthTestChanged();
    }
  }
  void setDepthWrite(bool depthWrite) {
    if(_depthWrite != depthWrite) {
      _depthWrite = depthWrite;
      emit depthWriteChanged();
    }
  }
  void setUniforms(const QVariantMap &uniforms) {
    if(_uniforms != uniforms) {
      _uniforms = uniforms;
      emit uniformsChanged();
    }
  }

  three::Material::Ptr getMaterial() override
  {
    if(!_material) _material = createMaterial();
    return _material;
  }

signals:
  void shaderIDChanged();
  void vertexShaderChanged();
  void fragmentShaderChanged();
  void depthTestChanged();
  void depthWriteChanged();
  void uniformsChanged();
};

}
}

#endif //THREEPPQ_SHADERMATERIAL_H
