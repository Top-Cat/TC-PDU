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
import uk.co.thomasc.tcpdu.page.RadiusConfig
import uk.co.thomasc.tcpdu.page.handleForbidden
import uk.co.thomasc.tcpdu.success

val radiusConfig = fc<ConfigProps> { props ->
    val history = useNavigate()
    val (success, setSuccess) = useState<Boolean?>(null)

    val hostRef = useRef<HTMLInputElement>()
    val portRef = useRef<HTMLInputElement>()
    val secretRef = useRef<HTMLInputElement>()
    val timeoutRef = useRef<HTMLInputElement>()
    val retriesRef = useRef<HTMLInputElement>()

    val (showRadiusSecret, setShowRadiusSecret) = useState(false)

    props.config?.let { config ->
        div("card border-primary") {
            div("card-header") {
                +"Radius"
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
                            attrs.htmlFor = "radius-host"
                            +"Host"
                        }
                        input(InputType.text, classes = "form-control") {
                            attrs.placeholder = "radius.example.com"
                            attrs.id = "radius-host"
                            attrs.defaultValue = config.radius.ip ?: ""
                            ref = hostRef
                        }
                    }

                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "radius-port"
                            +"Port"
                        }
                        input(InputType.number, classes = "form-control") {
                            attrs.placeholder = "1812"
                            attrs.id = "radius-port"
                            attrs.defaultValue = config.radius.port?.toString() ?: ""
                            ref = portRef
                        }
                    }

                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "radius-pw"
                            +"Secret"
                        }
                        div("input-group") {
                            input(if (showRadiusSecret) InputType.text else InputType.password, classes = "form-control") {
                                attrs.placeholder = "********"
                                attrs.id = "radius-pw"
                                attrs.defaultValue = config.radius.secret ?: ""
                                ref = secretRef
                            }
                            span("input-group-text") {
                                i("fas fa-eye" + if (showRadiusSecret) "" else "-slash") {
                                    attrs.onClickFunction = {
                                        setShowRadiusSecret(!showRadiusSecret)
                                    }
                                    attrs.role = "button"
                                }
                            }
                        }
                    }

                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "radius-timeout"
                            +"Timeout"
                        }
                        input(InputType.number, classes = "form-control w-25") {
                            attrs.placeholder = "5"
                            attrs.id = "radius-timeout"
                            attrs.defaultValue = config.radius.timeout?.toString() ?: ""
                            ref = timeoutRef
                        }
                    }

                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "radius-retry"
                            +"Retries"
                        }
                        input(InputType.number, classes = "form-control w-25") {
                            attrs.placeholder = "3"
                            attrs.id = "radius-retry"
                            attrs.defaultValue = config.radius.retries?.toString() ?: ""
                            ref = retriesRef
                        }
                    }

                    button(type = ButtonType.submit, classes = "btn btn-primary") {
                        attrs.onClickFunction = { ev ->
                            ev.preventDefault()
                            val newConfig = RadiusConfig(
                                hostRef.current?.value,
                                portRef.current?.value?.toIntOrNull(),
                                secretRef.current?.value,
                                timeoutRef.current?.value?.toIntOrNull(),
                                retriesRef.current?.value?.toIntOrNull()
                            )
                            Axios.post<String>("$apiRoot/config/radius", newConfig, generateConfig<RadiusConfig, String>())
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
    }
}
