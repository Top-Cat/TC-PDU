package uk.co.thomasc.tcpdu.page.system

import react.Props
import uk.co.thomasc.tcpdu.page.PDUSystem

external interface SystemProps : Props {
    var system: PDUSystem?
}
