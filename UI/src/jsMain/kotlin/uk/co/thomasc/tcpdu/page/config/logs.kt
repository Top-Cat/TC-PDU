package uk.co.thomasc.tcpdu.page.config

import kotlinx.html.InputType
import kotlinx.html.js.onChangeFunction
import org.w3c.dom.HTMLInputElement
import react.dom.div
import react.dom.input
import react.dom.table
import react.dom.tbody
import react.dom.td
import react.dom.th
import react.dom.thead
import react.dom.tr
import react.fc
import react.useState
import uk.co.thomasc.tcpdu.page.LogType

val logsConfig = fc<ConfigProps> { props ->
    val (serialMask, setSerialMask) = useState(props.config?.log?.serialMask ?: 0uL)
    val (syslogMask, setSyslogMask) = useState(props.config?.log?.syslogMask ?: 0uL)
    val (emailMask, setEmailMask) = useState(props.config?.log?.emailMask ?: 0uL)

    fun setBit(input: ULong, idx: Int, new: Boolean) =
        if (new) {
            input.or(1uL shl idx)
        } else {
            input.and((1uL shl idx).inv())
        }

    div("card border-primary") {
        div("card-header") {
            +"Logs"
        }
        div("card-body") {
            table("table table-sm table-striped") {
                thead {
                    tr {
                        th(classes = "col-6") {
                            +"Type"
                        }
                        th(classes = "col-2") {
                            +"Serial"
                        }
                        th(classes = "col-2") {
                            +"Syslog"
                        }
                        th(classes = "col-2") {
                            +"Email"
                        }
                    }
                }
                tbody {
                    LogType.entries.forEachIndexed { idx, logType ->
                        tr {
                            td {
                                +logType.name
                            }
                            td {
                                input(InputType.checkBox, classes = "form-check-input") {
                                    attrs.defaultChecked = serialMask.shr(idx).and(1uL) == 1uL
                                    attrs.onChangeFunction = { ev ->
                                        setSerialMask(setBit(serialMask, idx, (ev.target as HTMLInputElement).checked))
                                    }
                                }
                            }
                            td {
                                input(InputType.checkBox, classes = "form-check-input") {
                                    attrs.defaultChecked = syslogMask.shr(idx).and(1uL) == 1uL
                                    attrs.onChangeFunction = { ev ->
                                        setSyslogMask(setBit(syslogMask, idx, (ev.target as HTMLInputElement).checked))
                                    }
                                }
                            }
                            td {
                                input(InputType.checkBox, classes = "form-check-input") {
                                    attrs.defaultChecked = emailMask.shr(idx).and(1uL) == 1uL
                                    attrs.onChangeFunction = { ev ->
                                        setEmailMask(setBit(emailMask, idx, (ev.target as HTMLInputElement).checked))
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
