package uk.co.thomasc.tcpdu.page.system

import react.dom.div
import react.dom.i
import react.fc

val wifiStatus = fc<SystemProps> { props ->
    props.system?.let { system ->
        div("card border-primary") {
            div("card-header") {
                +"Wifi"
                div("float-end") {
                    if (system.wifi.connected) {
                        i("fas fa-circle text-success") {}
                    } else {
                        i("far fa-circle text-danger") {}
                    }
                }
            }
            div("card-body") {
                genericStatus {
                    attrs.state = system.wifi
                }
                hideIfNull("RSSI", system.wifi.rssi, "dBm")
            }
        }
    }
}
