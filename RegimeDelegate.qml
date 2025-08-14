import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
Button {
    y: 5
    height: 35
    property var regime: model.regime
    signal regimeClicked(variant regime)
    Material.roundedScale: Material.ExtraSmallScale
    text: regime.name
    font.pointSize: 8
    onClicked: {
        regimeClicked(regime)
    }
}
