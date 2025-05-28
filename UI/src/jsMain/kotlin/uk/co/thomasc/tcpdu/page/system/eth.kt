package uk.co.thomasc.tcpdu.page.system

import react.dom.html.ReactHTML.div
import react.dom.html.ReactHTML.i
import uk.co.thomasc.tcpdu.fcmemo
import web.cssom.ClassName

val ethStatus = fcmemo<SystemProps>("ETH Status") { props ->
    props.system?.let { system ->
        div {
            className = ClassName("card border-primary")
            div {
                className = ClassName("card-header")
                +"Ethernet"
                div {
                    className = ClassName("float-end")
                    if (system.eth.connected) {
                        i { className = ClassName("fas fa-circle text-success") }
                    } else {
                        i { className = ClassName("fas fa-circle text-danger") }
                    }
                }
            }
            div {
                className = ClassName("card-body")
                genericStatus {
                    state = system.eth
                }
                hideIfNull("Direction", system.eth.fullDuplex?.let { d -> if (d) "Full" else "Half" }, " duplex")
                hideIfNull("Speed", system.eth.linkSpeed, " Mbps")
            }
        }
    }
}
