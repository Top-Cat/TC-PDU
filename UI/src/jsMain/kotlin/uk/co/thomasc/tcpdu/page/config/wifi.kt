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
import uk.co.thomasc.tcpdu.page.WifiConfig
import uk.co.thomasc.tcpdu.page.handleForbidden
import uk.co.thomasc.tcpdu.success
import web.cssom.ClassName
import web.html.ButtonType
import web.html.HTMLInputElement
import web.html.InputType

val wifiConfig = fcmemo<ConfigProps>("WiFi Config") { props ->
    val history = useNavigate()
    val (success, setSuccess) = useState<Boolean?>(null)

    val ssidRef = useRef<HTMLInputElement>()
    val wifiPwRef = useRef<HTMLInputElement>()
    val enabledRef = useRef<HTMLInputElement>()

    props.config?.let { config ->
        div {
            className = ClassName("card border-primary")
            div {
                className = ClassName("card-header")
                +"Wifi"
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
                            defaultChecked = config.wifi.enabled == true
                            id = "wifi-enabled"
                            ref = enabledRef
                        }
                        label {
                            className = ClassName("form-check-label")
                            htmlFor = "wifi-enabled"
                            +"Enabled"
                        }
                    }

                    div {
                        label {
                            className = ClassName("form-label")
                            htmlFor = "wifi-ssid"
                            +"SSID"
                        }
                        input {
                            type = InputType.text
                            className = ClassName("form-control")
                            placeholder = "SSID"
                            id = "wifi-ssid"
                            defaultValue = config.wifi.ssid ?: ""
                            ref = ssidRef
                        }
                    }

                    div {
                        label {
                            className = ClassName("form-label")
                            htmlFor = "wifi-pw"
                            +"Wifi Password"
                        }
                        input {
                            type = InputType.password
                            className = ClassName("form-control")
                            placeholder = "Wifi Password"
                            id = "wifi-pw"
                            defaultValue = config.wifi.pass ?: ""
                            ref = wifiPwRef
                        }
                    }

                    button {
                        type = ButtonType.submit
                        className = ClassName("btn btn-primary")
                        onClick = { ev ->
                            ev.preventDefault()
                            val newConfig = WifiConfig(enabledRef.current?.checked, ssidRef.current?.value, wifiPwRef.current?.value)
                            Axios.post<String>("$apiRoot/config/wifi", newConfig, generateConfig<WifiConfig, String>())
                                .then {
                                    setSuccess(true)
                                    props.updateCallback(config.copy(wifi = newConfig))
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
