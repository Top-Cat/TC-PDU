package uk.co.thomasc.tcpdu.page.config

import external.Axios
import external.generateConfig
import kotlinx.html.ButtonType
import kotlinx.html.InputType
import kotlinx.html.id
import kotlinx.html.js.onChangeFunction
import kotlinx.html.js.onClickFunction
import org.w3c.dom.HTMLInputElement
import react.dom.button
import react.dom.defaultValue
import react.dom.div
import react.dom.input
import react.dom.label
import react.dom.table
import react.dom.tbody
import react.dom.td
import react.dom.th
import react.dom.thead
import react.dom.tr
import react.fc
import react.router.useNavigate
import react.useRef
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.errors
import uk.co.thomasc.tcpdu.page.LogConfig
import uk.co.thomasc.tcpdu.page.LogType
import uk.co.thomasc.tcpdu.page.handleForbidden
import uk.co.thomasc.tcpdu.success

val logsConfig = fc<ConfigProps> { props ->
    val history = useNavigate()
    val (success, setSuccess) = useState<Boolean?>(null)
    val (serialMask, setSerialMask) = useState(props.config?.log?.serialMask ?: 0uL)
    val (syslogMask, setSyslogMask) = useState(props.config?.log?.syslogMask ?: 0uL)
    val (emailMask, setEmailMask) = useState(props.config?.log?.emailMask ?: 0uL)

    val daysRef = useRef<HTMLInputElement>()

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
            if (success == true) {
                success { +"Config saved" }
            } else if (success == false) {
                errors { +"Unknown error" }
            }

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
                    LogType.entries.filter { it.enc >= 0 }.forEach { logType ->
                        val idx = logType.enc.toInt()
                        tr {
                            td {
                                +logType.human
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

            div("form-group") {
                label("form-label") {
                    attrs.htmlFor = "logs-retention"
                    +"Log retention"
                }
                input(InputType.text, classes = "form-control") {
                    attrs.placeholder = "3"
                    attrs.id = "logs-retention"
                    attrs.defaultValue = props.config?.log?.days?.toString() ?: ""
                    ref = daysRef
                }
            }

            button(type = ButtonType.submit, classes = "btn btn-primary") {
                attrs.onClickFunction = { ev ->
                    ev.preventDefault()
                    setSuccess(null)

                    val newDays = daysRef.current?.value?.toIntOrNull() ?: 3
                    val newConfig = LogConfig(serialMask, syslogMask, emailMask, days = newDays)
                    Axios.post<String>("$apiRoot/config/log", newConfig, generateConfig<LogConfig, String>())
                        .then { setSuccess(true) }
                        .handleForbidden(history)
                        .catch {
                            setSuccess(false)
                        }
                }
                +"Save"
            }
        }
    }
}
