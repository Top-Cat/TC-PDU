package uk.co.thomasc.tcpdu.page.system

import react.dom.div
import react.dom.i
import react.fc

val ethStatus = fc<SystemProps> { props ->
    props.system?.let { system ->
        div("card border-primary") {
            div("card-header") {
                +"Ethernet"
                div("float-end") {
                    if (system.eth.connected) {
                        i("fas fa-circle text-success") {}
                    } else {
                        i("far fa-circle text-danger") {}
                    }
                }
            }
            div("card-body") {
                genericStatus {
                    attrs.state = system.eth
                }
                hideIfNull("Direction", system.eth.fullDuplex?.let { d -> if (d) "Full" else "Half" }, " duplex")
                hideIfNull("Speed", system.eth.linkSpeed, " Mbps")
            }
        }
    }
}
