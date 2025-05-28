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
import uk.co.thomasc.tcpdu.page.SmtpConfig
import uk.co.thomasc.tcpdu.page.handleForbidden
import uk.co.thomasc.tcpdu.success
import web.cssom.ClassName
import web.html.ButtonType
import web.html.HTMLInputElement
import web.html.InputType

val smtpConfig = fcmemo<ConfigProps>("SMTP Config") { props ->
    val history = useNavigate()
    val (success, setSuccess) = useState<Boolean?>(null)

    val hostRef = useRef<HTMLInputElement>()
    val portRef = useRef<HTMLInputElement>()
    val userRef = useRef<HTMLInputElement>()
    val passRef = useRef<HTMLInputElement>()
    val toRef = useRef<HTMLInputElement>()
    val fromRef = useRef<HTMLInputElement>()

    val (showSmtpPassword, setShowSmtpPassword) = useState(false)

    props.config?.let { config ->
        div {
            className = ClassName("card border-primary")
            div {
                className = ClassName("card-header")
                +"SMTP"
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
                                htmlFor = "smtp-host"
                                +"Host"
                            }
                            input {
                                type = InputType.text
                                className = ClassName("form-control")
                                placeholder = "smtp.example.com"
                                id = "smtp-host"
                                defaultValue = config.log.smtp?.host ?: ""
                                ref = hostRef
                            }
                        }

                        div {
                            className = ClassName("col-md-3")
                            label {
                                className = ClassName("form-label")
                                htmlFor = "smtp-port"
                                +"Port"
                            }
                            input {
                                type = InputType.number
                                className = ClassName("form-control")
                                placeholder = "587"
                                id = "smtp-port"
                                defaultValue = config.log.smtp?.port?.toString() ?: ""
                                ref = portRef
                            }
                        }
                    }

                    div {
                        label {
                            className = ClassName("form-label")
                            htmlFor = "smtp-user"
                            +"User"
                        }
                        input {
                            type = InputType.text
                            className = ClassName("form-control")
                            placeholder = "smtp-user"
                            id = "smtp-user"
                            defaultValue = config.log.smtp?.user ?: ""
                            ref = userRef
                        }
                    }

                    div {
                        label {
                            className = ClassName("form-label")
                            htmlFor = "smtp-pw"
                            +"Password"
                        }
                        div {
                            className = ClassName("input-group")
                            input {
                                type = if (showSmtpPassword) InputType.text else InputType.password
                                className = ClassName("form-control")
                                placeholder = "********"
                                id = "smtp-pw"
                                defaultValue = config.log.smtp?.password ?: ""
                                ref = passRef
                            }
                            span {
                                className = ClassName("input-group-text")
                                i {
                                    className = ClassName("fas fa-eye" + if (showSmtpPassword) "" else "-slash")
                                    onClick = {
                                        setShowSmtpPassword(!showSmtpPassword)
                                    }
                                    role = AriaRole.button
                                }
                            }
                        }
                    }

                    div {
                        label {
                            className = ClassName("form-label")
                            htmlFor = "smtp-from"
                            +"From"
                        }
                        input {
                            type = InputType.text
                            className = ClassName("form-control")
                            placeholder = "from@example.com"
                            id = "smtp-from"
                            defaultValue = config.log.smtp?.from ?: ""
                            ref = fromRef
                        }
                    }

                    div {
                        label {
                            className = ClassName("form-label")
                            htmlFor = "smtp-to"
                            +"To"
                        }
                        input {
                            type = InputType.text
                            className = ClassName("form-control")
                            placeholder = "to@example.com"
                            id = "smtp-to"
                            defaultValue = config.log.smtp?.to ?: ""
                            ref = toRef
                        }
                    }

                    button {
                        type = ButtonType.submit
                        className = ClassName("btn btn-primary")
                        onClick = { ev ->
                            ev.preventDefault()
                            val newSmtpConfig = SmtpConfig(
                                hostRef.current?.value,
                                portRef.current?.value?.toIntOrNull(),
                                userRef.current?.value,
                                passRef.current?.value,
                                fromRef.current?.value,
                                toRef.current?.value
                            )
                            Axios.post<String>("$apiRoot/config/smtp", newSmtpConfig, generateConfig<SmtpConfig, String>())
                                .then {
                                    setSuccess(true)
                                    props.updateCallback(config.copy(log = config.log.copy(smtp = newSmtpConfig)))
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
