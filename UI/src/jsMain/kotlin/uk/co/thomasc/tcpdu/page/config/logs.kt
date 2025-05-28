package uk.co.thomasc.tcpdu.page.config

import external.Axios
import external.generateConfig
import react.dom.html.ReactHTML.button
import react.dom.html.ReactHTML.div
import react.dom.html.ReactHTML.input
import react.dom.html.ReactHTML.label
import react.dom.html.ReactHTML.table
import react.dom.html.ReactHTML.tbody
import react.dom.html.ReactHTML.td
import react.dom.html.ReactHTML.th
import react.dom.html.ReactHTML.thead
import react.dom.html.ReactHTML.tr
import react.router.useNavigate
import react.useRef
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.errors
import uk.co.thomasc.tcpdu.fcmemo
import uk.co.thomasc.tcpdu.page.LogConfig
import uk.co.thomasc.tcpdu.page.LogType
import uk.co.thomasc.tcpdu.page.handleForbidden
import uk.co.thomasc.tcpdu.success
import web.cssom.ClassName
import web.html.ButtonType
import web.html.HTMLInputElement
import web.html.InputType

val logsConfig = fcmemo<ConfigProps>("Logs Config") { props ->
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

    div {
        className = ClassName("card border-primary")
        div {
            className = ClassName("card-header")
            +"Logs"
        }
        div {
            className = ClassName("card-body")
            if (success == true) {
                success { +"Config saved" }
            } else if (success == false) {
                errors { +"Unknown error" }
            }

            table {
                className = ClassName("table table-sm table-striped")
                thead {
                    tr {
                        th {
                            className = ClassName("col-6")
                            +"Type"
                        }
                        th {
                            className = ClassName("col-2")
                            +"Serial"
                        }
                        th {
                            className = ClassName("col-2")
                            +"Syslog"
                        }
                        th {
                            className = ClassName("col-2")
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
                                input {
                                    type = InputType.checkbox
                                    className = ClassName("form-check-input")
                                    defaultChecked = serialMask.shr(idx).and(1uL) == 1uL
                                    onChange = { ev ->
                                        setSerialMask(setBit(serialMask, idx, ev.target.checked))
                                    }
                                }
                            }
                            td {
                                input {
                                    type = InputType.checkbox
                                    className = ClassName("form-check-input")
                                    defaultChecked = syslogMask.shr(idx).and(1uL) == 1uL
                                    onChange = { ev ->
                                        setSyslogMask(setBit(syslogMask, idx, ev.target.checked))
                                    }
                                }
                            }
                            td {
                                input {
                                    type = InputType.checkbox
                                    className = ClassName("form-check-input")
                                    defaultChecked = emailMask.shr(idx).and(1uL) == 1uL
                                    onChange = { ev ->
                                        setEmailMask(setBit(emailMask, idx, ev.target.checked))
                                    }
                                }
                            }
                        }
                    }
                }
            }

            div {
                label {
                    className = ClassName("form-label")
                    htmlFor = "logs-retention"
                    +"Log retention"
                }
                input {
                    type = InputType.text
                    className = ClassName("form-control")
                    placeholder = "3"
                    id = "logs-retention"
                    defaultValue = props.config?.log?.days?.toString() ?: ""
                    ref = daysRef
                }
            }

            button {
                type = ButtonType.submit
                className = ClassName("btn btn-primary")
                onClick = { ev ->
                    ev.preventDefault()
                    setSuccess(null)

                    val newDays = daysRef.current?.value?.toIntOrNull() ?: 3
                    val newConfig = LogConfig(serialMask, syslogMask, emailMask, days = newDays)
                    Axios.post<String>("$apiRoot/config/log", newConfig, generateConfig<LogConfig, String>())
                        .then {
                            setSuccess(true)
                            props.config?.let {
                                props.updateCallback(it.copy(log = newConfig.copy(smtp = it.log.smtp)))
                            }
                        }
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
