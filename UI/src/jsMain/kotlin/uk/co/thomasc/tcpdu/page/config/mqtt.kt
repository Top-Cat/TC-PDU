package uk.co.thomasc.tcpdu.page.config

import external.Axios
import external.generateConfig
import kotlinx.html.ButtonType
import kotlinx.html.InputType
import kotlinx.html.id
import kotlinx.html.js.onClickFunction
import kotlinx.html.role
import org.w3c.dom.HTMLInputElement
import react.dom.button
import react.dom.defaultValue
import react.dom.div
import react.dom.form
import react.dom.i
import react.dom.input
import react.dom.label
import react.dom.span
import react.fc
import react.router.useNavigate
import react.useRef
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.errors
import uk.co.thomasc.tcpdu.page.MqttConfig
import uk.co.thomasc.tcpdu.page.handleForbidden
import uk.co.thomasc.tcpdu.success

val mqttConfig = fc<ConfigProps> { props ->
    val history = useNavigate()
    val (success, setSuccess) = useState<Boolean?>(null)

    val hostRef = useRef<HTMLInputElement>()
    val portRef = useRef<HTMLInputElement>()
    val userRef = useRef<HTMLInputElement>()
    val passRef = useRef<HTMLInputElement>()
    val clientIdRef = useRef<HTMLInputElement>()
    val prefixRef = useRef<HTMLInputElement>()

    val (showMqttPassword, setShowMqttPassword) = useState(false)

    props.config?.let { config ->
        div("card border-primary") {
            div("card-header") {
                +"MQTT"
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
                            attrs.htmlFor = "mqtt-host"
                            +"Host"
                        }
                        input(InputType.text, classes = "form-control") {
                            attrs.placeholder = "mqtt.example.com"
                            attrs.id = "mqtt-host"
                            attrs.defaultValue = config.mqtt.host ?: ""
                            ref = hostRef
                        }
                    }

                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "mqtt-port"
                            +"Port"
                        }
                        input(InputType.number, classes = "form-control w-25") {
                            attrs.placeholder = "1883"
                            attrs.id = "mqtt-port"
                            attrs.defaultValue = config.mqtt.port?.toString() ?: ""
                            ref = portRef
                        }
                    }

                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "mqtt-user"
                            +"User"
                        }
                        input(InputType.text, classes = "form-control") {
                            attrs.placeholder = "mqtt-user"
                            attrs.id = "mqtt-user"
                            attrs.defaultValue = config.mqtt.user ?: ""
                            ref = userRef
                        }
                    }

                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "mqtt-pw"
                            +"Password"
                        }
                        div("input-group") {
                            input(if (showMqttPassword) InputType.text else InputType.password, classes = "form-control") {
                                attrs.placeholder = "********"
                                attrs.id = "mqtt-pw"
                                attrs.defaultValue = config.mqtt.password ?: ""
                                ref = passRef
                            }
                            span("input-group-text") {
                                i("fas fa-eye" + if (showMqttPassword) "" else "-slash") {
                                    attrs.onClickFunction = {
                                        setShowMqttPassword(!showMqttPassword)
                                    }
                                    attrs.role = "button"
                                }
                            }
                        }
                    }

                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "mqtt-clientid"
                            +"Client Id"
                        }
                        input(InputType.text, classes = "form-control") {
                            attrs.placeholder = "esp32-tcpdu"
                            attrs.id = "mqtt-clientid"
                            attrs.defaultValue = config.mqtt.clientId ?: ""
                            ref = clientIdRef
                        }
                    }

                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "mqtt-prefix"
                            +"Prefix"
                        }
                        input(InputType.text, classes = "form-control") {
                            attrs.placeholder = "tc-pdu/"
                            attrs.id = "mqtt-prefix"
                            attrs.defaultValue = config.mqtt.prefix ?: ""
                            ref = prefixRef
                        }
                    }

                    button(type = ButtonType.submit, classes = "btn btn-primary") {
                        attrs.onClickFunction = { ev ->
                            ev.preventDefault()
                            val mqttConfig = MqttConfig(
                                hostRef.current?.value,
                                portRef.current?.value?.toIntOrNull(),
                                userRef.current?.value,
                                passRef.current?.value,
                                clientIdRef.current?.value,
                                prefixRef.current?.value
                            )
                            Axios.post<String>("$apiRoot/config/mqtt", mqttConfig, generateConfig<MqttConfig, String>())
                                .then {
                                    setSuccess(true)
                                    props.updateCallback(config.copy(mqtt = mqttConfig))
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
