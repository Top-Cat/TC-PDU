package uk.co.thomasc.tcpdu.page.system

import react.dom.html.ReactHTML.div
import react.dom.html.ReactHTML.i
import uk.co.thomasc.tcpdu.fcmemo
import web.cssom.ClassName

val wifiStatus = fcmemo<SystemProps>("Wifi Status") { props ->
    props.system?.let { system ->
        div {
            className = ClassName("card border-primary")
            div {
                className = ClassName("card-header")
                +"Wifi"
                div {
                    className = ClassName("float-end")
                    if (system.wifi.connected) {
                        i { className = ClassName("fas fa-circle text-success") }
                    } else {
                        i { className = ClassName("fas fa-circle text-danger") }
                    }
                }
            }
            div {
                className = ClassName("card-body")
                genericStatus {
                    state = system.wifi
                }
                hideIfNull("RSSI", system.wifi.rssi, "dBm")
            }
        }
    }
}
