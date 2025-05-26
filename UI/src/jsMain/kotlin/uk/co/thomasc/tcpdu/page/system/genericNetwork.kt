package uk.co.thomasc.tcpdu.page.system

import react.ChildrenBuilder
import react.Props
import react.dom.html.ReactHTML.br
import react.dom.html.ReactHTML.p
import uk.co.thomasc.tcpdu.fcmemo
import uk.co.thomasc.tcpdu.page.INetworkState

fun ChildrenBuilder.hideIfNull(title: String, v: Any?, suffix: String = "") {
    v?.let { actual ->
        +"$title: $actual$suffix"
        br {}
    }
}

external interface NetworkStateProps : Props {
    var state: INetworkState
}

val genericStatus = fcmemo<NetworkStateProps>("Generic Status") { props ->
    val networkState = props.state

    p { +"Status: ${if (networkState.connected) "Connected" else "Disconnected"}" }

    p {
        hideIfNull("Mac", networkState.mac)

        hideIfNull("IP", networkState.ip)
        hideIfNull("Subnet", networkState.subnet)
        hideIfNull("Gateway", networkState.gateway)
        hideIfNull("DNS", networkState.dns)
    }
}
