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
import uk.co.thomasc.tcpdu.page.RadiusConfig
import uk.co.thomasc.tcpdu.page.handleForbidden
import uk.co.thomasc.tcpdu.success
import web.autofill.AutoFillBase
import web.cssom.ClassName
import web.html.ButtonType
import web.html.HTMLInputElement
import web.html.InputType

val radiusConfig = fcmemo<ConfigProps>("Radius Config") { props ->
    val history = useNavigate()
    val (success, setSuccess) = useState<Boolean?>(null)

    val hostRef = useRef<HTMLInputElement>()
    val portRef = useRef<HTMLInputElement>()
    val secretRef = useRef<HTMLInputElement>()
    val timeoutRef = useRef<HTMLInputElement>()
    val retriesRef = useRef<HTMLInputElement>()

    val (showRadiusSecret, setShowRadiusSecret) = useState(false)

    props.config?.let { config ->
        div {
            className = ClassName("card border-primary")
            div {
                className = ClassName("card-header")
                +"Radius"
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
                                htmlFor = "radius-host"
                                +"Host"
                            }
                            input {
                                type = InputType.text
                                className = ClassName("form-control")
                                placeholder = "radius.example.com"
                                id = "radius-host"
                                defaultValue = config.radius.ip ?: ""
                                ref = hostRef
                            }
                        }

                        div {
                            className = ClassName("col-md-3")
                            label {
                                className = ClassName("form-label")
                                htmlFor = "radius-port"
                                +"Port"
                            }
                            input {
                                type = InputType.number
                                className = ClassName("form-control")
                                placeholder = "1812"
                                id = "radius-port"
                                defaultValue = config.radius.port?.toString() ?: ""
                                ref = portRef
                            }
                        }
                    }

                    div {
                        label {
                            className = ClassName("form-label")
                            htmlFor = "radius-pw"
                            +"Secret"
                        }
                        div {
                            className = ClassName("input-group")
                            input {
                                type = if (showRadiusSecret) InputType.text else InputType.password
                                autoComplete = AutoFillBase.off
                                className = ClassName("form-control")
                                placeholder = "********"
                                id = "radius-pw"
                                defaultValue = config.radius.secret ?: ""
                                ref = secretRef
                            }
                            span {
                                className = ClassName("input-group-text")
                                i {
                                    className = ClassName("fas fa-eye" + if (showRadiusSecret) "" else "-slash")
                                    onClick = {
                                        setShowRadiusSecret(!showRadiusSecret)
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
                                htmlFor = "radius-timeout"
                                +"Timeout"
                            }
                            input {
                                type = InputType.number
                                className = ClassName("form-control w-50")
                                placeholder = "5"
                                id = "radius-timeout"
                                defaultValue = config.radius.timeout?.toString() ?: ""
                                ref = timeoutRef
                            }
                        }

                        div {
                            className = ClassName("col-md-6")
                            label {
                                className = ClassName("form-label")
                                htmlFor = "radius-retry"
                                +"Retries"
                            }
                            input {
                                type = InputType.number
                                className = ClassName("form-control w-50")
                                placeholder = "3"
                                id = "radius-retry"
                                defaultValue = config.radius.retries?.toString() ?: ""
                                ref = retriesRef
                            }
                        }
                    }

                    button {
                        type = ButtonType.submit
                        className = ClassName("btn btn-primary")
                        onClick = { ev ->
                            ev.preventDefault()
                            val newConfig = RadiusConfig(
                                hostRef.current?.value,
                                portRef.current?.value?.toIntOrNull(),
                                secretRef.current?.value,
                                timeoutRef.current?.value?.toIntOrNull(),
                                retriesRef.current?.value?.toIntOrNull()
                            )
                            Axios.post<String>("$apiRoot/config/radius", newConfig, generateConfig<RadiusConfig, String>())
                                .then {
                                    setSuccess(true)
                                    props.updateCallback(config.copy(radius = newConfig))
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
