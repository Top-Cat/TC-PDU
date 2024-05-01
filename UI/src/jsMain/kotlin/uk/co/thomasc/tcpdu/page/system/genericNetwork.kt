package uk.co.thomasc.tcpdu.page.system

import react.Props
import react.RBuilder
import react.dom.br
import react.dom.p
import react.fc
import uk.co.thomasc.tcpdu.page.INetworkState

fun RBuilder.hideIfNull(title: String, v: Any?, suffix: String = "") {
    v?.let { actual ->
        +"$title: $actual$suffix"
        br {}
    }
}

external interface NetworkStateProps : Props {
    var state: INetworkState
}

val genericStatus = fc<NetworkStateProps> { props ->
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
