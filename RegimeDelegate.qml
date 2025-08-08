import QtQuick
import QtQuick.Controls

Button {
    property var regime: model.regime
    signal regimeClicked(variant regime)

    text: regime.name
    onClicked: {
        regimeClicked(regime)
    }
}
