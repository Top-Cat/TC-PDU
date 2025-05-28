package uk.co.thomasc.tcpdu.page.config

import external.Axios
import external.generateConfig
import react.dom.aria.AriaRole
import react.dom.html.ReactHTML.button
import react.dom.html.ReactHTML.div
import react.dom.html.ReactHTML.form
import react.dom.html.ReactHTML.i
import react.dom.html.ReactHTML.input
import react.dom.html.ReactHTML.label
import react.dom.html.ReactHTML.span
import react.router.useNavigate
import react.useRef
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.errors
import uk.co.thomasc.tcpdu.fcmemo
import uk.co.thomasc.tcpdu.page.MqttConfig
import uk.co.thomasc.tcpdu.page.handleForbidden
import uk.co.thomasc.tcpdu.success
import web.cssom.ClassName
import web.html.ButtonType
import web.html.HTMLInputElement
import web.html.InputType

val mqttConfig = fcmemo<ConfigProps>("MQTT Config") { props ->
    val history = useNavigate()
    val (success, setSuccess) = useState<Boolean?>(null)

    val hostRef = useRef<HTMLInputElement>()
    val portRef = useRef<HTMLInputElement>()
    val userRef = useRef<HTMLInputElement>()
    val passRef = useRef<HTMLInputElement>()
    val clientIdRef = useRef<HTMLInputElement>()
    val prefixRef = useRef<HTMLInputElement>()
    val enabledRef = useRef<HTMLInputElement>()
    val addMacRef = useRef<HTMLInputElement>()

    val (showMqttPassword, setShowMqttPassword) = useState(false)

    props.config?.let { config ->
        div {
            className = ClassName("card border-primary")
            div {
                className = ClassName("card-header")
                +"MQTT"
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
                        className = ClassName("form-check")
                        input {
                            type = InputType.checkbox
                            className = ClassName("form-check-input")
                            defaultChecked = config.mqtt.enabled == true
                            id = "mqtt-enabled"
                            ref = enabledRef
                        }
                        label {
                            className = ClassName("form-check-label")
                            htmlFor = "mqtt-enabled"
                            +"Enabled"
                        }
                    }

                    div {
                        className = ClassName("row")
                        div {
                            className = ClassName("col-md-9")
                            label {
                                className = ClassName("form-label")
                                htmlFor = "mqtt-host"
                                +"Host"
                            }
                            input {
                                type = InputType.text
                                className = ClassName("form-control")
                                placeholder = "mqtt.example.com"
                                id = "mqtt-host"
                                defaultValue = config.mqtt.host ?: ""
                                ref = hostRef
                            }
                        }

                        div {
                            className = ClassName("col-md-3")
                            label {
                                className = ClassName("form-label")
                                htmlFor = "mqtt-port"
                                +"Port"
                            }
                            input {
                                type = InputType.number
                                className = ClassName("form-control")
                                placeholder = "1883"
                                id = "mqtt-port"
                                defaultValue = config.mqtt.port?.toString() ?: ""
                                ref = portRef
                            }
                        }
                    }

                    div {
                        label {
                            className = ClassName("form-label")
                            htmlFor = "mqtt-user"
                            +"User"
                        }
                        input {
                            type = InputType.text
                            className = ClassName("form-control")
                            placeholder = "mqtt-user"
                            id = "mqtt-user"
                            defaultValue = config.mqtt.user ?: ""
                            ref = userRef
                        }
                    }

                    div {
                        label {
                            className = ClassName("form-label")
                            htmlFor = "mqtt-pw"
                            +"Password"
                        }
                        div {
                            className = ClassName("input-group")
                            input {
                                type = if (showMqttPassword) InputType.text else InputType.password
                                className = ClassName("form-control")
                                placeholder = "********"
                                id = "mqtt-pw"
                                defaultValue = config.mqtt.password ?: ""
                                ref = passRef
                            }
                            span {
                                className = ClassName("input-group-text")
                                i {
                                    className = ClassName("fas fa-eye" + if (showMqttPassword) "" else "-slash")
                                    onClick = {
                                        setShowMqttPassword(!showMqttPassword)
                                    }
                                    role = AriaRole.button
                                }
                            }
                        }
                    }

                    div {
                        className = ClassName("row")
                        div {
                            className = ClassName("col-md-6")
                            label {
                                className = ClassName("form-label")
                                htmlFor = "mqtt-clientid"
                                +"Client Id"
                            }
                            input {
                                type = InputType.text
                                className = ClassName("form-control")
                                placeholder = "esp32-tcpdu"
                                id = "mqtt-clientid"
                                defaultValue = config.mqtt.clientId ?: ""
                                ref = clientIdRef
                            }
                        }

                        div {
                            className = ClassName("col-md-6")
                            label {
                                className = ClassName("form-label")
                                htmlFor = "mqtt-prefix"
                                +"Prefix"
                            }
                            input {
                                type = InputType.text
                                className = ClassName("form-control")
                                placeholder = "tc-pdu/"
                                id = "mqtt-prefix"
                                defaultValue = config.mqtt.prefix ?: ""
                                ref = prefixRef
                            }
                        }
                    }

                    div {
                        className = ClassName("form-check")
                        input {
                            type = InputType.checkbox
                            className = ClassName("form-check-input")
                            defaultChecked = config.mqtt.addMacToPrefix == true
                            id = "add-mac-to-prefix"
                            ref = addMacRef
                        }
                        label {
                            className = ClassName("form-check-label")
                            htmlFor = "add-mac-to-prefix"
                            +"Add mac to prefix"
                        }
                    }

                    button {
                        type = ButtonType.submit
                        className = ClassName("btn btn-primary")
                        onClick = { ev ->
                            ev.preventDefault()
                            val mqttConfig = MqttConfig(
                                enabledRef.current?.checked,
                                hostRef.current?.value,
                                portRef.current?.value?.toIntOrNull(),
                                userRef.current?.value,
                                passRef.current?.value,
                                clientIdRef.current?.value,
                                prefixRef.current?.value,
                                addMacRef.current?.checked
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
