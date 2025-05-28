package uk.co.thomasc.tcpdu.page.config

import external.Axios
import external.generateConfig
import react.dom.html.ReactHTML.button
import react.dom.html.ReactHTML.div
import react.dom.html.ReactHTML.form
import react.dom.html.ReactHTML.input
import react.dom.html.ReactHTML.label
import react.router.useNavigate
import react.useRef
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.errors
import uk.co.thomasc.tcpdu.fcmemo
import uk.co.thomasc.tcpdu.page.SyslogConfig
import uk.co.thomasc.tcpdu.page.handleForbidden
import uk.co.thomasc.tcpdu.success
import web.cssom.ClassName
import web.html.ButtonType
import web.html.HTMLInputElement
import web.html.InputType

val syslogConfig = fcmemo<ConfigProps>("Syslog Config") { props ->
    val history = useNavigate()
    val (success, setSuccess) = useState<Boolean?>(null)

    val hostRef = useRef<HTMLInputElement>()
    val portRef = useRef<HTMLInputElement>()

    props.config?.let { config ->
        div {
            className = ClassName("card border-primary")
            div {
                className = ClassName("card-header")
                +"Syslog"
            }
            div {
                className = ClassName("card-body")
                if (success == true) {
                    success { +"Config saved" }
                } else if (success == false) {
                    errors { +"Unknown error" }
                }

                form {
                    div {
                        className = ClassName("row")
                        div {
                            className = ClassName("col-md-9")
                            label {
                                className = ClassName("form-label")
                                htmlFor = "syslog-host"
                                +"Host"
                            }
                            input {
                                type = InputType.text
                                className = ClassName("form-control")
                                key = "syslog-host"
                                placeholder = "syslog.example.com"
                                id = "syslog-host"
                                defaultValue = config.syslog.host ?: ""
                                ref = hostRef
                            }
                        }

                        div {
                            className = ClassName("col-md-3")
                            label {
                                className = ClassName("form-label")
                                htmlFor = "syslog-port"
                                +"Port"
                            }
                            input {
                                type = InputType.number
                                className = ClassName("form-control")
                                key = "syslog-port"
                                placeholder = "5140"
                                id = "syslog-port"
                                defaultValue = config.syslog.port?.toString() ?: ""
                                ref = portRef
                            }
                        }
                    }

                    button {
                        type = ButtonType.submit
                        className = ClassName("btn btn-primary")
                        onClick = { ev ->
                            ev.preventDefault()
                            val syslogConfig = SyslogConfig(
                                hostRef.current?.value,
                                portRef.current?.value?.toIntOrNull()
                            )
                            Axios.post<String>("$apiRoot/config/syslog", syslogConfig, generateConfig<SyslogConfig, String>())
                                .then {
                                    setSuccess(true)
                                    props.updateCallback(config.copy(syslog = syslogConfig))
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
    }
}
