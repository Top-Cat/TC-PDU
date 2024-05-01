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
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.page.WifiConfig
import uk.co.thomasc.tcpdu.page.handleForbidden

val wifiConfig = fc<ConfigProps> { props ->
    val history = useNavigate()

    val ssidRef = useRef<HTMLInputElement>()
    val wifiPwRef = useRef<HTMLInputElement>()

    props.config?.let { config ->
        div("card border-primary") {
            div("card-header") {
                +"Wifi"
            }
            div("card-body") {
                form {
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
                            Axios.post<String>(
                                "$apiRoot/config/wifi",
                                WifiConfig(ssidRef.current?.value, wifiPwRef.current?.value),
                                generateConfig<WifiConfig, String>()
                            )
                                .then {
                                    // TODO: Show toast
                                    console.log("Success")
                                }.handleForbidden(history)
                        }
                        +"Save"
                    }
                }
            }
        }
    }
}
