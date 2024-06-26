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
import uk.co.thomasc.tcpdu.page.WifiConfig
import uk.co.thomasc.tcpdu.page.handleForbidden
import uk.co.thomasc.tcpdu.success

val wifiConfig = fc<ConfigProps> { props ->
    val history = useNavigate()
    val (success, setSuccess) = useState<Boolean?>(null)

    val ssidRef = useRef<HTMLInputElement>()
    val wifiPwRef = useRef<HTMLInputElement>()
    val enabledRef = useRef<HTMLInputElement>()

    props.config?.let { config ->
        div("card border-primary") {
            div("card-header") {
                +"Wifi"
            }
            div("card-body") {
                if (success == true) {
                    success { +"Config saved" }
                } else if (success == false) {
                    errors { +"Unknown error" }
                }

                form {
                    div("form-check") {
                        input(InputType.checkBox, classes = "form-check-input") {
                            attrs.defaultChecked = config.wifi.enabled == true
                            attrs.id = "wifi-enabled"
                            ref = enabledRef
                        }
                        label("form-check-label") {
                            attrs.htmlFor = "wifi-enabled"
                            +"Enabled"
                        }
                    }

                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "wifi-ssid"
                            +"SSID"
                        }
                        input(InputType.text, classes = "form-control") {
                            attrs.placeholder = "SSID"
                            attrs.id = "wifi-ssid"
                            attrs.defaultValue = config.wifi.ssid ?: ""
                            ref = ssidRef
                        }
                    }

                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "wifi-pw"
                            +"Wifi Password"
                        }
                        input(InputType.password, classes = "form-control") {
                            attrs.placeholder = "Wifi Password"
                            attrs.id = "wifi-pw"
                            attrs.defaultValue = config.wifi.pass ?: ""
                            ref = wifiPwRef
                        }
                    }

                    button(type = ButtonType.submit, classes = "btn btn-primary") {
                        attrs.onClickFunction = { ev ->
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
