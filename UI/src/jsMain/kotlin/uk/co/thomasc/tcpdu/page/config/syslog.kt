package uk.co.thomasc.tcpdu.page.config

import external.Axios
import external.generateConfig
import kotlinx.html.ButtonType
import kotlinx.html.InputType
import kotlinx.html.id
import kotlinx.html.js.onClickFunction
import org.w3c.dom.HTMLInputElement
import react.dom.button
import react.dom.defaultValue
import react.dom.div
import react.dom.form
import react.dom.input
import react.dom.label
import react.fc
import react.router.useNavigate
import react.useRef
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.errors
import uk.co.thomasc.tcpdu.page.SyslogConfig
import uk.co.thomasc.tcpdu.page.handleForbidden
import uk.co.thomasc.tcpdu.success

val syslogConfig = fc<ConfigProps> { props ->
    val history = useNavigate()
    val (success, setSuccess) = useState<Boolean?>(null)

    val hostRef = useRef<HTMLInputElement>()
    val portRef = useRef<HTMLInputElement>()

    props.config?.let { config ->
        div("card border-primary") {
            div("card-header") {
                +"Syslog"
            }
            div("card-body") {
                if (success == true) {
                    success { +"Config saved" }
                } else if (success == false) {
                    errors { +"Unknown error" }
                }

                form {
                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "syslog-host"
                            +"Host"
                        }
                        input(InputType.text, classes = "form-control") {
                            key = "syslog-host"
                            attrs.placeholder = "syslog.example.com"
                            attrs.id = "syslog-host"
                            attrs.defaultValue = config.syslog.host ?: ""
                            ref = hostRef
                        }
                    }

                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "syslog-port"
                            +"Port"
                        }
                        input(InputType.number, classes = "form-control w-25") {
                            key = "syslog-port"
                            attrs.placeholder = "5140"
                            attrs.id = "syslog-port"
                            attrs.defaultValue = config.syslog.port?.toString() ?: ""
                            ref = portRef
                        }
                    }

                    button(type = ButtonType.submit, classes = "btn btn-primary") {
                        attrs.onClickFunction = { ev ->
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
