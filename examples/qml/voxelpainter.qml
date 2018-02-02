import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2

import three.quick 1.0

Window {
    id: mainWindow
    minimumWidth: 1280
    minimumHeight: 1024

    visible: true

    Component {
        id: voxelFactory
        Box {
            property alias color: voxelMaterial.color
            property alias texture: voxelMaterial.map

            material: MeshLambertMaterial {
                id: voxelMaterial
            }
        }
    }

    ThreeD {
        id: threeD
        anchors.fill: parent
        focus: true
        z: 1

        property var objectList: []

        ImageTexture {
            id: cubeTexture
            image: ":/square-outline-textured.png"
            imageFormat: Image.RGBA8888
        }
        Scene {
            id: scene
            background: "#f0f0f0"

            AmbientLight {
                id: ambientLight
                color: "#606060"
            }
            DirectionalLight {
                color: "#ffffff"
                position: "1,0.75,0.5"
            }

            Box {
                id: rollOver
                width: 50
                height: 50
                depth: 50

                material: MeshBasicMaterial {
                    color: "#ff0000"
                    opacity: 0.5
                }
            }

            GridLines {
                id: grid
                size: 1000
                divisions: 20
            }

            Plane {
                id: plane
                type: Three.BufferGeometry
                width: 1000
                height: 1000

                material: MeshBasicMaterial {visible: false}

                Component.onCompleted: {
                    rotateX(-Math.PI / 2)
                    threeD.objectList.push(this)
                }
            }

            camera: PerspectiveCamera {
                fov: 45
                aspect: threeD.width / threeD.height
                near: 1
                far: 10000

                lookAt: scene.position

                position: "500,800,1300"
            }
        }
    }
    MouseArea {
        anchors.fill: threeD
        z: 2
        hoverEnabled: true

        property var raycaster: Three.raycaster(scene.camera)

        onPositionChanged: {
            mouse.accepted = true

            var m = Qt.vector2d((mouse.x / threeD.width) * 2 - 1, -(mouse.y / threeD.height) * 2 + 1)
            raycaster.set(m);

            var intersects = raycaster.intersectObjects(threeD.objectList);
            if (intersects.length > 0) {
                var intersect = intersects[0];
                grid.snap(rollOver, intersect);

                threeD.update()
            }
        }
        onClicked: {
            mouse.accepted = true

            var m = Qt.vector2d((mouse.x / threeD.width) * 2 - 1, -(mouse.y / threeD.height) * 2 + 1)
            raycaster.set(m);

            var intersects = raycaster.intersectObjects(threeD.objectList);
            if (intersects.length > 0) {
                var intersect = intersects[ 0 ];

                // delete cube
                if ( mouse.modifiers & Qt.ShiftModifier ) {
                    if ( intersect.object !== plane ) {

                        scene.remove( intersect.object );
                        threeD.objectList.splice( threeD.objectList.indexOf( intersect.object ), 1 );
                    }

                // create cube
                } else {
                    var voxel = voxelFactory.createObject(scene, {
                        width: 50, height: 50, depth: 50, color: "#feb74c",
                        texture: cubeTexture})

                    scene.add(voxel);
                    grid.snap(voxel, intersect);

                    threeD.objectList.push( voxel );
                }
                threeD.update()
            }
        }
    }
}