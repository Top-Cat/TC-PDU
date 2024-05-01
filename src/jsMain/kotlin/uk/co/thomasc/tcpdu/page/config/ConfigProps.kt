package uk.co.thomasc.tcpdu.page.config

import react.Props
import uk.co.thomasc.tcpdu.page.PDUConfig

external interface ConfigProps : Props {
    var config: PDUConfig?
}
